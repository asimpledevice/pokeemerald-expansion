#include "global.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "roguelike.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

/*
 *  The route map: an FTL-style node map of the current route. The player
 *  picks the next node from the ones linked to where they stand, and the
 *  screen hands control back to the caller, which runs that node.
 *
 *  The art is placeholder: paths, node markers and the cursor are drawn as
 *  tiles generated here, so the slice needs no new graphics files. Nodes sit
 *  4 tiles apart so diagonal paths run at exactly 45 degrees.
 */

#define NODE_SPACING 4 // tiles between node origins, across and down
#define MAP_LEFT 2
#define MAP_TOP 2
#define NODE_X(column) (MAP_LEFT + (column) * NODE_SPACING)
#define NODE_Y(row) (MAP_TOP + (row) * NODE_SPACING)

enum {
    TILE_GROUND,
    TILE_PATH_ACROSS_TOP,
    TILE_PATH_ACROSS_BOTTOM,
    TILE_PATH_DOWN,
    TILE_PATH_UP,
    TILE_PATH_SIDE_LEFT,
    TILE_PATH_SIDE_RIGHT,
    TILE_NODE_TOP_LEFT,
    TILE_NODE_TOP_RIGHT,
    TILE_NODE_BOTTOM_LEFT,
    TILE_NODE_BOTTOM_RIGHT,
    TILE_COUNT,
};

enum {
    COLOR_GROUND = 1,
    COLOR_PATH,
    COLOR_NODE,
    COLOR_OUTLINE,
};

// Background palettes: one per node type (only the node colour differs),
// plus ones for the node the player stands on, nodes already played and
// nodes Team Rocket has overtaken.
#define PAL_PLAYER ROUTE_NODE_TYPE_COUNT
#define PAL_VISITED (ROUTE_NODE_TYPE_COUNT + 1)
#define PAL_ROCKET (ROUTE_NODE_TYPE_COUNT + 2)
#define PAL_TEXT 15

#define TAG_CURSOR 0x5200

enum {
    WIN_INFO,
};

static EWRAM_DATA struct {
    // Graphics buffers first: DMA copies to VRAM need 4-byte alignment.
    u8 tiles[TILE_COUNT * TILE_SIZE_4BPP] __attribute__((aligned(4)));
    u8 cursorTiles[4 * TILE_SIZE_4BPP] __attribute__((aligned(4)));
    u16 tilemap[32 * 32] __attribute__((aligned(4)));
    MainCallback onNodeChosen;
    struct RouteMap map;
    u8 state;
    u8 choice; // index into the moves from the current node
    u8 cursorSpriteId;
} *sRouteMap = NULL;

static void CB2_InitRouteMap(void);
static void CB2_RouteMap(void);
static void VBlankCB_RouteMap(void);
static void SpriteCB_Cursor(struct Sprite *sprite);

static const struct BgTemplate sRouteMapBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }, {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const struct WindowTemplate sRouteMapWindowTemplates[] =
{
    [WIN_INFO] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 16,
        .width = 28,
        .height = 4,
        .paletteNum = PAL_TEXT,
        .baseBlock = 1
    },
    DUMMY_WIN_TEMPLATE
};

static const u16 sNodeColors[PAL_ROCKET + 1] =
{
    [ROUTE_NODE_EMPTY] = RGB(13, 22, 10),
    [ROUTE_NODE_START] = RGB(24, 24, 24),
    [ROUTE_NODE_WILD] = RGB(10, 28, 10),
    [ROUTE_NODE_TRAINER] = RGB(28, 8, 8),
    [ROUTE_NODE_EVENT] = RGB(30, 26, 6),
    [ROUTE_NODE_CENTER] = RGB(31, 16, 22),
    [ROUTE_NODE_MART] = RGB(8, 14, 30),
    [ROUTE_NODE_TRANSITION] = RGB(31, 31, 31),
    [ROUTE_NODE_GYM_TRAINER] = RGB(22, 12, 4),
    [ROUTE_NODE_LEADER] = RGB(20, 6, 26),
    [PAL_PLAYER] = RGB(6, 26, 31),
    [PAL_VISITED] = RGB(16, 18, 15),
    [PAL_ROCKET] = RGB(9, 3, 12),
};

