#include "global.h"
#include "pokemon.h"
#include "random.h"
#include "roguelike.h"
#include "constants/pokemon.h"
#include "data/roguelike.h"
#include "data/roguelike_snapshots.h"

EWRAM_DATA struct RoguelikeRun gRoguelikeRun = {0};

static u32 RandomBelow(rng_value_t *rng, u32 n)
{
    return LocalRandom(rng) % n;
}

static bool32 RowsAreNeighbours(u32 a, u32 b)
{
    return (a > b ? a - b : b - a) <= 1;
}

// Every row in one set has a neighbouring row in the other, both ways round.
static bool32 ColumnsCanLink(u32 rowsA, u32 rowsB)
{
    u32 a, b;

    for (a = 0; a < ROUTE_MAP_ROWS; a++)
    {
        bool32 found = FALSE;

        if (!(rowsA & (1 << a)))
            continue;
        for (b = 0; b < ROUTE_MAP_ROWS; b++)
        {
            if ((rowsB & (1 << b)) && RowsAreNeighbours(a, b))
                found = TRUE;
        }
        if (!found)
            return FALSE;
    }
    for (b = 0; b < ROUTE_MAP_ROWS; b++)
    {
        bool32 found = FALSE;

        if (!(rowsB & (1 << b)))
            continue;
        for (a = 0; a < ROUTE_MAP_ROWS; a++)
        {
            if ((rowsA & (1 << a)) && RowsAreNeighbours(a, b))
                found = TRUE;
        }
        if (!found)
            return FALSE;
    }
    return TRUE;
}

static u32 CountBits(u32 bits)
{
    u32 count = 0;

    while (bits)
    {
        count += bits & 1;
        bits >>= 1;
    }
    return count;
}

// Picks which rows a middle column uses. The column after it is either
// another middle column or the single transition node, so the last middle
// column must stay within one row of the exit.
static u32 PickColumnRows(rng_value_t *rng, u32 previousRows, u32 exitRow, bool32 isLastMiddle)
{
    u32 tries, row;

    for (tries = 0; tries < 64; tries++)
    {
        u32 count = ROUTE_COLUMN_MIN_NODES + RandomBelow(rng, ROUTE_COLUMN_MAX_NODES - ROUTE_COLUMN_MIN_NODES + 1);
        u32 rows = 0;

        while (CountBits(rows) < count)
            rows |= 1 << RandomBelow(rng, ROUTE_MAP_ROWS);
        if (isLastMiddle)
        {
            for (row = 0; row < ROUTE_MAP_ROWS; row++)
            {
                if (!RowsAreNeighbours(row, exitRow))
                    rows &= ~(1 << row);
            }
        }
        if (rows != 0 && ColumnsCanLink(previousRows, rows))
            return rows;
    }

    // Fallback: the rows next to the previous column's first row always link.
    for (row = 0; row < ROUTE_MAP_ROWS; row++)
    {
        if (previousRows & (1 << row))
            return (1 << row) | (row + 1 < ROUTE_MAP_ROWS ? 1 << (row + 1) : 0);
    }
    return 1 << exitRow;
}

static u32 PickNodeType(rng_value_t *rng, bool32 hasCenter, bool32 hasMart)
{
    u32 type, total = 0, roll;

    for (type = 0; type < ROUTE_NODE_TYPE_COUNT; type++)
    {
        if ((type == ROUTE_NODE_CENTER && hasCenter) || (type == ROUTE_NODE_MART && hasMart))
            continue;
        total += sRouteNodeTypeWeights[type];
    }
    roll = RandomBelow(rng, total);
    for (type = 0; type < ROUTE_NODE_TYPE_COUNT; type++)
    {
        if ((type == ROUTE_NODE_CENTER && hasCenter) || (type == ROUTE_NODE_MART && hasMart))
            continue;
        if (roll < sRouteNodeTypeWeights[type])
            return type;
        roll -= sRouteNodeTypeWeights[type];
    }
    return ROUTE_NODE_WILD;
}

