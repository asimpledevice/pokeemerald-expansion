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
#include "constants/battle.h"
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

static void StartRouteTrainerBattle(void)
{
    u32 trainerId = sSliceTrainers[Random() % ARRAY_COUNT(sSliceTrainers)];

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

    GenerateRouteMap(&map, gRoguelikeRun.routeSeed);
    switch (map.type[gRoguelikeRun.column][gRoguelikeRun.row])
    {
    case ROUTE_NODE_WILD:
        if (!StartRouteWildBattle())
            ReopenRouteMap(ROUTE_MESSAGE_NONE);
        break;
    case ROUTE_NODE_TRAINER:
        StartRouteTrainerBattle();
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
    ReopenRouteMap(ROUTE_MESSAGE_NONE);
}

// Debug boot (ROGUELIKE_DEBUG_BOOT): play Route 1 maps back to back with a
// level 7 Bulbasaur, starting a new route at each transition.
void CB2_RoguelikeDebugBoot(void)
{
    if (CalculatePlayerPartyCount() == 0)
    {
        CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 7, Random32(), OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
        GiveMonInitialMoveset(&gParties[B_TRAINER_PLAYER][0]);
        CalculatePlayerPartyCount();
    }
    StartRouteOnMap(gRoguelikeRun.routeSeed + 1, MAP_ROUTE1, CB2_RoguelikeDebugBoot);
}

#else

// The roguelike only runs in the FireRed/LeafGreen build.
void CB2_RoguelikeDebugBoot(void)
{
}

#endif // IS_FRLG