static const u8 *const sNodeNames[ROUTE_NODE_TYPE_COUNT] =
{
    [ROUTE_NODE_EMPTY] = COMPOUND_STRING(""),
    [ROUTE_NODE_START] = COMPOUND_STRING("Start"),
    [ROUTE_NODE_WILD] = COMPOUND_STRING("Wild POKéMON"),
    [ROUTE_NODE_TRAINER] = COMPOUND_STRING("TRAINER"),
    [ROUTE_NODE_EVENT] = COMPOUND_STRING("Event"),
    [ROUTE_NODE_CENTER] = COMPOUND_STRING("POKéMON CENTER"),
    [ROUTE_NODE_MART] = COMPOUND_STRING("POKé MART"),
    [ROUTE_NODE_TRANSITION] = COMPOUND_STRING("Next town"),
    [ROUTE_NODE_GYM_TRAINER] = COMPOUND_STRING("GYM TRAINER"),
    [ROUTE_NODE_LEADER] = COMPOUND_STRING("GYM LEADER"),
};

static const u8 *const sMessages[] =
{
    [ROUTE_MESSAGE_NONE] = NULL,
    [ROUTE_MESSAGE_FOUND_POTION] = COMPOUND_STRING("You found a POTION!"),
    [ROUTE_MESSAGE_HEALED] = COMPOUND_STRING("Your POKéMON are fully healed."),
    [ROUTE_MESSAGE_MART_CLOSED] = COMPOUND_STRING("The MART is closed for now."),
    [ROUTE_MESSAGE_WIPED_OUT] = COMPOUND_STRING("Your team was wiped out."),
};

static const u8 sBlankTile[TILE_SIZE_4BPP] = {0};

static const u16 sCursorPalette[16] = {RGB(0, 0, 0), RGB(31, 31, 31), RGB(2, 2, 2)};

static const struct OamData sOam_Cursor =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Cursor =
{
    .tileTag = TAG_CURSOR,
    .paletteTag = TAG_CURSOR,
    .oam = &sOam_Cursor,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Cursor,
};

static void SetPixel(u8 *tile, u32 x, u32 y, u32 color)
{
    u8 *byte = &tile[y * 4 + x / 2];

    if (x & 1)
        *byte = (*byte & 0x0F) | (color << 4);
    else
        *byte = (*byte & 0xF0) | color;
}

// Squared distance from the centre of a 16x16 marker, in half pixels.
static u32 MarkerDistance(u32 x, u32 y)
{
    s32 dx = 2 * (s32)x - 15, dy = 2 * (s32)y - 15;

    return dx * dx + dy * dy;
}

static void DrawTiles(void)
{
    u8 *tiles = sRouteMap->tiles;
    u32 tile, x, y;

    for (tile = 0; tile < TILE_COUNT; tile++)
    {
        for (y = 0; y < 8; y++)
            for (x = 0; x < 8; x++)
                SetPixel(&tiles[tile * TILE_SIZE_4BPP], x, y, COLOR_GROUND);
    }
    for (x = 0; x < 8; x++)
    {
        SetPixel(&tiles[TILE_PATH_ACROSS_TOP * TILE_SIZE_4BPP], x, 6, COLOR_PATH);
        SetPixel(&tiles[TILE_PATH_ACROSS_TOP * TILE_SIZE_4BPP], x, 7, COLOR_PATH);
        SetPixel(&tiles[TILE_PATH_ACROSS_BOTTOM * TILE_SIZE_4BPP], x, 0, COLOR_PATH);
        SetPixel(&tiles[TILE_PATH_ACROSS_BOTTOM * TILE_SIZE_4BPP], x, 1, COLOR_PATH);
    }
    for (y = 0; y < 8; y++)
    {
        for (x = 0; x < 8; x++)
        {
            if (x == y || x == y + 1 || x + 1 == y)
                SetPixel(&tiles[TILE_PATH_DOWN * TILE_SIZE_4BPP], x, y, COLOR_PATH);
            if (x + y == 7 || x + y == 6 || x + y == 8)
                SetPixel(&tiles[TILE_PATH_UP * TILE_SIZE_4BPP], x, y, COLOR_PATH);
        }
        SetPixel(&tiles[TILE_PATH_SIDE_LEFT * TILE_SIZE_4BPP], 6, y, COLOR_PATH);
        SetPixel(&tiles[TILE_PATH_SIDE_LEFT * TILE_SIZE_4BPP], 7, y, COLOR_PATH);
        SetPixel(&tiles[TILE_PATH_SIDE_RIGHT * TILE_SIZE_4BPP], 0, y, COLOR_PATH);
        SetPixel(&tiles[TILE_PATH_SIDE_RIGHT * TILE_SIZE_4BPP], 1, y, COLOR_PATH);
    }
    // The node marker: a filled circle with an outline, split into four tiles.
    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            u32 quarter = TILE_NODE_TOP_LEFT + (y >= 8 ? 2 : 0) + (x >= 8 ? 1 : 0);
            u32 distance = MarkerDistance(x, y);

            if (distance <= 13 * 13)
                SetPixel(&tiles[quarter * TILE_SIZE_4BPP], x % 8, y % 8, distance <= 10 * 10 ? COLOR_NODE : COLOR_OUTLINE);
        }
    }
    LoadBgTiles(1, tiles, sizeof(sRouteMap->tiles), 0);

    // The cursor: a hollow square, two pixels thick, in 1D sprite tile order.
    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            u32 quarter = (y >= 8 ? 2 : 0) + (x >= 8 ? 1 : 0);

            if (x < 2 || x > 13 || y < 2 || y > 13)
                SetPixel(&sRouteMap->cursorTiles[quarter * TILE_SIZE_4BPP], x % 8, y % 8, (x == 0 || x == 15 || y == 0 || y == 15) ? 2 : 1);
        }
    }
}