static u32 CountLinksOut(const struct RouteMap *map, u32 column, u32 row)
{
    return CountBits(map->links[column][row]);
}

static u32 CountLinksIn(const struct RouteMap *map, u32 column, u32 row)
{
    u32 from, count = 0;

    if (column == 0)
        return 0;
    for (from = 0; from < ROUTE_MAP_ROWS; from++)
    {
        if (map->links[column - 1][from] & (1 << row))
            count++;
    }
    return count;
}

void GenerateRouteMap(struct RouteMap *map, u32 seed)
{
    rng_value_t rng = LocalRandomSeed(seed);
    u32 column, row, next;
    u32 startRow, exitRow, rows;
    bool32 hasCenter = FALSE, hasMart = FALSE;

    memset(map, 0, sizeof(*map));
    map->numColumns = ROUTE_MAP_COLUMNS;

    // Start and exit sit in the middle two rows, so both have room for branches.
    startRow = 1 + RandomBelow(&rng, 2);
    exitRow = 1 + RandomBelow(&rng, 2);
    map->type[0][startRow] = ROUTE_NODE_START;
    map->type[ROUTE_MAP_EXIT_COLUMN][exitRow] = ROUTE_NODE_TRANSITION;

    rows = 1 << startRow;
    for (column = ROUTE_MAP_FIRST_MIDDLE_COLUMN; column <= ROUTE_MAP_LAST_MIDDLE_COLUMN; column++)
    {
        rows = PickColumnRows(&rng, rows, exitRow, column == ROUTE_MAP_LAST_MIDDLE_COLUMN);
        for (row = 0; row < ROUTE_MAP_ROWS; row++)
        {
            if (!(rows & (1 << row)))
                continue;
            map->type[column][row] = PickNodeType(&rng, hasCenter, hasMart);
            if (map->type[column][row] == ROUTE_NODE_CENTER)
                hasCenter = TRUE;
            else if (map->type[column][row] == ROUTE_NODE_MART)
                hasMart = TRUE;
        }
    }

    // Link every node to each neighbouring node in the next column...
    for (column = 0; column < ROUTE_MAP_EXIT_COLUMN; column++)
    {
        for (row = 0; row < ROUTE_MAP_ROWS; row++)
        {
            if (map->type[column][row] == ROUTE_NODE_EMPTY)
                continue;
            for (next = 0; next < ROUTE_MAP_ROWS; next++)
            {
                if (map->type[column + 1][next] != ROUTE_NODE_EMPTY && RowsAreNeighbours(row, next))
                    map->links[column][row] |= 1 << next;
            }
        }
    }

    // ...then thin the links out, keeping every node joined both ways.
    for (column = 0; column < ROUTE_MAP_EXIT_COLUMN; column++)
    {
        for (row = 0; row < ROUTE_MAP_ROWS; row++)
        {
            for (next = 0; next < ROUTE_MAP_ROWS; next++)
            {
                if (!(map->links[column][row] & (1 << next)))
                    continue;
                if (CountLinksOut(map, column, row) > 1
                 && CountLinksIn(map, column + 1, next) > 1
                 && RandomBelow(&rng, 100) < ROUTE_LINK_DROP_CHANCE)
                    map->links[column][row] &= ~(1 << next);
            }
        }
    }

    // Sideways links between neighbouring nodes in the same middle column.
    for (column = ROUTE_MAP_FIRST_MIDDLE_COLUMN; column <= ROUTE_MAP_LAST_MIDDLE_COLUMN; column++)
    {
        for (row = 0; row + 1 < ROUTE_MAP_ROWS; row++)
        {
            if (map->type[column][row] != ROUTE_NODE_EMPTY
             && map->type[column][row + 1] != ROUTE_NODE_EMPTY
             && RandomBelow(&rng, 100) < ROUTE_SIDE_LINK_CHANCE)
                map->sideLinks[column] |= 1 << row;
        }
    }
}

