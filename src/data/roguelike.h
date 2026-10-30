// Tunable numbers for the roguelike. Every value here is meant to be changed
// during playtesting; the design doc explains where each one comes from.

// Middle-column node types, drawn by weight. Centers and Marts are bonuses
// (towns guarantee them), so each route holds at most one of each.
static const u8 sRouteNodeTypeWeights[ROUTE_NODE_TYPE_COUNT] =
{
    [ROUTE_NODE_WILD] = 35,
    [ROUTE_NODE_TRAINER] = 30,
    [ROUTE_NODE_EVENT] = 20,
    [ROUTE_NODE_CENTER] = 5,
    [ROUTE_NODE_MART] = 5,
};

// Each middle column holds this many nodes (inclusive).
#define ROUTE_COLUMN_MIN_NODES 2
#define ROUTE_COLUMN_MAX_NODES 4

// Chance, in percent, of dropping an optional link between columns. A link
// is only dropped when both of its nodes keep at least one other link.
#define ROUTE_LINK_DROP_CHANCE 35

// Battle points needed to jump to each tier, for a Medium Fast Pokémon that
// fights every battle. Flat for now: the per-tier costs proposed in the design
// doc are not agreed yet. About 6 fights per stop and one jump every 1.5
// stops gives 9.
static const u8 sSnapshotJumpCost[SNAPSHOT_TIER_COUNT] =
{
    [0] = 0,
    [1] = 9,
    [2] = 9,
    [3] = 9,
    [4] = 9,
    [5] = 9,
    [6] = 9,
};