static void LoadPalettes(void)
{
    u32 i;
    u16 palette[16] = {0};

    palette[COLOR_GROUND] = sNodeColors[ROUTE_NODE_EMPTY];
    palette[COLOR_PATH] = RGB(26, 22, 14);
    palette[COLOR_OUTLINE] = RGB(3, 3, 3);
    for (i = 0; i <= PAL_ROCKET; i++)
    {
        palette[COLOR_NODE] = sNodeColors[i];
        LoadPalette(palette, BG_PLTT_ID(i), PLTT_SIZE_4BPP);
    }
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(PAL_TEXT), PLTT_SIZE_4BPP);
}

static void DrawRouteMap(void)
{
    const struct RouteMap *map = &sRouteMap->map;
    u32 column, row, next, i;

    FillBgTilemapBufferRect(1, TILE_GROUND, 0, 0, 32, 32, 0);
    for (column = 0; column + 1 < map->numColumns; column++)
    {
        for (row = 0; row < ROUTE_MAP_ROWS; row++)
        {
            u32 x = NODE_X(column), y = NODE_Y(row);

            for (next = 0; next < ROUTE_MAP_ROWS; next++)
            {
                if (!(map->links[column][row] & (1 << next)))
                    continue;
                for (i = 0; i < NODE_SPACING - 2; i++)
                {
                    if (next == row)
                    {
                        FillBgTilemapBufferRect(1, TILE_PATH_ACROSS_TOP, x + 2 + i, y, 1, 1, 0);
                        FillBgTilemapBufferRect(1, TILE_PATH_ACROSS_BOTTOM, x + 2 + i, y + 1, 1, 1, 0);
                    }
                    else if (next > row)
                    {
                        FillBgTilemapBufferRect(1, TILE_PATH_DOWN, x + 2 + i, y + 2 + i, 1, 1, 0);
                    }
                    else
                    {
                        FillBgTilemapBufferRect(1, TILE_PATH_UP, x + 2 + i, y - 1 - i, 1, 1, 0);
                    }
                }
            }
        }
    }
    for (column = 0; column < map->numColumns; column++)
    {
        for (row = 0; row + 1 < ROUTE_MAP_ROWS; row++)
        {
            if (!(map->sideLinks[column] & (1 << row)))
                continue;
            for (i = 0; i < NODE_SPACING - 2; i++)
            {
                FillBgTilemapBufferRect(1, TILE_PATH_SIDE_LEFT, NODE_X(column), NODE_Y(row) + 2 + i, 1, 1, 0);
                FillBgTilemapBufferRect(1, TILE_PATH_SIDE_RIGHT, NODE_X(column) + 1, NODE_Y(row) + 2 + i, 1, 1, 0);
            }
        }
    }
    for (column = 0; column < map->numColumns; column++)
    {
        for (row = 0; row < ROUTE_MAP_ROWS; row++)
        {
            u32 x = NODE_X(column), y = NODE_Y(row);
            u32 palette = map->type[column][row];

            if (palette == ROUTE_NODE_EMPTY)
                continue;
            if (column == gRoguelikeRun.column && row == gRoguelikeRun.row)
                palette = PAL_PLAYER;
            else if (!gRoguelikeRun.inGym && RouteNodeIsOvertaken(map, column, gRoguelikeRun.rocketMoves))
                palette = PAL_ROCKET;
            else if (RouteNodeWasVisited(column, row))
                palette = PAL_VISITED;
            FillBgTilemapBufferRect(1, TILE_NODE_TOP_LEFT, x, y, 1, 1, palette);
            FillBgTilemapBufferRect(1, TILE_NODE_TOP_RIGHT, x + 1, y, 1, 1, palette);
            FillBgTilemapBufferRect(1, TILE_NODE_BOTTOM_LEFT, x, y + 1, 1, 1, palette);
            FillBgTilemapBufferRect(1, TILE_NODE_BOTTOM_RIGHT, x + 1, y + 1, 1, 1, palette);
        }
    }
    CopyBgTilemapBufferToVram(1);
}