void GenerateGymMap(struct RouteMap *map, u32 numTrainers)
{
    u32 column;

    memset(map, 0, sizeof(*map));
    numTrainers = min(numTrainers, ROUTE_MAP_COLUMNS - 2);
    map->numColumns = numTrainers + 2;
    map->type[0][GYM_MAP_ROW] = ROUTE_NODE_START;
    for (column = 1; column <= numTrainers; column++)
        map->type[column][GYM_MAP_ROW] = ROUTE_NODE_GYM_TRAINER;
    map->type[numTrainers + 1][GYM_MAP_ROW] = ROUTE_NODE_LEADER;
    for (column = 0; column <= numTrainers; column++)
        map->links[column][GYM_MAP_ROW] = 1 << GYM_MAP_ROW;
}

u32 GetRouteMapStartRow(const struct RouteMap *map)
{
    u32 row;

    for (row = 0; row < ROUTE_MAP_ROWS; row++)
    {
        if (map->type[0][row] == ROUTE_NODE_START)
            return row;
    }
    return 0;
}

void GetCurrentRunMap(struct RouteMap *map)
{
    if (gRoguelikeRun.inGym)
        GenerateGymMap(map, gRoguelikeRun.gymTrainers);
    else
        GenerateRouteMap(map, gRoguelikeRun.routeSeed);
}

bool32 RoguelikeIsActive(void)
{
    return IS_FRLG && gRoguelikeRun.runSeed != 0;
}

bool32 RouteNodeWasVisited(u32 column, u32 row)
{
    return (gRoguelikeRun.visited & (1u << (column * ROUTE_MAP_ROWS + row))) != 0;
}

void MarkRouteNodeVisited(u32 column, u32 row)
{
    gRoguelikeRun.visited |= 1u << (column * ROUTE_MAP_ROWS + row);
}

u32 GetRouteMapChoices(const struct RouteMap *map, u32 column, u32 row, struct RouteChoice *choices)
{
    u32 next, count = 0;

    if (row > 0 && (map->sideLinks[column] & (1 << (row - 1))))
    {
        choices[count].column = column;
        choices[count++].row = row - 1;
    }
    for (next = 0; next < ROUTE_MAP_ROWS; next++)
    {
        if (column + 1 < map->numColumns && (map->links[column][row] & (1 << next)))
        {
            choices[count].column = column + 1;
            choices[count++].row = next;
        }
    }
    if (map->sideLinks[column] & (1 << row))
    {
        choices[count].column = column;
        choices[count++].row = row + 1;
    }
    return count;
}

s32 GetRocketFrontColumn(const struct RouteMap *map, u32 rocketMoves)
{
    s32 front = (s32)rocketMoves - ROUTE_ROCKET_LEAD;

    if (front < 0)
        return -1;
    return min(front, map->numColumns - 1);
}

bool32 RouteNodeIsOvertaken(const struct RouteMap *map, u32 column, u32 rocketMoves)
{
    return (s32)column <= GetRocketFrontColumn(map, rocketMoves);
}

void StartRoguelikeRoute(u32 seed)
{
    struct RouteMap map;

    GenerateRouteMap(&map, seed);
    gRoguelikeRun.inGym = FALSE;
    gRoguelikeRun.routeSeed = seed;
    gRoguelikeRun.column = 0;
    gRoguelikeRun.row = GetRouteMapStartRow(&map);
    gRoguelikeRun.rocketMoves = 0;
    gRoguelikeRun.visited = 0;
    MarkRouteNodeVisited(gRoguelikeRun.column, gRoguelikeRun.row);
}

bool32 RouteMapCanReach(const struct RouteMap *map, u32 fromColumn, u32 fromRow, u32 toColumn, u32 toRow)
{
    u32 column, row, next;
    u32 reached = 1 << fromRow; // rows reached in the current column

    if (map->type[fromColumn][fromRow] == ROUTE_NODE_EMPTY || toColumn < fromColumn)
        return FALSE;
    for (column = fromColumn; column < toColumn; column++)
    {
        u32 nextReached = 0;

        for (row = 0; row < ROUTE_MAP_ROWS; row++)
        {
            if (!(reached & (1 << row)))
                continue;
            for (next = 0; next < ROUTE_MAP_ROWS; next++)
            {
                if (map->links[column][row] & (1 << next))
                    nextReached |= 1 << next;
            }
        }
        reached = nextReached;
    }
    return (reached & (1 << toRow)) != 0;
}

