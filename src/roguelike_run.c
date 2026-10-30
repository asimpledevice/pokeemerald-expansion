#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "evolution_scene.h"
#include "item.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "party_menu.h"
#include "random.h"
#include "save.h"
#include "task.h"
#include "roguelike.h"
#include "script_pokemon_util.h"
#include "shop.h"
#include "starter_choose.h"
#include "wild_encounter.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "overworld.h"
#include "field_control_avatar.h"
#include "fieldmap.h"
#include "script.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/opponents.h"
#include "constants/species.h"

#if IS_FRLG

/*
 *  Running a route: after the player picks a node on the route map, this
 *  file plays it out (a battle, an event, a Center) and reopens the map.
 *  Battles start straight from here, with no overworld transition, and
 *  return to the route map instead of the field.
 */

// Land encounter slot weights, as in the main games (12 slots).
static const u8 sLandSlotWeights[] = {20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1};

// Placeholder trainers for the slice: Viridian Forest's Bug Catchers, whose
// level 6-8 teams suit a level 7 starter now that fainting is permanent.
static const u16 sSliceTrainers[] =
{
    TRAINER_BUG_CATCHER_RICK,
    TRAINER_BUG_CATCHER_DOUG,
    TRAINER_BUG_CATCHER_ANTHONY,
    TRAINER_BUG_CATCHER_CHARLIE,
};

struct RoguelikeTownInfo
{
    u16 map;
    u8 centerWarp; // warp id of the Pokémon Center door: the player arrives there
    u8 gymWarp;    // warp id of the Gym door: the player leaves the Gym there
    u8 gymTrainers;
    u16 gymTrainerIds[2];
    u16 leader;
    u16 badgeFlag;
};

// Slice data: the two towns a run reaches so far. Viridian's Gym stays closed.
static const struct RoguelikeTownInfo sTowns[ROGUELIKE_TOWN_COUNT] =
{
    [ROGUELIKE_TOWN_VIRIDIAN] =
    {
        .map = MAP_VIRIDIAN_CITY,
        .centerWarp = 0,
        .gymWarp = 2,
    },
    [ROGUELIKE_TOWN_PEWTER] =
    {
        .map = MAP_PEWTER_CITY,
        .centerWarp = 5,
        .gymWarp = 2,
        .gymTrainers = 1,
        .gymTrainerIds = {TRAINER_CAMPER_LIAM},
        .leader = TRAINER_LEADER_BROCK,
        .badgeFlag = FLAG_BADGE01_GET,
    },
};

// The random part of a route Mart's shelf. The design doc's pool also holds
// HMs and tools (Fishing Rod, Bug-Catching Net); they join it once they exist.
static const u16 sMartPool[] =
{
    ITEM_TM_BULLET_SEED,
    ITEM_TM_WATER_PULSE,
    ITEM_TM_ROCK_TOMB,
    ITEM_TM_AERIAL_ACE,
    ITEM_TM_DIG,
    ITEM_TM_PROTECT,
    ITEM_MIRACLE_SEED,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_SILK_SCARF,
};

#define MART_RANDOM_ITEMS 2
static EWRAM_DATA u16 sMartStock[2 + MART_RANDOM_ITEMS + 1] = {0};

// Team Rocket's slice trainers, for overtaken nodes.
static const u16 sRocketTrainers[] =
{
    TRAINER_TEAM_ROCKET_GRUNT,
    TRAINER_TEAM_ROCKET_GRUNT_2,
};

static void CB2_HandleRouteNode(void);
static void CB2_ReturnFromRouteBattle(void);
static void CB2_ProcessJumps(void);
static void LearnSkippedMoves(u32 partyId, u32 species, u32 fromLevel, u32 toLevel, MainCallback then);
static void CB2_LearnQueuedMoves(void);
static void ApplyCatchFloor(void);
static void CB2_EndRun(void);
static void CB2_EndRouteIntoTown(void);
static void CB2_EndGym(void);

