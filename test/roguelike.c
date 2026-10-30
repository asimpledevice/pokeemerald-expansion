#include "global.h"
#include "roguelike.h"
#include "test/test.h"
#include "constants/pokemon.h"

#define SEEDS_TO_CHECK 200

static u32 CountNodes(const struct RouteMap *map, u32 column, u32 type)
{
    u32 row, count = 0;

    for (row = 0; row < ROUTE_MAP_ROWS; row++)
    {
        if (type == ROUTE_NODE_TYPE_COUNT ? map->type[column][row] != ROUTE_NODE_EMPTY : map->type[column][row] == type)
            count++;
    }
    return count;
}

static void FindNode(const struct RouteMap *map, u32 column, u32 type, u32 *row)
{
    for (*row = 0; *row < ROUTE_MAP_ROWS; (*row)++)
    {
        if (map->type[column][*row] == type)
            return;
    }
}

TEST("GenerateRouteMap gives the same map for the same seed")
{
    struct RouteMap a, b;

    GenerateRouteMap(&a, 12345);
    GenerateRouteMap(&b, 12345);
    EXPECT(memcmp(&a, &b, sizeof(a)) == 0);
}

TEST("GenerateRouteMap has one start and one transition at the ends")
{
    u32 seed;
    struct RouteMap map;

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        EXPECT_EQ(CountNodes(&map, 0, ROUTE_NODE_TYPE_COUNT), 1);
        EXPECT_EQ(CountNodes(&map, 0, ROUTE_NODE_START), 1);
        EXPECT_EQ(CountNodes(&map, ROUTE_MAP_EXIT_COLUMN, ROUTE_NODE_TYPE_COUNT), 1);
        EXPECT_EQ(CountNodes(&map, ROUTE_MAP_EXIT_COLUMN, ROUTE_NODE_TRANSITION), 1);
    }
}

// The last middle column must sit within a row of the exit, so it can hold just one node.
TEST("GenerateRouteMap fills middle columns with 2 to 4 nodes, the last with 1 or more")
{
    u32 seed, column, count;
    struct RouteMap map;

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        for (column = ROUTE_MAP_FIRST_MIDDLE_COLUMN; column <= ROUTE_MAP_LAST_MIDDLE_COLUMN; column++)
        {
            count = CountNodes(&map, column, ROUTE_NODE_TYPE_COUNT);
            EXPECT_GE(count, column == ROUTE_MAP_LAST_MIDDLE_COLUMN ? 1 : 2);
            EXPECT_LE(count, 4);
            EXPECT_EQ(CountNodes(&map, column, ROUTE_NODE_START), 0);
            EXPECT_EQ(CountNodes(&map, column, ROUTE_NODE_TRANSITION), 0);
        }
    }
}

TEST("GenerateRouteMap links only neighbouring rows of real nodes")
{
    u32 seed, column, row, next;
    struct RouteMap map;

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        for (column = 0; column < ROUTE_MAP_EXIT_COLUMN; column++)
        {
            for (row = 0; row < ROUTE_MAP_ROWS; row++)
            {
                if (map.type[column][row] == ROUTE_NODE_EMPTY)
                    EXPECT_EQ(map.links[column][row], 0);
                for (next = 0; next < ROUTE_MAP_ROWS; next++)
                {
                    if (!(map.links[column][row] & (1 << next)))
                        continue;
                    EXPECT_NE(map.type[column + 1][next], ROUTE_NODE_EMPTY);
                    EXPECT_LE(row > next ? row - next : next - row, 1);
                }
            }
        }
    }
}

TEST("GenerateRouteMap lets every node be reached from the start and reach the transition")
{
    u32 seed, column, row, startRow, exitRow;
    struct RouteMap map;

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        FindNode(&map, 0, ROUTE_NODE_START, &startRow);
        FindNode(&map, ROUTE_MAP_EXIT_COLUMN, ROUTE_NODE_TRANSITION, &exitRow);
        for (column = 0; column < ROUTE_MAP_COLUMNS; column++)
        {
            for (row = 0; row < ROUTE_MAP_ROWS; row++)
            {
                if (map.type[column][row] == ROUTE_NODE_EMPTY)
                    continue;
                EXPECT(RouteMapCanReach(&map, 0, startRow, column, row));
                EXPECT(RouteMapCanReach(&map, column, row, ROUTE_MAP_EXIT_COLUMN, exitRow));
            }
        }
    }
}

TEST("GenerateRouteMap places at most one Center and one Mart")
{
    u32 seed, column, centers, marts;
    struct RouteMap map;

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        centers = marts = 0;
        for (column = 0; column < ROUTE_MAP_COLUMNS; column++)
        {
            centers += CountNodes(&map, column, ROUTE_NODE_CENTER);
            marts += CountNodes(&map, column, ROUTE_NODE_MART);
        }
        EXPECT_LE(centers, 1);
        EXPECT_LE(marts, 1);
    }
}

TEST("GetSnapshotJumpCost scales with growth rate")
{
    EXPECT_EQ(GetSnapshotJumpCost(0, GROWTH_MEDIUM_FAST, 7), 0);
    EXPECT_GT(GetSnapshotJumpCost(3, GROWTH_SLOW, 35), GetSnapshotJumpCost(3, GROWTH_MEDIUM_FAST, 35));
    EXPECT_GT(GetSnapshotJumpCost(3, GROWTH_MEDIUM_FAST, 35), GetSnapshotJumpCost(3, GROWTH_FAST, 35));
    // Medium Slow is cheap early and close to Medium Fast late.
    EXPECT_LT(GetSnapshotJumpCost(1, GROWTH_MEDIUM_SLOW, 14), GetSnapshotJumpCost(1, GROWTH_MEDIUM_FAST, 14));
    EXPECT_GT(GetSnapshotJumpCost(6, GROWTH_MEDIUM_SLOW, 60), GetSnapshotJumpCost(1, GROWTH_MEDIUM_SLOW, 14));
}