static u32 GetChoices(struct RouteChoice *choices)
{
    // A wiped-out run only waits for A.
    if (gRoguelikeRun.message == ROUTE_MESSAGE_WIPED_OUT)
        return 0;
    return GetRouteMapChoices(&sRouteMap->map, gRoguelikeRun.column, gRoguelikeRun.row, choices);
}

static bool32 GetChosenNode(struct RouteChoice *chosen)
{
    struct RouteChoice choices[ROUTE_MAP_MAX_CHOICES];
    u32 count = GetChoices(choices);

    if (count == 0)
        return FALSE;
    *chosen = choices[sRouteMap->choice % count];
    return TRUE;
}

static u8 *GetMessageText(u8 *text)
{
    u8 *end;

    switch (gRoguelikeRun.message)
    {
    case ROUTE_MESSAGE_JUMPED:
        end = StringCopy(text, GetSpeciesName(gRoguelikeRun.jumpSpecies));
        end = StringCopy(end, COMPOUND_STRING(" jumped to Lv. "));
        end = ConvertIntToDecimalStringN(end, gRoguelikeRun.jumpLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
        return StringCopy(end, COMPOUND_STRING("!"));
    case ROUTE_MESSAGE_LOST_MONS:
        end = ConvertIntToDecimalStringN(text, gRoguelikeRun.lostMons, STR_CONV_MODE_LEFT_ALIGN, 1);
        return StringCopy(end, COMPOUND_STRING(" POKéMON lost for good."));
    default:
        return StringCopy(text, sMessages[gRoguelikeRun.message]);
    }
}

static void PrintInfo(void)
{
    u8 text[64];
    u8 *end;
    struct RouteChoice chosen;
    s32 lead = ROUTE_ROCKET_LEAD - (gRoguelikeRun.rocketMoves - gRoguelikeRun.column);
    static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    // The last node's message shows until the cursor moves.
    if (gRoguelikeRun.message != ROUTE_MESSAGE_NONE)
    {
        GetMessageText(text);
    }
    else if (GetChosenNode(&chosen))
    {
        end = StringCopy(text, COMPOUND_STRING("Next: "));
        if (!gRoguelikeRun.inGym && RouteNodeIsOvertaken(&sRouteMap->map, chosen.column, gRoguelikeRun.rocketMoves + 1))
            StringCopy(end, COMPOUND_STRING("TEAM ROCKET"));
        else if (RouteNodeWasVisited(chosen.column, chosen.row))
            StringCopy(end, COMPOUND_STRING("Nothing new"));
        else
            StringCopy(end, sNodeNames[sRouteMap->map.type[chosen.column][chosen.row]]);
    }
    else
    {
        text[0] = EOS;
    }
    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 4, 1, sTextColors, TEXT_SKIP_DRAW, text);
    if (gRoguelikeRun.message == ROUTE_MESSAGE_WIPED_OUT)
    {
        StringCopy(text, COMPOUND_STRING("You are not a POKéMON MASTER."));
    }
    else if (gRoguelikeRun.inGym)
    {
        StringCopy(text, COMPOUND_STRING("GYM: TEAM ROCKET can't follow you here."));
    }
    else if (lead <= 0)
    {
        StringCopy(text, COMPOUND_STRING("TEAM ROCKET has caught up with you!"));
    }
    else
    {
        // Forward moves keep the gap; sideways moves close it.
        end = StringCopy(text, COMPOUND_STRING("ROCKET is "));
        end = ConvertIntToDecimalStringN(end, lead, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(end, lead == 1 ? COMPOUND_STRING(" sideways step behind")
                                  : COMPOUND_STRING(" sideways steps behind"));
    }
    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 4, 13, sTextColors, TEXT_SKIP_DRAW, text);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static void MoveCursorToChoice(void)
{
    struct Sprite *cursor = &gSprites[sRouteMap->cursorSpriteId];
    struct RouteChoice chosen;

    if (!GetChosenNode(&chosen))
    {
        cursor->x = cursor->y = -16;
        return;
    }
    cursor->x = NODE_X(chosen.column) * 8 + 8;
    cursor->y = NODE_Y(chosen.row) * 8 + 8;
}

static void SpriteCB_Cursor(struct Sprite *sprite)
{
    sprite->invisible = (++sprite->data[0] / 16) % 2;
}

void OpenRouteMap(MainCallback onNodeChosen)
{
    SetVBlankCallback(NULL);
    sRouteMap = AllocZeroed(sizeof(*sRouteMap));
    sRouteMap->onNodeChosen = onNodeChosen;
    GetCurrentRunMap(&sRouteMap->map);
    SetMainCallback2(CB2_InitRouteMap);
}

static void CB2_InitRouteMap(void)
{
    struct SpriteSheet cursorSheet;
    struct SpritePalette cursorPalette = {sCursorPalette, TAG_CURSOR};

    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPaletteFade();
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sRouteMapBgTemplates, ARRAY_COUNT(sRouteMapBgTemplates));
    SetBgTilemapBuffer(1, sRouteMap->tilemap);
    InitWindows(sRouteMapWindowTemplates);
    DeactivateAllTextPrinters();

    // Clear the text layer: its tile 0 and tilemap still hold the last screen's graphics.
    LoadBgTiles(0, sBlankTile, sizeof(sBlankTile), 0);
    FillBgTilemapBufferRect(0, 0, 0, 0, 32, 32, PAL_TEXT);

    DrawTiles();
    LoadPalettes();
    DrawRouteMap();

    cursorSheet.data = sRouteMap->cursorTiles;
    cursorSheet.size = sizeof(sRouteMap->cursorTiles);
    cursorSheet.tag = TAG_CURSOR;
    LoadSpriteSheet(&cursorSheet);
    LoadSpritePalette(&cursorPalette);
    sRouteMap->cursorSpriteId = CreateSprite(&sSpriteTemplate_Cursor, 0, 0, 0);
    MoveCursorToChoice();

    PutWindowTilemap(WIN_INFO);
    PrintInfo();
    ScheduleBgCopyTilemapToVram(0);

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
    ShowBg(0);
    ShowBg(1);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_RouteMap);
    SetMainCallback2(CB2_RouteMap);
}