static const MainCallback sRouteEndCallbacks[] =
{
    [ROUTE_END_TOWN] = CB2_EndRouteIntoTown,
    [ROUTE_END_GYM] = CB2_EndGym,
    [ROUTE_END_DEBUG_LOOP] = CB2_RoguelikeDebugBoot,
};

static MainCallback GetRouteEndCallback(void)
{
    return sRouteEndCallbacks[gRoguelikeRun.routeEnd];
}

// Where to go once post-battle jumps (and their evolutions) are done.
static EWRAM_DATA MainCallback sAfterJumps = NULL;
static EWRAM_DATA u8 sJumpPartyId = 0;
static EWRAM_DATA u16 sPendingEvolution = SPECIES_NONE;

// Moves a Pokémon reached while it already knew four. Each one opens the
// summary screen's move choice: forget a move for it, or skip it.
#define MAX_QUEUED_MOVES 8
static EWRAM_DATA struct
{
    MainCallback then; // where to go once every queued move is settled
    u16 moves[MAX_QUEUED_MOVES];
    u8 partyId;
    u8 count;
    u8 next;
} sMoveQueue = {0};
static const struct WildPokemonInfo *GetRouteLandMons(u32 map)
{
    u32 i;

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        if (gWildMonHeaders[i].mapGroup == MAP_GROUP(map) && gWildMonHeaders[i].mapNum == MAP_NUM(map))
            return gWildMonHeaders[i].encounterTypes[0].landMonsInfo;
    }
    return NULL;
}

static void StartRouteBattle(void)
{
    gMain.savedCallback = CB2_ReturnFromRouteBattle;
    gBattleEnvironment = BATTLE_ENVIRONMENT_GRASS;
    PlayBattleBGM();
    SetMainCallback2(CB2_InitBattle);
}

static bool32 StartRouteWildBattle(void)
{
    const struct WildPokemonInfo *info = GetRouteLandMons(gRoguelikeRun.routeMapId);
    const struct WildPokemon *mon;
    u32 slot, roll, total = 0, level;

    if (info == NULL)
        return FALSE;
    for (slot = 0; slot < ARRAY_COUNT(sLandSlotWeights); slot++)
        total += sLandSlotWeights[slot];
    roll = Random() % total;
    for (slot = 0; slot < ARRAY_COUNT(sLandSlotWeights) - 1 && roll >= sLandSlotWeights[slot]; slot++)
        roll -= sLandSlotWeights[slot];
    mon = &info->wildPokemon[slot];
    level = mon->minLevel + Random() % (mon->maxLevel - mon->minLevel + 1);

    CreateScriptedWildMon(mon->species, level, ITEM_NONE);
    gBattleTypeFlags = 0;
    StartRouteBattle();
    return TRUE;
}

static void StartRouteTrainerBattle(u32 trainerId)
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    TRAINER_BATTLE_PARAM.opponentB = 0xFFFF;
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], GetTrainerStructFromId(trainerId));
    CalculateEnemyPartyCount();
    StartRouteBattle();
}

static void ReopenRouteMap(u32 message)
{
    gRoguelikeRun.message = message;
    OpenRouteMap(CB2_HandleRouteNode);
}

// Starts a route at its first node and opens its map.
void StartRouteOnMap(u32 seed, u32 routeMapId, u32 routeEnd)
{
    StartRoguelikeRoute(seed);
    gRoguelikeRun.routeMapId = routeMapId;
    gRoguelikeRun.routeEnd = routeEnd;
    gRoguelikeRun.onRouteMap = TRUE;
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}

static void CB2_ReturnFromRouteMart(void)
{
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}

