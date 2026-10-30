#ifndef GUARD_GLOBAL_ROGUELIKE_H
#define GUARD_GLOBAL_ROGUELIKE_H

// The roguelike's run state, saved in SaveBlock1 (see include/roguelike.h).
// New Game wipes SaveBlock1, which also ends any run.
struct RoguelikeRun
{
    u32 runSeed;    // set when the run starts; every route's seed derives from it
    u32 routeSeed;  // the route map is regenerated from this, not stored
    u16 routeMapId; // the FRLG route whose encounters this route uses
    u8 routeEnd;    // enum RouteEnd: what the transition or the Leader leads to
    u8 onRouteMap:1; // TRUE from a route or Gym's start until the player is back in the field
    u8 inGym:1;      // TRUE while on a Gym map
    u8 stop;        // routes started so far this run
    u8 town;        // the town this route or Gym belongs to (ROGUELIKE_TOWN_*)
    u8 gymTrainers; // Gym trainers before the Leader
    u8 column;      // where the player is on the map
    u8 row;
    u8 rocketMoves; // moves taken on this route; Team Rocket's pursuit counts these
    u8 message;     // enum RouteMessage
    u32 visited;    // bit (column * ROUTE_MAP_ROWS + row) set: the node has been played
    u16 starterSpecies; // the run's starter, given again when a run ends
    u8 starterLevel;
    u8 jumpLevel;
    u16 jumpSpecies;    // the last Pokémon to jump, for ROUTE_MESSAGE_JUMPED
    u8 lostMons;        // Pokémon lost in the last battle, for ROUTE_MESSAGE_LOST_MONS
    // Battle points towards each Pokémon's next jump, in hundredths, kept
    // by personality so party order can change.
    u32 pointsOwner[PARTY_SIZE];
    u16 points[PARTY_SIZE];
};

#endif // GUARD_GLOBAL_ROGUELIKE_H