// Experience a growth curve needs at a level, compared with Medium Fast's
// (level cubed), in percent. Only the four Gen 1 curves differ from 100.
static u32 GetGrowthCurvePercent(u32 growthRate, u32 level)
{
    s32 n = level;
    s32 cube = n * n * n;

    if (n <= 1)
        return 100;
    switch (growthRate)
    {
    case GROWTH_FAST:
        return 80;
    case GROWTH_SLOW:
        return 125;
    case GROWTH_MEDIUM_SLOW:
        return (6 * cube / 5 - 15 * n * n + 100 * n - 140) * 100 / cube;
    default:
        return 100;
    }
}

// Cost of jumping to a tier, in hundredths of a battle point.
u32 GetSnapshotJumpCost(u32 tier, u32 growthRate, u32 targetLevel)
{
    if (tier == 0 || tier >= SNAPSHOT_TIER_COUNT)
        return 0;
    return sSnapshotJumpCost[tier] * GetGrowthCurvePercent(growthRate, targetLevel);
}

bool32 GetNextSnapshotJump(u32 species, u32 level, struct SnapshotJump *jump)
{
    u32 line, form, tier;

    for (line = 0; line < ARRAY_COUNT(sSnapshotLines); line++)
    {
        for (form = 0; form < ARRAY_COUNT(sSnapshotLines[line].species); form++)
        {
            if (sSnapshotLines[line].species[form] == species && species != SPECIES_NONE)
                break;
        }
        if (form == ARRAY_COUNT(sSnapshotLines[line].species))
            continue;
        for (tier = 1; tier < SNAPSHOT_TIER_COUNT; tier++)
        {
            if (sSnapshotLines[line].level[tier] > level)
            {
                jump->tier = tier;
                jump->level = sSnapshotLines[line].level[tier];
                // Never devolve a Pokémon that evolved ahead of its line.
                jump->species = sSnapshotLines[line].species[max(form, sSnapshotLines[line].form[tier])];
                return TRUE;
            }
        }
        return FALSE;
    }
    return FALSE;
}

u32 GetRoguelikeObedienceLevel(u32 badges)
{
    return sObedienceLevels[min(badges, ARRAY_COUNT(sObedienceLevels) - 1)];
}

// The battle points slot for a party Pokémon, claiming a free slot (one whose
// owner has left the party) if it has none yet.
u16 *GetSnapshotPoints(struct Pokemon *mon)
{
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);
    u32 slot, i, freeSlot = PARTY_SIZE;

    for (slot = 0; slot < PARTY_SIZE; slot++)
    {
        if (gRoguelikeRun.pointsOwner[slot] == personality)
            return &gRoguelikeRun.points[slot];
    }
    for (slot = 0; slot < PARTY_SIZE && freeSlot == PARTY_SIZE; slot++)
    {
        freeSlot = slot;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE
             && GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_PERSONALITY) == gRoguelikeRun.pointsOwner[slot])
                freeSlot = PARTY_SIZE;
        }
    }
    if (freeSlot == PARTY_SIZE)
        freeSlot = 0;
    gRoguelikeRun.pointsOwner[freeSlot] = personality;
    gRoguelikeRun.points[freeSlot] = 0;
    return &gRoguelikeRun.points[freeSlot];
}

// Called instead of giving experience: every Pokémon still standing that
// fought the fainted one earns a knockout.
void RoguelikeAwardKnockout(u32 partyBits)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (!(partyBits & (1u << i))
         || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE
         || GetMonData(mon, MON_DATA_HP) == 0)
            continue;
        *GetSnapshotPoints(mon) += SNAPSHOT_POINTS_PER_KNOCKOUT;
    }
}