// A route Mart: Potions and Poké Balls always, plus a draw from the pool.
// The draw is seeded by the route and node, so revisiting shows the same shelf.
static void OpenRouteMart(void)
{
    rng_value_t rng = LocalRandomSeed(gRoguelikeRun.routeSeed * 31 + gRoguelikeRun.column * ROUTE_MAP_ROWS + gRoguelikeRun.row);
    u32 count = 0, i;

    sMartStock[count++] = ITEM_POKE_BALL;
    sMartStock[count++] = ITEM_POTION;
    while (count < ARRAY_COUNT(sMartStock) - 1)
    {
        u16 item = sMartPool[LocalRandom(&rng) % ARRAY_COUNT(sMartPool)];

        for (i = 0; i < count && sMartStock[i] != item; i++)
            ;
        if (i == count)
            sMartStock[count++] = item;
    }
    sMartStock[count] = ITEM_NONE;
    CreateBuyMenuFromScreen(sMartStock, CB2_ReturnFromRouteMart);
}

static void CB2_HandleRouteNode(void)
{
    struct RouteMap map;
    u32 column = gRoguelikeRun.column, row = gRoguelikeRun.row;

    GetCurrentRunMap(&map);
    // Team Rocket holds every node it has overtaken, played or not.
    if (!gRoguelikeRun.inGym && RouteNodeIsOvertaken(&map, column, gRoguelikeRun.rocketMoves))
    {
        MarkRouteNodeVisited(column, row);
        StartRouteTrainerBattle(sRocketTrainers[Random() % ARRAY_COUNT(sRocketTrainers)]);
        return;
    }
    // Sideways moves can return to a node: it only plays once, though a
    // Mart opens again.
    if (map.type[column][row] == ROUTE_NODE_MART)
    {
        MarkRouteNodeVisited(column, row);
        OpenRouteMart();
        return;
    }
    if (RouteNodeWasVisited(column, row))
    {
        ReopenRouteMap(ROUTE_MESSAGE_NONE);
        return;
    }
    MarkRouteNodeVisited(column, row);
    switch (map.type[column][row])
    {
    case ROUTE_NODE_WILD:
        if (!StartRouteWildBattle())
            ReopenRouteMap(ROUTE_MESSAGE_NONE);
        break;
    case ROUTE_NODE_TRAINER:
        StartRouteTrainerBattle(sSliceTrainers[Random() % ARRAY_COUNT(sSliceTrainers)]);
        break;
    case ROUTE_NODE_GYM_TRAINER:
        StartRouteTrainerBattle(sTowns[gRoguelikeRun.town].gymTrainerIds[gRoguelikeRun.column - 1]);
        break;
    case ROUTE_NODE_LEADER:
        StartRouteTrainerBattle(sTowns[gRoguelikeRun.town].leader);
        break;
    case ROUTE_NODE_EVENT:
        // Placeholder until events exist.
        AddBagItem(ITEM_POTION, 1);
        ReopenRouteMap(ROUTE_MESSAGE_FOUND_POTION);
        break;
    case ROUTE_NODE_CENTER:
        HealPlayerParty();
        ReopenRouteMap(ROUTE_MESSAGE_HEALED);
        break;
    case ROUTE_NODE_TRANSITION:
        SetMainCallback2(GetRouteEndCallback());
        break;
    default:
        ReopenRouteMap(ROUTE_MESSAGE_NONE);
        break;
    }
}

static void ReopenRouteMapAfterBattle(void)
{
    ReopenRouteMap(gRoguelikeRun.message);
}

// Permadeath: Pokémon that fainted are gone for good. Returns how many.
static u32 RemoveFaintedMons(void)
{
    u32 i, lost = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(mon, MON_DATA_HP) == 0)
        {
            ZeroMonData(mon);
            lost++;
        }
    }
    CompactPartySlots();
    CalculatePlayerPartyCount();
    return lost;
}

