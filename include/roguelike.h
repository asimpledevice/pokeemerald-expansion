#ifndef GUARD_ROGUELIKE_H
#define GUARD_ROGUELIKE_H

#include "main.h"
#include "constants/roguelike.h"

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
    ROUTE_NODE_GYM_TRAINER,
    ROUTE_NODE_LEADER,
    ROUTE_NODE_TYPE_COUNT,
};

struct RouteMap
{
    u8 type[ROUTE_MAP_COLUMNS][ROUTE_MAP_ROWS];  // enum RouteNodeType
    u8 links[ROUTE_MAP_COLUMNS][ROUTE_MAP_ROWS]; // bit n set: links to row n of the next column
    u8 sideLinks[ROUTE_MAP_COLUMNS]; // bit n set: rows n and n+1 of this column are linked
    u8 numColumns; // routes use all of them; Gyms are shorter
};

// A Gym: a straight line of Gym trainers ending in the Leader.
#define GYM_MAP_ROW 1

void GenerateRouteMap(struct RouteMap *map, u32 seed);
void GenerateGymMap(struct RouteMap *map, u32 numTrainers);
bool32 RouteMapCanReach(const struct RouteMap *map, u32 fromColumn, u32 fromRow, u32 toColumn, u32 toRow);
u32 GetRouteMapStartRow(const struct RouteMap *map);

// Debug only: 1 skips the title screen and plays route maps back to back;
// 2 starts a new game in Pallet Town past Oak, with a starter; 3 starts one
// at Pewter Gym's door with a level 24 Ivysaur. Keep FALSE in commits.
#define ROGUELIKE_DEBUG_BOOT FALSE

// Team Rocket reaches a route's transition after this many moves: 2.4 times
// the shortest path across the route (FTL's fleet ratio). Tunable.

// The current run. Not saved yet: the playable slice keeps it in memory.
// One-line messages the route map shows after a node.
enum RouteMessage
{
    ROUTE_MESSAGE_NONE,
    ROUTE_MESSAGE_FOUND_POTION,
    ROUTE_MESSAGE_HEALED,
    ROUTE_MESSAGE_MART_CLOSED,
    ROUTE_MESSAGE_WIPED_OUT,
    ROUTE_MESSAGE_JUMPED,
    ROUTE_MESSAGE_LOST_MONS,
};

struct RoguelikeRun
{
    u32 runSeed;   // set when the run starts; every route's seed derives from it
    u32 routeSeed; // the route map is regenerated from this, not stored
    MainCallback onRouteEnd; // runs when the player takes the transition or beats the Leader
    u16 routeMapId; // the FRLG route whose encounters this route uses
    u8 stop;        // routes started so far this run
    u8 town;        // the town this route or Gym belongs to (ROGUELIKE_TOWN_*)
    u8 inGym;       // TRUE while on a Gym map
    u8 gymTrainers; // Gym trainers before the Leader
    u8 column;      // where the player is on the map
    u8 row;
    u8 rocketMoves; // moves taken on this route; Team Rocket's pursuit counts these
    u8 message;     // enum RouteMessage
    u32 visited;    // bit (column * ROUTE_MAP_ROWS + row) set: the node has been played
    u16 starterSpecies; // the run's starter, given again when a run ends
    u8 starterLevel;
    u16 jumpSpecies;    // the last Pokémon to jump, for ROUTE_MESSAGE_JUMPED
    u8 jumpLevel;
    u8 lostMons;        // Pokémon lost in the last battle, for ROUTE_MESSAGE_LOST_MONS
    // Battle points towards each Pokémon's next jump, in hundredths, kept
    // by personality so party order can change.
    u32 pointsOwner[PARTY_SIZE];
    u16 points[PARTY_SIZE];
};

extern struct RoguelikeRun gRoguelikeRun;

void GetCurrentRunMap(struct RouteMap *map);
bool32 RoguelikeIsActive(void);
bool32 RouteNodeWasVisited(u32 column, u32 row);
void MarkRouteNodeVisited(u32 column, u32 row);

// The moves the player can make from a node: forward along links and
// sideways along side links, ordered top to bottom as drawn.
struct RouteChoice
{
    u8 column;
    u8 row;
};
u32 GetRouteMapChoices(const struct RouteMap *map, u32 column, u32 row, struct RouteChoice *choices);
#define ROUTE_MAP_MAX_CHOICES (ROUTE_MAP_ROWS + 2)

// Team Rocket's lead, in moves (tunable). Rocket sweeps the route one column
// per move, so a player who always moves forward is never caught; each detour
// step closes the gap by one. 8 puts Rocket at a 6-move route's exit after 14
// moves, the design doc's 2.4x the main path.
#define ROUTE_ROCKET_LEAD 8

// Team Rocket sweeps the map from the left, one column per move once it has
// arrived. Nodes in columns up to the front are overtaken: entering one
// starts a Rocket battle. Returns -1 while Rocket has yet to arrive.
s32 GetRocketFrontColumn(const struct RouteMap *map, u32 rocketMoves);
bool32 RouteNodeIsOvertaken(const struct RouteMap *map, u32 column, u32 rocketMoves);
void StartRoguelikeRoute(u32 seed);
void StartRouteOnMap(u32 seed, u32 routeMapId, MainCallback onRouteEnd);
void OpenRouteMap(MainCallback onNodeChosen);
void CB2_RoguelikeDebugBoot(void);
void RoguelikeStartRoute(void);
void RoguelikeStartGym(void);
void RoguelikeOnNewGame(void);

// Snapshot levelling: Pokémon level in fixed jumps, one per tier of ten levels.
// A jump's cost is counted in battle points (see src/data/roguelike.h) and
// scaled by the Pokémon's original growth curve at the level it jumps to.
#define SNAPSHOT_TIER_COUNT 7 // tier 0 is the catch level, tier 6 is level 60

u32 GetSnapshotJumpCost(u32 tier, u32 growthRate, u32 targetLevel); // hundredths of a battle point

struct SnapshotLine
{
    u16 species[3];                 // the line's forms, SPECIES_NONE when shorter
    u8 level[SNAPSHOT_TIER_COUNT];  // tier 0 is the catch floor (0 when exempt)
    u8 form[SNAPSHOT_TIER_COUNT];   // index into species at each tier
};

struct SnapshotJump
{
    u8 tier;
    u8 level;
    u16 species;
};

// The next jump for a Pokémon: the first tier above its level. FALSE when it
// has none (top tier, or not on any snapshot line).
bool32 GetNextSnapshotJump(u32 species, u32 level, struct SnapshotJump *jump);

// Battle points per knockout, in hundredths.
#define SNAPSHOT_POINTS_PER_KNOCKOUT 100
void RoguelikeAwardKnockout(u32 partyBits);
struct Pokemon;
u16 *GetSnapshotPoints(struct Pokemon *mon);

// Obedience by tier: Pokémon above this level may disobey.
u32 GetRoguelikeObedienceLevel(u32 badges);

#endif // GUARD_ROGUELIKE_H
