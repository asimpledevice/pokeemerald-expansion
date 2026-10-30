#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "item.h"
#include "main.h"
#include "pokemon.h"
#include "random.h"
#include "roguelike.h"
#include "script_pokemon_util.h"
#include "wild_encounter.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "overworld.h"
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

// Placeholder trainers for the slice: early-route FRLG Youngsters.
static const u16 sSliceTrainers[] =
{
    TRAINER_YOUNGSTER_BEN,
    TRAINER_YOUNGSTER_CALVIN,
    TRAINER_YOUNGSTER_JOSH,
    TRAINER_YOUNGSTER_TIMMY,
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

static void CB2_HandleRouteNode(void);
static void CB2_ReturnFromRouteBattle(void);
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
void StartRouteOnMap(u32 seed, u32 routeMapId, MainCallback onRouteEnd)
{
    StartRoguelikeRoute(seed);
    gRoguelikeRun.routeMapId = routeMapId;
    gRoguelikeRun.onRouteEnd = onRouteEnd;
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}

static void CB2_HandleRouteNode(void)
{
    struct RouteMap map;

    GetCurrentRunMap(&map);
    switch (map.type[gRoguelikeRun.column][gRoguelikeRun.row])
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
    case ROUTE_NODE_MART:
        ReopenRouteMap(ROUTE_MESSAGE_MART_CLOSED);
        break;
    case ROUTE_NODE_TRANSITION:
        SetMainCallback2(gRoguelikeRun.onRouteEnd);
        break;
    default:
        ReopenRouteMap(ROUTE_MESSAGE_NONE);
        break;
    }
}

static void CB2_ReturnFromRouteBattle(void)
{
    CpuFill16(0, (void *)BG_PLTT, BG_PLTT_SIZE);
    ResetOamRange(0, 128);
    if (gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW || gBattleOutcome == B_OUTCOME_FORFEITED)
    {
        // Run over comes with the run rules; for now the party is healed.
        HealPlayerParty();
        ReopenRouteMap(ROUTE_MESSAGE_WIPED_OUT);
        return;
    }
    if (gRoguelikeRun.inGym && gRoguelikeRun.column + 1 == gRoguelikeRun.gymTrainers + 2)
    {
        // Beat the Leader.
        SetMainCallback2(gRoguelikeRun.onRouteEnd);
        return;
    }
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}

// Leaves the route or Gym screens for the overworld, at a warp in a town.
static void WarpIntoTown(u32 town, u32 warpId)
{
    SetWarpDestination(MAP_GROUP(sTowns[town].map), MAP_NUM(sTowns[town].map), warpId, -1, -1);
    DmaFillLarge16(3, 0, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaFill32Defvars(3, 0, (void *)OAM, OAM_SIZE);
    DmaFill16Defvars(3, 0, (void *)PLTT, PLTT_SIZE);
    WarpIntoMap();
    gFieldCallback = FieldCB_DefaultWarpExit;
    SetMainCallback2(CB2_LoadMap);
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

// Special, called by a map script after fading out: VAR_0x8004 is the FRLG
// route whose encounters to use, VAR_0x8005 the town the route leads to.
void RoguelikeStartRoute(void)
{
    if (gRoguelikeRun.runSeed == 0)
        gRoguelikeRun.runSeed = Random32() | 1;
    gRoguelikeRun.stop++;
    gRoguelikeRun.town = gSpecialVar_0x8005;
    StartRouteOnMap(gRoguelikeRun.runSeed + gRoguelikeRun.stop, gSpecialVar_0x8004, CB2_EndRouteIntoTown);
}

// Special, called by a Gym's map script on entry: VAR_0x8005 is the town.
void RoguelikeStartGym(void)
{
    gRoguelikeRun.town = gSpecialVar_0x8005;
    gRoguelikeRun.inGym = TRUE;
    gRoguelikeRun.gymTrainers = sTowns[gRoguelikeRun.town].gymTrainers;
    gRoguelikeRun.onRouteEnd = CB2_EndGym;
    gRoguelikeRun.column = 0;
    gRoguelikeRun.row = GYM_MAP_ROW;
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}


static void GiveDebugStarter(void)
{
    if (CalculatePlayerPartyCount() == 0)
    {
        CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 7, Random32(), OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
        GiveMonInitialMoveset(&gParties[B_TRAINER_PLAYER][0]);
        CalculatePlayerPartyCount();
    }
}

// Debug boot mode 2: a new game with a name and gender already set.
static void CB2_DebugFieldStart(void)
{
    static const u8 sText_Player[] = _("RED");
    static const u8 sText_Rival[] = _("BLUE");

    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, sText_Player);
    StringCopy_PlayerName(gSaveBlock1Ptr->rivalName, sText_Rival);
    SetMainCallback2(CB2_NewGame);
}

// Called at the end of NewGameInitData. Starts a fresh run; in debug boot
// mode 2 it also skips Oak's cutscene, gives a level 7 Bulbasaur and starts
// the player just south of Pallet Town's north road.
void RoguelikeOnNewGame(void)
{
    memset(&gRoguelikeRun, 0, sizeof(gRoguelikeRun));
    if (ROGUELIKE_DEBUG_BOOT == 2)
    {
        VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 3);
        GiveDebugStarter();
        SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN), MAP_NUM(MAP_PALLET_TOWN), WARP_ID_NONE, 12, 4);
        WarpIntoMap();
    }
}

// Debug boot (ROGUELIKE_DEBUG_BOOT): mode 1 plays Route 1 maps back to back
// with a level 7 Bulbasaur; mode 2 starts a new game in Pallet Town.
void CB2_RoguelikeDebugBoot(void)
{
    if (ROGUELIKE_DEBUG_BOOT == 2)
    {
        CB2_DebugFieldStart();
        return;
    }
    GiveDebugStarter();
    StartRouteOnMap(gRoguelikeRun.routeSeed + 1, MAP_ROUTE1, CB2_RoguelikeDebugBoot);
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

#endif // IS_FRLG