static void CB2_ReturnFromRouteBattle(void)
{
    u32 lost;

    CpuFill16(0, (void *)BG_PLTT, BG_PLTT_SIZE);
    ResetOamRange(0, 128);
    lost = RemoveFaintedMons();
    if (gPartiesCount[B_TRAINER_PLAYER] == 0)
    {
        gRoguelikeRun.message = ROUTE_MESSAGE_WIPED_OUT;
        OpenRouteMap(CB2_EndRun);
        return;
    }
    gRoguelikeRun.lostMons = lost;
    gRoguelikeRun.message = lost ? ROUTE_MESSAGE_LOST_MONS : ROUTE_MESSAGE_NONE;
    if (gRoguelikeRun.inGym && gRoguelikeRun.column + 1 == gRoguelikeRun.gymTrainers + 2)
        sAfterJumps = GetRouteEndCallback(); // beat the Leader
    else
        sAfterJumps = ReopenRouteMapAfterBattle;
    sJumpPartyId = 0;
    sMoveQueue.count = sMoveQueue.next = 0;
    sMoveQueue.then = CB2_ProcessJumps;
    if (gBattleOutcome == B_OUTCOME_CAUGHT)
        ApplyCatchFloor();
    SetMainCallback2(CB2_LearnQueuedMoves);
}

// A Pokémon caught below its line's floor joins at the floor (tier 0). The
// newest party member of the caught species is the one just caught; one sent
// to the PC keeps its level for now.
static void ApplyCatchFloor(void)
{
    s32 i;

    for (i = PARTY_SIZE - 1; i >= 0; i--)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 level, floor;

        if (species == SPECIES_NONE || species != gBattleResults.caughtMonSpecies)
            continue;
        level = GetMonData(mon, MON_DATA_LEVEL);
        floor = GetSnapshotCatchFloor(species);
        if (level < floor)
        {
            u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][floor];

            SetMonData(mon, MON_DATA_EXP, &exp);
            CalculateMonStats(mon);
            LearnSkippedMoves(i, species, level, floor, CB2_ProcessJumps);
            level = floor;
        }
        gRoguelikeRun.jumpSpecies = species;
        gRoguelikeRun.jumpLevel = level;
        if (gRoguelikeRun.message == ROUTE_MESSAGE_NONE)
            gRoguelikeRun.message = ROUTE_MESSAGE_CAUGHT;
        return;
    }
}

// Teaches the moves a Pokémon would have learned on the levels it skipped.
// Moves that don't fit wait in the queue for the player's choice, which
// CB2_LearnQueuedMoves runs before going on to `then`.
static void LearnSkippedMoves(u32 partyId, u32 species, u32 fromLevel, u32 toLevel, MainCallback then)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][partyId];
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    u32 i;

    sMoveQueue.partyId = partyId;
    sMoveQueue.count = 0;
    sMoveQueue.next = 0;
    sMoveQueue.then = then;
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].level <= fromLevel || learnset[i].level > toLevel)
            continue;
        if (GiveMoveToMon(mon, learnset[i].move) == MON_HAS_MAX_MOVES && sMoveQueue.count < MAX_QUEUED_MOVES)
            sMoveQueue.moves[sMoveQueue.count++] = learnset[i].move;
    }
}

static void CB2_AfterMoveChoice(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sMoveQueue.partyId];
    u32 slot = GetMoveSlotToReplace();

    if (slot < MAX_MON_MOVES)
    {
        // HMs can't be forgotten: ask again.
        if (CannotForgetMove(GetMonData(mon, MON_DATA_MOVE1 + slot)))
        {
            SetMainCallback2(CB2_LearnQueuedMoves);
            return;
        }
        RemoveMonPPBonus(mon, slot);
        SetMonMoveSlot(mon, sMoveQueue.moves[sMoveQueue.next], slot);
    }
    sMoveQueue.next++;
    SetMainCallback2(CB2_LearnQueuedMoves);
}

static void CB2_LearnQueuedMoves(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sMoveQueue.partyId];

    while (sMoveQueue.next < sMoveQueue.count)
    {
        u16 move = sMoveQueue.moves[sMoveQueue.next];

        if (!MonKnowsMove(mon, move))
        {
            ShowSelectMovePokemonSummaryScreen(gParties[B_TRAINER_PLAYER], sMoveQueue.partyId, CB2_AfterMoveChoice, move);
            return;
        }
        sMoveQueue.next++;
    }
    sMoveQueue.count = sMoveQueue.next = 0;
    SetMainCallback2(sMoveQueue.then);
}

