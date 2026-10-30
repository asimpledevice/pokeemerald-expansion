#ifndef GUARD_ROGUELIKE_H
#define GUARD_ROGUELIKE_H

#include "main.h"

// Route maps: an FTL-style node map read left to right. Column 0 holds the
// start node, the last column the transition into the next town, and the
// columns in between the route's nodes. Nodes sit on a grid of rows, and
// links only join neighbouring rows, so every path runs straight or at 45°.
#define ROUTE_MAP_COLUMNS 7
#define ROUTE_MAP_ROWS 4
#define ROUTE_MAP_FIRST_MIDDLE_COLUMN 1
#define ROUTE_MAP_LAST_MIDDLE_COLUMN (ROUTE_MAP_COLUMNS - 2)
#define ROUTE_MAP_EXIT_COLUMN (ROUTE_MAP_COLUMNS - 1)

enum RouteNodeType
{
    ROUTE_NODE_EMPTY,
    ROUTE_NODE_START,
    ROUTE_NODE_WILD,
    ROUTE_NODE_TRAINER,
    ROUTE_NODE_EVENT,
    ROUTE_NODE_CENTER,
    ROUTE_NODE_MART,
    ROUTE_NODE_TRANSITION,
    ROUTE_NODE_TYPE_COUNT,
};

struct RouteMap
{
    u8 type[ROUTE_MAP_COLUMNS][ROUTE_MAP_ROWS];  // enum RouteNodeType
    u8 links[ROUTE_MAP_COLUMNS][ROUTE_MAP_ROWS]; // bit n set: links to row n of the next column
};

void GenerateRouteMap(struct RouteMap *map, u32 seed);
bool32 RouteMapCanReach(const struct RouteMap *map, u32 fromColumn, u32 fromRow, u32 toColumn, u32 toRow);
u32 GetRouteMapStartRow(const struct RouteMap *map);

// Debug only: TRUE skips the title screen and opens a route map at boot,
// reopening it after every choice. Keep FALSE in commits.
#define ROGUELIKE_DEBUG_BOOT FALSE

// Team Rocket reaches a route's transition after this many moves: 2.4 times
// the shortest path across the route (FTL's fleet ratio). Tunable.
#define ROUTE_ROCKET_ARRIVAL_MOVES 14

// The current run. Not saved yet: the playable slice keeps it in memory.
struct RoguelikeRun
{
    u32 routeSeed; // the route map is regenerated from this, not stored
    u8 column;     // where the player is on the route map
    u8 row;
    u8 rocketMoves; // moves taken on this route; Team Rocket's pursuit counts these
};

extern struct RoguelikeRun gRoguelikeRun;

void StartRoguelikeRoute(u32 seed);
void OpenRouteMap(MainCallback onNodeChosen);
void CB2_RoguelikeDebugBoot(void);

// Snapshot levelling: Pokémon level in fixed jumps, one per tier of ten levels.
// A jump's cost is counted in battle points (see src/data/roguelike.h) and
// scaled by the Pokémon's original growth curve at the level it jumps to.
#define SNAPSHOT_TIER_COUNT 7 // tier 0 is the catch level, tier 6 is level 60

u32 GetSnapshotJumpCost(u32 tier, u32 growthRate, u32 targetLevel); // hundredths of a battle point

#endif // GUARD_ROGUELIKE_H