static void VBlankCB_RouteMap(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

enum {
    STATE_FADE_IN,
    STATE_INPUT,
    STATE_FADE_OUT,
};

static void HandleInput(void)
{
    struct RouteChoice choices[ROUTE_MAP_MAX_CHOICES];
    u32 count = GetChoices(choices);

    if (count == 0)
    {
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sRouteMap->state = STATE_FADE_OUT;
        }
        return;
    }
    if (JOY_NEW(DPAD_UP) && count > 1)
    {
        PlaySE(SE_SELECT);
        sRouteMap->choice = (sRouteMap->choice + count - 1) % count;
        gRoguelikeRun.message = ROUTE_MESSAGE_NONE;
        MoveCursorToChoice();
        PrintInfo();
    }
    else if (JOY_NEW(DPAD_DOWN) && count > 1)
    {
        PlaySE(SE_SELECT);
        sRouteMap->choice = (sRouteMap->choice + 1) % count;
        gRoguelikeRun.message = ROUTE_MESSAGE_NONE;
        MoveCursorToChoice();
        PrintInfo();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        gRoguelikeRun.column = choices[sRouteMap->choice % count].column;
        gRoguelikeRun.row = choices[sRouteMap->choice % count].row;
        gRoguelikeRun.rocketMoves++;
        gRoguelikeRun.message = ROUTE_MESSAGE_NONE;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        sRouteMap->state = STATE_FADE_OUT;
    }
}

static void CB2_RouteMap(void)
{
    switch (sRouteMap->state)
    {
    case STATE_FADE_IN:
        if (!gPaletteFade.active)
            sRouteMap->state = STATE_INPUT;
        break;
    case STATE_INPUT:
        HandleInput();
        break;
    case STATE_FADE_OUT:
        if (!gPaletteFade.active)
        {
            MainCallback onNodeChosen = sRouteMap->onNodeChosen;

            FreeAllWindowBuffers();
            TRY_FREE_AND_SET_NULL(sRouteMap);
            SetMainCallback2(onNodeChosen);
            return;
        }
        break;
    }
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}