// After a jump's moves are settled: evolve if the line says so, then carry
// on with the rest of the party.
static void CB2_AfterJumpMoves(void)
{
    if (sPendingEvolution != SPECIES_NONE)
    {
        u32 species = sPendingEvolution;

        sPendingEvolution = SPECIES_NONE;
        gCB2_AfterEvolution = CB2_ProcessJumps;
        BeginEvolutionScene(&gParties[B_TRAINER_PLAYER][sMoveQueue.partyId], species, FALSE, sMoveQueue.partyId);
        return;
    }
    SetMainCallback2(CB2_ProcessJumps);
}

// Snapshot jumps after a battle: each Pokémon with enough battle points jumps
// to its next tier, one tier per battle, choosing moves to forget when it
// learns more than it can hold. Evolutions leave for the evolution scene,
// which comes back here for the rest of the party.
static void CB2_ProcessJumps(void)
{
    for (; sJumpPartyId < PARTY_SIZE; sJumpPartyId++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sJumpPartyId];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 level = GetMonData(mon, MON_DATA_LEVEL);
        u32 growthRate = gSpeciesInfo[species].growthRate;
        u32 cost, exp;
        u16 *points;
        struct SnapshotJump jump;

        if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (!GetNextSnapshotJump(species, level, &jump))
            continue;
        points = GetSnapshotPoints(mon);
        cost = GetSnapshotJumpCost(jump.tier, growthRate, jump.level);
        if (*points < cost)
            continue;
        *points -= cost;
        exp = gExperienceTables[growthRate][jump.level];
        SetMonData(mon, MON_DATA_EXP, &exp);
        CalculateMonStats(mon);
        gRoguelikeRun.jumpSpecies = jump.species;
        gRoguelikeRun.jumpLevel = jump.level;
        if (gRoguelikeRun.message == ROUTE_MESSAGE_NONE)
            gRoguelikeRun.message = ROUTE_MESSAGE_JUMPED;
        sPendingEvolution = (jump.species != species) ? jump.species : SPECIES_NONE;
        LearnSkippedMoves(sJumpPartyId++, jump.species, level, jump.level, CB2_AfterJumpMoves);
        SetMainCallback2(CB2_LearnQueuedMoves);
        return;
    }
    SetMainCallback2(sAfterJumps);
}
// Leaves the route or Gym screens for the overworld.
static void WarpToField(u32 map, u32 warpId, s32 x, s32 y)
{
    gRoguelikeRun.onRouteMap = FALSE;
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), warpId, x, y);
    DmaFillLarge16(3, 0, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaFill32Defvars(3, 0, (void *)OAM, OAM_SIZE);
    DmaFill16Defvars(3, 0, (void *)PLTT, PLTT_SIZE);
    WarpIntoMap();
    gFieldCallback = FieldCB_DefaultWarpExit;
    SetMainCallback2(CB2_LoadMap);
}

// Arrives at a warp in a town.
static void WarpIntoTown(u32 town, u32 warpId)
{
    WarpToField(sTowns[town].map, warpId, -1, -1);
}

static void CB2_EndRouteIntoTown(void)
{
    // Viridian's old man blocks the north road until FRLG's tutorial is done.
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2);
    WarpIntoTown(gRoguelikeRun.town, sTowns[gRoguelikeRun.town].centerWarp);
}

static void CB2_EndGym(void)
{
    FlagSet(sTowns[gRoguelikeRun.town].badgeFlag);
    gRoguelikeRun.inGym = FALSE;
    WarpIntoTown(gRoguelikeRun.town, sTowns[gRoguelikeRun.town].gymWarp);
}

static void GiveStarter(u32 species, u32 level)
{
    ZeroPlayerPartyMons();
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], species, level, Random32(), OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(&gParties[B_TRAINER_PLAYER][0]);
    CalculatePlayerPartyCount();
}

