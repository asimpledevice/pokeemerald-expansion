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