TEST("GenerateGymMap is a straight line of Gym trainers ending in the Leader")
{
    u32 trainers, column;
    struct RouteMap map;

    for (trainers = 1; trainers <= 2; trainers++)
    {
        GenerateGymMap(&map, trainers);
        EXPECT_EQ(map.numColumns, trainers + 2);
        EXPECT_EQ(map.type[0][GYM_MAP_ROW], ROUTE_NODE_START);
        for (column = 1; column <= trainers; column++)
            EXPECT_EQ(map.type[column][GYM_MAP_ROW], ROUTE_NODE_GYM_TRAINER);
        EXPECT_EQ(map.type[trainers + 1][GYM_MAP_ROW], ROUTE_NODE_LEADER);
        EXPECT(RouteMapCanReach(&map, 0, GYM_MAP_ROW, trainers + 1, GYM_MAP_ROW));
    }
}

TEST("GenerateRouteMap links sideways only between neighbouring nodes in middle columns")
{
    u32 seed, column, row, sideways = 0;
    struct RouteMap map;

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        EXPECT_EQ(map.sideLinks[0], 0);
        EXPECT_EQ(map.sideLinks[ROUTE_MAP_EXIT_COLUMN], 0);
        for (column = 0; column < ROUTE_MAP_COLUMNS; column++)
        {
            for (row = 0; row < ROUTE_MAP_ROWS; row++)
            {
                if (!(map.sideLinks[column] & (1 << row)))
                    continue;
                sideways++;
                EXPECT_LT(row + 1, ROUTE_MAP_ROWS);
                EXPECT_NE(map.type[column][row], ROUTE_NODE_EMPTY);
                EXPECT_NE(map.type[column][row + 1], ROUTE_NODE_EMPTY);
            }
        }
    }
    EXPECT_GT(sideways, 0);
}

TEST("GetRouteMapChoices lists sideways and forward moves top to bottom")
{
    u32 seed, i, count;
    struct RouteMap map;
    struct RouteChoice choices[ROUTE_MAP_MAX_CHOICES];

    for (seed = 0; seed < SEEDS_TO_CHECK; seed++)
    {
        GenerateRouteMap(&map, seed);
        count = GetRouteMapChoices(&map, 0, GetRouteMapStartRow(&map), choices);
        EXPECT_GT(count, 0);
        for (i = 0; i < count; i++)
            EXPECT_EQ(choices[i].column, 1);
        for (i = 0; i + 1 < count; i++)
            EXPECT_LT(choices[i].row, choices[i + 1].row);
    }
    // A sideways link adds the node above or below to the choices.
    memset(&map, 0, sizeof(map));
    map.numColumns = ROUTE_MAP_COLUMNS;
    map.type[1][1] = map.type[1][2] = map.type[2][1] = ROUTE_NODE_WILD;
    map.links[1][1] = 1 << 1;
    map.sideLinks[1] = 1 << 1;
    count = GetRouteMapChoices(&map, 1, 1, choices);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(choices[0].column, 2);
    EXPECT_EQ(choices[1].column, 1);
    EXPECT_EQ(choices[1].row, 2);
    count = GetRouteMapChoices(&map, 1, 2, choices);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(choices[0].row, 1);
}

TEST("Team Rocket only catches up through sideways moves")
{
    struct RouteMap map;
    u32 column;

    GenerateRouteMap(&map, 1);
    EXPECT_EQ(GetRocketFrontColumn(&map, 0), -1);
    // Moving straight ahead, Rocket stays behind the player.
    for (column = 1; column < ROUTE_MAP_COLUMNS; column++)
        EXPECT(!RouteNodeIsOvertaken(&map, column, column));
    // Enough sideways moves let it reach the player's column.
    EXPECT(RouteNodeIsOvertaken(&map, 2, 2 + ROUTE_ROCKET_LEAD));
    EXPECT(!RouteNodeIsOvertaken(&map, 2, 2 + ROUTE_ROCKET_LEAD - 1));
    EXPECT_EQ(GetRocketFrontColumn(&map, 100), ROUTE_MAP_EXIT_COLUMN);
}

TEST("GetNextSnapshotJump moves up a tier and evolves on schedule")
{
    struct SnapshotJump jump;

    EXPECT(GetNextSnapshotJump(SPECIES_BULBASAUR, 7, &jump));
    EXPECT_EQ(jump.tier, 1);
    EXPECT_EQ(jump.level, 14);
    EXPECT_EQ(jump.species, SPECIES_IVYSAUR);
    EXPECT(GetNextSnapshotJump(SPECIES_IVYSAUR, 24, &jump));
    EXPECT_EQ(jump.level, 32);
    EXPECT_EQ(jump.species, SPECIES_VENUSAUR);
    // An evolved Pokémon never jumps back to an earlier form.
    EXPECT(GetNextSnapshotJump(SPECIES_RATICATE, 10, &jump));
    EXPECT_EQ(jump.species, SPECIES_RATICATE);
    EXPECT(!GetNextSnapshotJump(SPECIES_VENUSAUR, 60, &jump));
    EXPECT(!GetNextSnapshotJump(SPECIES_NONE, 5, &jump));
}

TEST("GetRoguelikeObedienceLevel rises with badges")
{
    EXPECT_EQ(GetRoguelikeObedienceLevel(0), 19);
    EXPECT_EQ(GetRoguelikeObedienceLevel(2), 29);
    EXPECT_EQ(GetRoguelikeObedienceLevel(7), 59);
    EXPECT_GE(GetRoguelikeObedienceLevel(8), MAX_LEVEL);
}