// Where a run starts: in front of Prof. Oak, whose lab opens the starter choice.
#define OAKS_LAB_START_X 6
#define OAKS_LAB_START_Y 5

// The party was wiped out: the run is over. The player goes back to Oak's
// lab with no badges, to choose a starter for the next run.
static void CB2_EndRun(void)
{
    u32 flag;

    memset(&gRoguelikeRun, 0, sizeof(gRoguelikeRun));
    for (flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    ZeroPlayerPartyMons();
    CalculatePlayerPartyCount();
    WarpToField(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, WARP_ID_NONE, OAKS_LAB_START_X, OAKS_LAB_START_Y);
}

static void CB2_GiveRoguelikeStarter(void)
{
    u32 species = GetStarterPokemon(gSpecialVar_Result);

    // Starters begin at their line's catch floor: 7, 9 and 8.
    GiveStarter(species, GetSnapshotCatchFloor(species));
    FlagSet(FLAG_SYS_POKEMON_GET);
    gSpecialVar_Result = species;
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// Special, called by Oak's lab when a run has no Pokémon yet: the starter
// choice. VAR_RESULT is the species chosen.
void RoguelikeChooseStarter(void)
{
    SetMainCallback2(CB2_ChooseStarter);
    gMain.savedCallback = CB2_GiveRoguelikeStarter;
}

// The FRLG story never runs in the roguelike: each scene the slice passes
// through is set to its finished state, so no cutscene or errand blocks the
// way (Oak's escort at Pallet's north exit, the lab's starter scene, the
// parcel at Viridian's Mart, the old man on Viridian's north road).
static void SetRoguelikeWorldState(void)
{
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 3);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 9);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 2);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2);
    FlagClear(FLAG_HIDE_OAK_IN_HIS_LAB);
    FlagSet(FLAG_HIDE_RIVAL_IN_LAB);
    FlagSet(FLAG_HIDE_BULBASAUR_BALL);
    FlagSet(FLAG_HIDE_SQUIRTLE_BALL);
    FlagSet(FLAG_HIDE_CHARMANDER_BALL);
    FlagSet(FLAG_HIDE_POKEDEX);
    FlagSet(FLAG_SYS_POKEDEX_GET);
}

// Special, called by a map script after fading out: VAR_0x8004 is the FRLG
// route whose encounters to use, VAR_0x8005 the town the route leads to.
void RoguelikeStartRoute(void)
{
    if (gRoguelikeRun.runSeed == 0)
    {
        gRoguelikeRun.runSeed = Random32() | 1;
        gRoguelikeRun.starterSpecies = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES);
        gRoguelikeRun.starterLevel = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL);
        // A run starts with a few Poké Balls; route Marts sell more.
        AddBagItem(ITEM_POKE_BALL, ROGUELIKE_STARTING_BALLS);
    }
    gRoguelikeRun.stop++;
    gRoguelikeRun.town = gSpecialVar_0x8005;
    StartRouteOnMap(gRoguelikeRun.runSeed + gRoguelikeRun.stop, gSpecialVar_0x8004, ROUTE_END_TOWN);
}

// Special, called by a Gym's map script on entry: VAR_0x8005 is the town.
void RoguelikeStartGym(void)
{
    gRoguelikeRun.town = gSpecialVar_0x8005;
    gRoguelikeRun.inGym = TRUE;
    gRoguelikeRun.gymTrainers = sTowns[gRoguelikeRun.town].gymTrainers;
    gRoguelikeRun.routeEnd = ROUTE_END_GYM;
    gRoguelikeRun.onRouteMap = TRUE;
    gRoguelikeRun.column = 0;
    gRoguelikeRun.row = GYM_MAP_ROW;
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}


bool32 RoguelikeIsOnRouteMap(void)
{
    return RoguelikeIsActive() && gRoguelikeRun.onRouteMap;
}

static void Task_ResumeRoute(u8 taskId)
{
    DestroyTask(taskId);
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}

// Continue on a save made from the route map: the field loads the map the
// route or Gym started from, behind a black screen, and the route map
// reopens straight away. Field controls stay locked so that map's scripts
// don't run in the meantime.
void FieldCB_RoguelikeResumeRoute(void)
{
    LockPlayerFieldControls();
    CreateTask(Task_ResumeRoute, 0);
}

// Saves from the route map. The saved location is still the map the route
// or Gym started from, which Continue loads before reopening the route map.
bool32 RoguelikeSaveOnRouteMap(void)
{
    SaveMapView();
    IncrementGameStat(GAME_STAT_SAVED_GAME);
    return TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
}

static void GiveDebugStarter(void)
{
    if (CalculatePlayerPartyCount() == 0)
        GiveStarter(SPECIES_BULBASAUR, 7);
}

// Debug boot modes 2 and 3: a new game with a name and gender already set.
static void CB2_DebugFieldStart(void)
{
    static const u8 sText_Player[] = _("RED");
    static const u8 sText_Rival[] = _("BLUE");

    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, sText_Player);
    StringCopy_PlayerName(gSaveBlock1Ptr->rivalName, sText_Rival);
    SetMainCallback2(CB2_NewGame);
}

// Called at the end of NewGameInitData, after Oak's speech. Starts a fresh
// run in Oak's lab, where the player chooses a starter. Debug boot mode 2
// instead gives a level 7 Bulbasaur and starts just south of Pallet Town's
// north road; mode 3 starts inside Pewter Gym's door with a level 24 Ivysaur.
void RoguelikeOnNewGame(void)
{
    memset(&gRoguelikeRun, 0, sizeof(gRoguelikeRun));
    SetRoguelikeWorldState();
    if (ROGUELIKE_DEBUG_BOOT == 3)
    {
        GiveStarter(SPECIES_IVYSAUR, 24);
        gRoguelikeRun.runSeed = 1;
        gRoguelikeRun.starterSpecies = SPECIES_BULBASAUR;
        gRoguelikeRun.starterLevel = 7;
        SetWarpDestination(MAP_GROUP(MAP_PEWTER_CITY_GYM), MAP_NUM(MAP_PEWTER_CITY_GYM), WARP_ID_NONE, 6, 13);
    }
    else if (ROGUELIKE_DEBUG_BOOT == 2)
    {
        GiveDebugStarter();
        SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN), MAP_NUM(MAP_PALLET_TOWN), WARP_ID_NONE, 12, 4);
    }
    else
    {
        SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB), MAP_NUM(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB), WARP_ID_NONE, OAKS_LAB_START_X, OAKS_LAB_START_Y);
    }
    WarpIntoMap();
}

// Debug boot (ROGUELIKE_DEBUG_BOOT): mode 1 plays Route 1 maps back to back
// with a level 7 Bulbasaur; mode 2 starts a new game in Pallet Town; mode 3
// starts one at Pewter Gym.
void CB2_RoguelikeDebugBoot(void)
{
    if (ROGUELIKE_DEBUG_BOOT >= 2)
    {
        CB2_DebugFieldStart();
        return;
    }
    GiveDebugStarter();
    gRoguelikeRun.runSeed = 1;
    StartRouteOnMap(gRoguelikeRun.routeSeed + 1, MAP_ROUTE1, ROUTE_END_DEBUG_LOOP);
}

#else

// The roguelike only runs in the FireRed/LeafGreen build.
void CB2_RoguelikeDebugBoot(void)
{
}

void RoguelikeStartRoute(void)
{
}

void RoguelikeStartGym(void)
{
}

void RoguelikeOnNewGame(void)
{
}

bool32 RoguelikeIsOnRouteMap(void)
{
    return FALSE;
}

void RoguelikeChooseStarter(void)
{
}

void FieldCB_RoguelikeResumeRoute(void)
{
}

bool32 RoguelikeSaveOnRouteMap(void)
{
    return FALSE;
}

#endif // IS_FRLG
