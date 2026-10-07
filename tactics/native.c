/* Native field tactics: retain the original CoM projection, collision and art.
 * US matching RAM layout is fixed by configure.py; these private key variables
 * are injected only after UpdateKeyState, never in the original matching build.
 */
#include "main.h"
#include "intr.h"
#include "key.h"
#include "gba/keys.h"
#include "gba/syscall.h"
#include "gba/io_reg.h"
#include "mode.h"
#include "system_state.h"
#include "game_state.h"
#include "map/map.h"
#include "map/map_runtime.h"
#include "map/field_state.h"
#include "menu/world_types.h"
#include "debug/debug_text.h"
#include "worldgen.h"
#include "anim.h"
#include "obj_api.h"
#include "sprites_evt.h"
#include "sprites_frd.h"
#include "sprites_sora.h"
#include "battle_actor.h"
#include "sprite_palettes.h"
#include "card_def_data.h"
#include "card_ids.h"
#include "sprites_card_pictures.h"
#include "field_deck.h"
#include "field_save.h"
#include "display.h"

typedef char NativeActorOffset[(offsetof(FieldState, actor) == 0x18) ? 1 : -1];
typedef char NativeDoorOffset[(offsetof(MapRoomState, doorRoom) == 0x0f) ? 1 : -1];
typedef char NativeHpOffset[(offsetof(GameState, hp) == 0x32) ? 1 : -1];

#define FIELD_KEYS_HELD (*(vu16*)0x02034000)
#define FIELD_KEYS_PRESSED (*(vu16*)0x02034002)
#define FIELD_KEYS_REPEAT (*(vu16*)0x02034004)

extern MapFloorState gMapFloorState;
extern MapRoomState* gMapRoomState;
extern void Mode_MapFld_0();
extern void Mode_MapFld_2();
extern void MapEnmUpdateAnim(MapEnmWork* work);
extern s32 MapEnmCheckAttacked(MapEnmWork* work);
extern void ColliderUpdateAll();
extern void ColliderSetDisabled(Collider* collider, u8 disabled);

/* Public diagnostic state for emulator replay. */
u32 gNativeCommands;
u32 gNativeKills;
u16 gNativeMoveLeft;
u16 gNativeActionLeft;
u16 gNativeBusy;
u16 gNativeDirection;
u16 gNativeEnemyFrames;
static s32 sStartX, sStartY;
static u16 sFrames;
static u16 sAttack;
static u16 sRawKeys;
static Mode sNativeMode;
static struct TacWorld sWorld;
static MapFloorDef sFloorDef;
static MapEventDoor sNoEvents;
u32 gNativeSeed;
u16 gNativeFloor;
u16 gNativeRoomVisits;
u16 gNativeChests;
u16 gNativeResult;
/* Native assets remain at their matching ROM addresses. Party visuals follow
 * recorded grounded positions, so followers never invent a floor height. */
typedef struct NativeFriend {
    AnimState anim;
    void* tiles;
    ObjPalette* palette;
    FldPos pos;
    u16 pose, timer;
} NativeFriend;
static NativeFriend sFriends[2];
static void* sSoraIdleTiles;
static const AnimDef sFriendAnims[2][2] = {
    {{gDonaFl00Frames,gDonaFl00Anims,gDonaFl00Tiles,0},
     {gDonaBtLl00Frames,gDonaBtLl00Anims,gDonaBtLl00Tiles,0}},
    {{gGoofyFl00Frames,gGoofyFl00Anims,gGoofyFl00Tiles,0},
     {gGoofy16Frames,gGoofy16Anims,gGoofy16Tiles,0}}
};
static void NativePartyPose(u8 member) {
    NativeFriend* friend;
    if (!member) return;
    friend = &sFriends[member - 1];
    friend->pose = 1;
    friend->timer = 48;
    AnimChangeWithDef(sFriendAnims[member - 1], &friend->anim, 1,
        ANIM_FLAG_LOOP, friend->tiles);
}
static FldPos sPartyPos[3];
static u16 sPartyMove[3], sPartyAction[3];
u16 gNativeParty;
static void* sCardTiles[4];
static void* sValueTiles;
static ObjPalette* sValuePalette;
FieldDeck gNativeDeck;
static FieldSaveState sSuspend;
static u8 sSaveBytes[FIELD_SAVE_SIZE * 2];
static u32 sGeneration;
static s16 sSaveSlot;
static u8 sResume;
static u8 sTerminalSaveCleared;
u16 gNativeSaveNotice;
static u16 sPlayedValue;
static Task* sEnemyTasks[6];
u16 gNativeEnemyHp[6];
u16 gNativeBreaks;
u16 gNativeTurn;
static ObjPalette* sCardPalettes[4];
static const CardDef* sCards[4];
u16 gNativeGuard;
static void NativePartyInit(void) {
    u16 i, j;
    static const u16 kinds[4] = {CARD_KIND_KINGDOM_KEY, CARD_KIND_FIRE, CARD_KIND_CURE, CARD_KIND_GOOFY};
    sSoraIdleTiles = AllocObjTiles(0x500, gSor1fl00Tiles);
    sFriends[0].tiles = AllocObjTiles(0x800, gDonaFl00Tiles);
    sFriends[0].palette = LoadObjPalette(gDonaldPalette, 32);
    AnimInit(&sFriends[0].anim, gDonaFl00Anims, gDonaFl00Frames);
    sFriends[1].tiles = AllocObjTiles(0x800, gGoofyFl00Tiles);
    sFriends[1].palette = LoadObjPalette(gGoofyPalette, 32);
    AnimInit(&sFriends[1].anim, gGoofyFl00Anims, gGoofyFl00Frames);
    for (i = 0; i < 2; i++) {
        sFriends[i].pose = sFriends[i].timer = 0;
        AnimStart(&sFriends[i].anim, 0, ANIM_FLAG_LOOP);
        sFriends[i].pos = gFieldState->actor.fieldPosition;
    }
    for (i = 0; i < 3; i++) {
        sPartyPos[i] = gFieldState->actor.fieldPosition;
        sPartyMove[i] = 3;
        sPartyAction[i] = 1;
    }
    for (i = 0; i < 4; i++) {
        sCards[i] = NULL;
        sCardTiles[i] = NULL;
        sCardPalettes[i] = NULL;
        /* US table: 950 records of 52 bytes, verified against the linker map. */
        for (j = 0; j < 950; j++) {
            if (gCardDefs[j].kind == kinds[i] && gCardDefs[j].value == 5) {
                sCards[i] = &gCardDefs[j];
                break;
            }
        }
        if (sCards[i]) {
            sCardTiles[i] = LoadObjTiles(sCards[i]->tiles2, 0x200);
            sCardPalettes[i] = LoadObjPalette(sCards[i]->palette2, 32);
        }
    }
    sValueTiles = LoadObjTiles(gCardValueDigitTiles, 0x1e0);
    sValuePalette = LoadObjPalette(gCard00Palette, 32);
    gNativeParty = 0;
    gNativeGuard = 0;
}
static void NativePartySelect(void) {
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    sPartyMove[gNativeParty] = gNativeMoveLeft;
    sPartyAction[gNativeParty] = gNativeActionLeft;
    gNativeParty = (gNativeParty + 1) % 3;
    gFieldState->actor.fieldPosition = sPartyPos[gNativeParty];
    gFieldState->actor.speed = 0;
    gNativeMoveLeft = sPartyMove[gNativeParty];
    gNativeActionLeft = sPartyAction[gNativeParty];
}
static void NativePartyDraw(void) {
    u8 i;
    s16 x, y;
    int card;
    u8 kind;
    for (i = 0; i < 2; i++) {
        sFriends[i].pos = sPartyPos[i + 1];
        if (sFriends[i].timer) sFriends[i].timer--;
        if (sFriends[i].pose && !sFriends[i].timer && !(i == 1 && gNativeGuard)) {
            sFriends[i].pose = 0;
            AnimChangeWithDef(sFriendAnims[i], &sFriends[i].anim, 0,
                ANIM_FLAG_LOOP, sFriends[i].tiles);
        }
        AnimUpdate(&sFriends[i].anim);
        x = (sFriends[i].pos.x - gFieldState->x) >> 8;
        y = (sFriends[i].pos.y + sFriends[i].pos.z - gFieldState->y) >> 8;
        /* Offset only coincident starting sprites; subsequent positions use
         * the actual trail including ledge and jump heights. */
        if (sFriends[i].pos.x == gFieldState->actor.fieldPosition.x &&
            sFriends[i].pos.y == gFieldState->actor.fieldPosition.y)
            x += i ? 24 : -24;
        if (sFriends[i].tiles && sFriends[i].palette)
            DrawSprite(x, y, AnimGetGfx(&sFriends[i].anim), sFriends[i].tiles,
                sFriends[i].palette, NULL, 0x800,
                -0x1004 - (sFriends[i].pos.y >> 8) * 4);
    }
    for (i = 0; i < FIELD_HAND_MAX; i++) {
        card = FieldDeckHand(&gNativeDeck, i);
        if (card < 0) continue;
        kind = gNativeDeck.kind[card];
        y = i == gNativeDeck.selected ? 134 : 145;
        x = 112 + i * 27;
        if (sCardTiles[kind] && sCardPalettes[kind])
            DrawSprite(x, y, sCards[kind]->gfx2,
                sCardTiles[kind], sCardPalettes[kind], NULL, 0, 2);
        if (sValueTiles && sValuePalette)
            DrawSprite(x - 3, y - 4, gCardValueDigitFrames[gNativeDeck.value[card]],
                sValueTiles, sValuePalette, NULL, 0, 1);
    }
}
static void NativePartyFree(void) {
    u8 i;
    for (i = 0; i < 2; i++) {
        ReleaseObjTiles(sFriends[i].tiles);
        ReleaseObjPalette(sFriends[i].palette);
    }
    ReleaseObjTiles(sSoraIdleTiles);
    ReleaseObjTiles(sValueTiles);
    ReleaseObjPalette(sValuePalette);
    for (i = 0; i < 4; i++) {
        if (sCardTiles[i]) ReleaseObjTiles(sCardTiles[i]);
        if (sCardPalettes[i]) ReleaseObjPalette(sCardPalettes[i]);
    }
}
u8 NativeChestOpen(MapGmk01Work* work) {
    if (!(work->placement->flags & GMK_FLAG_USED)) {
        work->placement->flags |= GMK_FLAG_USED;
        GetMapFloorRoom(gMapFloorState.room)->flags |= FLOOR_ROOM_FLAG_CHEST_OPENED;
        gNativeChests++;
        FieldDeckReward(&gNativeDeck, GetMapFloorRoom(gMapFloorState.room)->seed);
        gGameState.hp += 12;
        if (gGameState.hp > gGameState.progression.maxHp)
            gGameState.hp = gGameState.progression.maxHp;
    }
    gMapRoomState->flags &= ~ROOM_FLAG_ATTACK_HIT;
    work->update = NULL;
    return 1;
}
void NativeEnemyContact(MapEnmWork* work) {
    (void)work;
    /* Contact never starts real-time damage or a separate battle scene. */
    gMapRoomState->flags &= ~(ROOM_FLAG_START_BATTLE | ROOM_FLAG_ENEMY_STRUCK);
}
extern MapFormDef gMapForm;
extern u8 gMapChkUseParams;
extern const MapEnmDef* gMapEnmDefs[];
extern void MapEnmSetupArgs(MapEnmArgs* args, const MapEnmDef* def);
extern void SeedRandom(u32 seed);
extern u8 (*gMapGmkSpotFuncs[14])(FldPos*);
extern void FldPosPlaceAtCell(FldPos* pos, s16 x, s16 y, u8 w, u8 h);
u8 NativeGmkFindSpot(FldPos* pos, u8 finder) {
    s16 x, y;
    pos->x = pos->y = pos->z = pos->ground = 0;
    if (finder < 14 && gMapGmkSpotFuncs[finder](pos)) return 1;
    /* Vanilla mandatory-prop callers ignore placement failure. Generated rooms
     * must never pass uninitialized stack coordinates to a chest or collider. */
    for (y = gMapRoomState->topRow; y <= gMapRoomState->bottomRow; y++) {
        for (x = 0; x < gMapRoomState->cols; x++) {
            if (MapCellIsFreeOfType(x, y, 0)) {
                MapReserveArea(x, y, 1, 1);
                FldPosPlaceAtCell(pos, x, y, 1, 1);
                return 1;
            }
        }
    }
    return 0;
}
u8 NativeGetKeyReleaseTime(u16 key) { return 255; }
const MapFloorDef* NativeGetMapFloorDef(u8 floor) { return &sFloorDef; }
u8* NativeGetMapRoomLinks(u8 room) {
    if (room < TAC_WORLD_ROOMS) return sWorld.links[room];
    return sWorld.links[0];
}
MapEventDoor* NativeGetMapEventDoor(u8 index) { return &sNoEvents; }
static void NativeBuildWorld(void) {
    u8 i;
    MapFloorRoom* room;
    static const u8 worlds[3] = {WORLD_TRAVERSE_TOWN, WORLD_AGRABAH, WORLD_CASTLE_OBLIVION};
    TacWorldGenerate(&sWorld, gNativeSeed, gNativeFloor);
    sFloorDef.entryRoom = 0;
    sFloorDef.exitRoom = 7;
    sFloorDef.links = &sWorld.links[0][0];
    sNoEvents.kind = 5;
    sFloorDef.eventDoors = &sNoEvents;
    gMapFloorState.world = worlds[gNativeFloor % 3];
    gMapFloorState.room = 0;
    gMapFloorState.entrySide = 5;
    gMapFloorState.flags = FLOOR_FLAG_LOGO_SHOWN;
    for (i = 0; i < TAC_WORLD_ROOMS; i++) {
        room = GetMapFloorRoom(i);
        room->flags = FLOOR_ROOM_FLAG_CREATED;
        room->seed = sWorld.rooms[i].seed;
        room->nameId = 0;
        room->roomType = sWorld.rooms[i].chest ? 9 : 0;
        room->cardValue = 5;
        room->enemiesLeft = sWorld.rooms[i].enemies;
        room->przCardsLeft = 0;
    }
}
static void NativeRestoreWorld(void) {
    u8 i;
    gNativeSeed = sSuspend.seed;
    gNativeFloor = sSuspend.floor;
    NativeBuildWorld();
    gMapFloorState.room = sSuspend.room;
    gGameState.hp = sSuspend.hp;
    gNativeDeck = sSuspend.deck;
    gNativeKills = sSuspend.kills;
    gNativeChests = sSuspend.chests;
    gNativeTurn = sSuspend.turn;
    for (i = 0; i < TAC_WORLD_ROOMS; i++) {
        GetMapFloorRoom(i)->flags = sSuspend.roomFlags[i];
        GetMapFloorRoom(i)->enemiesLeft = sSuspend.roomEnemies[i];
    }
}
static void NativeReadSuspend(void) {
    vu8* ram = (vu8*)0x0e000000;
    u16 i;
    for (i = 0; i < FIELD_SAVE_SIZE * 2; i++) sSaveBytes[i] = ram[i];
    sSaveSlot = FieldSaveSelect(&sSuspend, &sGeneration, sSaveBytes,
        sSaveBytes + FIELD_SAVE_SIZE);
    if (sSaveSlot >= 0) {NativeRestoreWorld();sResume = 1;}
}
static void NativeWriteSuspend(void) {
    u8 i, j, n = 0;
    u16 k;
    s16 slot = sSaveSlot == 0 ? 1 : 0;
    MapEnmWork* enemy;
    vu8* ram = (vu8*)(0x0e000000 + slot * FIELD_SAVE_SIZE);
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    sPartyMove[gNativeParty] = gNativeMoveLeft;
    sPartyAction[gNativeParty] = gNativeActionLeft;
    sSuspend.seed = gNativeSeed;
    sSuspend.floor = gNativeFloor;
    sSuspend.room = gMapFloorState.room;
    sSuspend.party = gNativeParty;
    sSuspend.hp = gGameState.hp;
    sSuspend.guard = gNativeGuard;
    sSuspend.turn = gNativeTurn;
    sSuspend.kills = gNativeKills;
    sSuspend.chests = gNativeChests;
    sSuspend.deck = gNativeDeck;
    for (i = 0; i < TAC_WORLD_ROOMS; i++) {
        sSuspend.roomFlags[i] = GetMapFloorRoom(i)->flags;
        sSuspend.roomEnemies[i] = GetMapFloorRoom(i)->enemiesLeft;
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) sSuspend.partyPos[i][j] = ((s32*)&sPartyPos[i])[j];
        sSuspend.move[i] = sPartyMove[i];
        sSuspend.action[i] = sPartyAction[i];
    }
    for (i = 0; i < 6; i++) {
        sSuspend.enemyHp[i] = sSuspend.enemyKind[i] = 0;
        for (j = 0; j < 4; j++) sSuspend.enemyPos[i][j] = 0;
    }
    for (i = 0; i < 6; i++) if (sEnemyTasks[i]) {
        enemy = sEnemyTasks[i]->work;
        for (j = 0; j < 4; j++) sSuspend.enemyPos[n][j] = ((s32*)&enemy->obj.fieldPosition)[j];
        sSuspend.enemyHp[n] = gNativeEnemyHp[i];
        sSuspend.enemyKind[n] = enemy->def == gMapEnmDefs[1];
        n++;
    }
    gNativeSaveNotice = 2;
    if (!FieldSaveEncode(&sSuspend, sGeneration + 1, sSaveBytes)) return;
    for (k = 0; k < 4; k++) ram[k] = 0;
    for (k = 4; k < FIELD_SAVE_SIZE; k++) ram[k] = sSaveBytes[k];
    for (k = 0; k < 4; k++) ram[k] = sSaveBytes[k];
    for (k = 0; k < FIELD_SAVE_SIZE; k++) if (ram[k] != sSaveBytes[k]) return;
    sGeneration++;
    sSaveSlot = slot;
    gNativeSaveNotice = 1;
}

extern const char gWhitePalette[32];
extern void MapEnmSetAnim(MapEnmWork* work, u8 index, u16 flags);
static void NativeLabel(u8 x, u8 y, const char* text) {
    char encoded[80];
    u8 i = 0;
    while (*text && i < 76) {
        if (*text == ' ') { encoded[i++] = 0x81; encoded[i++] = 0x40; }
        else { encoded[i++] = 0x82;
            if (*text >= '0' && *text <= '9') encoded[i++] = *text - '0' + 0x4f;
            else encoded[i++] = *text - 'A' + 0x60;
        }
        text++;
    }
    encoded[i] = 0;
    DebugTextPrint(x, y, 0, encoded);
}
static void NativeHud(void) {
    char line[] = "MOVE 3 ACT 1 HP 000";
    char location[] = "FLOOR 1 ROOM 00";
    u16 hp = gGameState.hp;
    int card = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
    static const char* const names[4] = {"KEYBLADE A PLAY", "FIRE A PLAY", "CURE A PLAY", "GUARD A PLAY"};
    vu16* screen = GetBgScreenBase(0);
    u16 i;
    for (i = 0; i < 1024; i++) screen[i] = 0;
    /* Darken only the scenery under the HUD; card and actor OBJ art stays
     * full color. Outside these two windows the field has no blend effect. */
    gDispCnt |= 0x6000;
    gWin0H = gWin1H = 240;
    gWin0V = 26;
    gWin1V = (128 << 8) | 160;
    gWinIn = 0x3f3f;
    gWinOut = 0x1e;
    gBldCnt = 0x00ee;
    gBldY = 10;

    line[5] = '0' + gNativeMoveLeft;
    line[11] = '0' + gNativeActionLeft;
    line[16] = line[17] = line[18] = '0';
    while (hp >= 100) { line[16]++; hp -= 100; }
    while (hp >= 10) { line[17]++; hp -= 10; }
    line[18] += hp;
    NativeLabel(0, 0, line);
    NativeLabel(0, 8, gNativeResult == 1 ? "DEFEAT SELECT RETRY" : gNativeResult == 2 ? "RUN CLEAR SELECT RETRY" : gNativeEnemyFrames ? "ENEMY TURN" : card < 0 ? "EMPTY L R RELOAD" : names[gNativeDeck.kind[card]]);
    location[6] += gNativeFloor < 3 ? gNativeFloor : 2;
    location[13] += gMapFloorState.room >= 10;
    location[14] += gMapFloorState.room >= 10 ? gMapFloorState.room - 10 : gMapFloorState.room;
    NativeLabel(0, 16, location);
    NativeLabel(128, 16, gNativeParty == 0 ? "SORA" : gNativeParty == 1 ? "DONALD" : "GOOFY");
    if (gNativeSaveNotice) NativeLabel(0, 32, gNativeSaveNotice == 1 ? "SAVED" : "SAVE FAILED");
    NativeLabel(0, 24, gNativeGuard ? "GUARD" : "L R CARDS");
    DebugTextDraw(0);
    /* DebugText indexes tiles by screen row. Rendering below y=120 from
     * char bank 3 would overflow BG VRAM into actor OBJ tiles. Render the
     * two footer rows in safe tile slots, then move only their tilemap cells. */
    for (i = 0; i < 32; i++) {
        screen[19 * 32 + i] = screen[3 * 32 + i];
        screen[18 * 32 + i] = screen[4 * 32 + i];
        screen[3 * 32 + i] = screen[4 * 32 + i] = 0;
    }
    DebugTextClear();
}
static void NativeExit(void) {
    NativePartyFree();
    DebugTextFree();
    Mode_MapFld_2();
}

static void NativeInit(s32 arg) {
    const struct TacWorldRoom* room = &sWorld.rooms[gMapFloorState.room];
    MapFloorRoom* saved = GetMapFloorRoom(gMapFloorState.room);
    MapEnmArgs enemy;
    u8 i, kind;
    gMapForm.layout = room->layout;
    gMapForm.minWidth = gMapForm.maxWidth = room->width;
    gMapForm.minHeight = room->heightMin;
    gMapForm.maxHeight = room->heightMax;
    gMapForm.minDepth = room->depthMin;
    gMapForm.maxDepth = room->depthMax;
    gMapChkUseParams = 1;
    Mode_MapFld_0();
    gMapChkUseParams = 0;
    gNativeRoomVisits++;
    /* Spawn once from the room seed, never from idle frame timing. */
    SeedRandom(room->seed ^ 0x454e4d);
    for (i = 0; i < 6; i++) {sEnemyTasks[i] = NULL; gNativeEnemyHp[i] = 0;}
    for (i = 0; i < saved->enemiesLeft && i < 6; i++) {
        kind = sResume ? sSuspend.enemyKind[i] : i & 1;
        MapEnmSetupArgs(&enemy, gMapEnmDefs[kind]);
        sEnemyTasks[i] = TaskCreate(&gFieldState->tasks4, gMapEnmDefs[kind]->desc, &enemy);
        gNativeEnemyHp[i] = gMapFloorState.room == 7 && i == 0 ? 24 + gNativeFloor * 8 : 6 + gNativeFloor * 2;
    }
    gNativeMoveLeft = 3;
    gNativeActionLeft = 1;
    gNativeBusy = 0;
    gNativeEnemyFrames = 0;
    gNativeDirection = 0;
    sFrames = 0;
    sAttack = 0;
    NativePartyInit();
    if (sResume) {
        u8 j;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 4; j++) ((s32*)&sPartyPos[i])[j] = sSuspend.partyPos[i][j];
            sPartyMove[i] = sSuspend.move[i];
            sPartyAction[i] = sSuspend.action[i];
        }
        gNativeParty = sSuspend.party;
        gNativeGuard = sSuspend.guard;
        gFieldState->actor.fieldPosition = sPartyPos[gNativeParty];
        gNativeMoveLeft = sPartyMove[gNativeParty];
        gNativeActionLeft = sPartyAction[gNativeParty];
        for (i = 0; i < saved->enemiesLeft; i++) {
            MapEnmWork* work = sEnemyTasks[i]->work;
            for (j = 0; j < 4; j++) ((s32*)&work->obj.fieldPosition)[j] = sSuspend.enemyPos[i][j];
            gNativeEnemyHp[i] = sSuspend.enemyHp[i];
        }
        sResume = 0;
    }
    DebugTextInit(0, 0x2000, 0x800);
    DebugTextLoadPalette(0, gWhitePalette, 32, 15);
}

static void NativeDamageEnemy(Task* task, u16 damage) {
    u8 i;
    MapFloorRoom* room;
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] == task) {
        if (sPlayedValue && sPlayedValue < 3 + gNativeFloor) {
            gNativeBreaks++;
            return;
        }
        if (damage < gNativeEnemyHp[i]) {gNativeEnemyHp[i] -= damage;return;}
        gNativeEnemyHp[i] = 0;
        sEnemyTasks[i] = NULL;
        ((MapEnmWork*)task->work)->flags |= MAP_ENM_FLAG_REMOVED;
        room = GetMapFloorRoom(gMapFloorState.room);
        if (room->enemiesLeft) room->enemiesLeft--;
        TaskKill(&gFieldState->tasks4, task);
        gNativeKills++;
        return;
    }
}
static s32 NativeAbs(s32 n) {return n < 0 ? -n : n;}
static void NativeEnemyTurn(void) {
    u8 i, j, closest;
    s32 distance, best, dx, dy, dz, floor;
    FldPos pos;
    MapCell* cell;
    MapEnmWork* work;
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    gNativeTurn++;
    for (i = 0; i < 6 && !gNativeResult; i++) if (sEnemyTasks[i]) {
        work = sEnemyTasks[i]->work;
        best = 0x7fffffff; closest = 0;
        for (j = 0; j < 3; j++) {
            distance = NativeAbs(work->obj.fieldPosition.x - sPartyPos[j].x) +
                NativeAbs(work->obj.fieldPosition.y + work->obj.fieldPosition.z -
                    sPartyPos[j].y - sPartyPos[j].z);
            if (distance < best) {best = distance; closest = j;}
        }
        dz = NativeAbs(work->obj.fieldPosition.z - sPartyPos[closest].z);
        if (best <= (40 << 8) && dz <= (16 << 8)) {
            gGameState.hp -= gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : 4 + gNativeFloor;
            if (gGameState.hp <= 0) {gGameState.hp = 0;gNativeResult = 1;}
        } else {
            pos = work->obj.fieldPosition;
            dx = sPartyPos[closest].x - pos.x;
            dy = sPartyPos[closest].y + sPartyPos[closest].z - pos.y - pos.z;
            if (NativeAbs(dx) > NativeAbs(dy)) pos.x += dx < 0 ? -4096 : 4096;
            else pos.y += dy < 0 ? -2048 : 2048;
            /* Bound before the vanilla u16 cell conversion. */
            if (pos.x < 0 || pos.y + pos.z < 0 ||
                pos.x >= (gMapRoomState->cols << 13) ||
                pos.y + pos.z >= (gMapRoomState->rows << 12)) continue;
            cell = MapCellAtPos(pos.x, pos.y + pos.z);
            if (!cell || cell->lowerZ == 0x100000) continue;
            floor = GetFldPosFloor(&pos);
            if (NativeAbs(floor - pos.ground) > (16 << 8)) continue;
            pos.y += pos.z - floor;
            pos.z = pos.ground = floor;
            work->obj.fieldPosition = pos;
            ColliderSetPosition(&work->collider, pos.x, pos.y, pos.z);
        }
    }
}

static void NativeEnemies(void) {
    ListNode* node = gFieldState->tasks4.head.activeHead;
    ListNode* next;
    Task* task;
    MapEnmWork* work;
    MapFloorRoom* room;
    while (node != NULL) {
        next = node->next;
        task = node->owner;
        work = task->work;
        if (!(work->flags & 0x8000)) {
            work->flags |= 0x8000;
            MapEnmSetAnim(work, 1, 1);
            ColliderSetDisabled(&work->collider, 0);
        }
        if (sAttack && !(work->flags & 0x2000) && MapEnmCheckAttacked(work)) {
            work->flags |= 0x2000;
            NativeDamageEnemy(task, sPlayedValue ? 5 + sPlayedValue / 2 : 3);
        } else {
            MapEnmUpdateAnim(work);
        }
        node = next;
    }
    gMapRoomState->flags &= ~(ROOM_FLAG_ENEMY_STRUCK | ROOM_FLAG_START_BATTLE);
}

static void NativeFire(void) {
    ListNode* node = gFieldState->tasks4.head.activeHead;
    Task* target = NULL;
    Task* task;
    MapEnmWork* work;
    s32 best = 128 << 8, dx, dy, dz;
    while (node) {
        task = node->owner;
        work = task->work;
        dx = work->obj.fieldPosition.x - gFieldState->actor.fieldPosition.x;
        dy = work->obj.fieldPosition.y - gFieldState->actor.fieldPosition.y;
        dz = work->obj.fieldPosition.z - gFieldState->actor.fieldPosition.z;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dz < 0) dz = -dz;
        if (dx + dy < best && dz <= (24 << 8)) {
            best = dx + dy;
            target = task;
        }
        node = node->next;
    }
    if (target) NativeDamageEnemy(target, 6 + sPlayedValue + (gNativeParty == 1 ? 3 : 0));
}

static void NativeUpdate(void) {
    u16 raw = (~REG_KEYINPUT) & KEYS_MASK;
    u16 pressed = raw & ~sRawKeys;
    u16 held = 0;
    u16 edge = 0;
    s32 dx, dy;
    ListNode* node;
    Task* task;
    MapEnmWork* work;
    FldWork* player = ((Task*)gFieldState->tasks2.head.activeHead->owner)->work;
    sRawKeys = raw;
    if (gNativeResult && (pressed & SELECT_BUTTON)) {
        gNativeResult = 0;
        sTerminalSaveCleared = 0;
        gNativeFloor = 0;
        gNativeSeed += 0x9e3779b9;
        gNativeKills = gNativeChests = 0;
        gGameState.hp = gGameState.progression.maxHp;
        FieldDeckInit(&gNativeDeck);
        NativeBuildWorld();
        ModeRequest(&sNativeMode, 0);
        return;
    }
    if (!gNativeResult && !gNativeBusy && !gNativeEnemyFrames &&
        !(gFieldState->flags & (FIELD_FLAG_FREEZE_PLAYER | FIELD_FLAG_ROOM_CREATE))) {
        if ((raw & (START_BUTTON | SELECT_BUTTON)) == (START_BUTTON | SELECT_BUTTON) &&
            (pressed & (START_BUTTON | SELECT_BUTTON))) {
            NativeWriteSuspend();
        } else if (pressed & SELECT_BUTTON) {
            NativePartySelect();
        } else if ((raw & (L_BUTTON | R_BUTTON)) == (L_BUTTON | R_BUTTON) &&
            (pressed & (L_BUTTON | R_BUTTON)) && gNativeActionLeft) {
            if (FieldDeckReload(&gNativeDeck)) gNativeActionLeft = 0;
        } else if (pressed & L_BUTTON) {
            FieldDeckCycle(&gNativeDeck, -1);
        } else if (pressed & R_BUTTON) {
            FieldDeckCycle(&gNativeDeck, 1);
        } else if (!(pressed & (A_BUTTON | B_BUTTON)) && (pressed & DPAD_ANY) && gNativeMoveLeft) {
            gNativeDirection = raw & DPAD_ANY;
            /* Adjacent directions retain the original isometric diagonal walk. */
            if ((gNativeDirection & (DPAD_UP | DPAD_DOWN)) == (DPAD_UP | DPAD_DOWN))
                gNativeDirection &= ~DPAD_DOWN;
            if ((gNativeDirection & (DPAD_LEFT | DPAD_RIGHT)) == (DPAD_LEFT | DPAD_RIGHT))
                gNativeDirection &= ~DPAD_RIGHT;
            sStartX = gFieldState->actor.fieldPosition.x;
            sStartY = gFieldState->actor.fieldPosition.y;
            gNativeBusy = 1;
            sFrames = 0;
            gNativeMoveLeft--;
            gNativeCommands++;
        } else if ((pressed & (A_BUTTON | B_BUTTON)) && gNativeActionLeft) {
            if (pressed & A_BUTTON) {
                int card = FieldDeckPlay(&gNativeDeck);
                if (card >= 0) {
                    u8 kind = gNativeDeck.kind[card];
                    NativePartyPose(gNativeParty);
                    sPlayedValue = gNativeDeck.value[card];
                    if (kind == FIELD_CARD_CURE) {
                        gGameState.hp += 8 + sPlayedValue + (gNativeParty == 1 ? 8 : 0);
                        if (gGameState.hp > gGameState.progression.maxHp)
                            gGameState.hp = gGameState.progression.maxHp;
                    } else if (kind == FIELD_CARD_GUARD) {
                        gNativeGuard = gNativeParty == 2 ? 2 : 1;
                    } else if (kind == FIELD_CARD_FIRE) {
                        NativeFire();
                    } else {
                        edge = A_BUTTON;
                        sAttack = 1;
                        node = gFieldState->tasks4.head.activeHead;
                        while (node) {
                            ((MapEnmWork*)((Task*)node->owner)->work)->flags &= ~0x2000;
                            node = node->next;
                        }
                        gNativeBusy = 2;
                        sFrames = 0;
                    }
                    gNativeActionLeft = 0;
                    gNativeCommands++;
                }
            } else {
            edge = pressed & (A_BUTTON | B_BUTTON);
            sAttack = (edge & A_BUTTON) != 0;
            gNativeDirection = sAttack ? 0 : raw & DPAD_ANY;
            if (gNativeDirection && gNativeMoveLeft) gNativeMoveLeft--;
            else gNativeDirection = 0;
            gNativeBusy = 2;
            sFrames = 0;
            gNativeActionLeft--;
            gNativeCommands++;
            }
        } else if (pressed & START_BUTTON) {
            node = gFieldState->tasks4.head.activeHead;
            while (node) {
                task = node->owner;
                ((MapEnmWork*)task->work)->flags &= ~0x4000;
                node = node->next;
            }
            NativeEnemyTurn();
            gNativeEnemyFrames = 24;
            gNativeCommands++;
        }
    }
    if (gNativeBusy == 1) {
        dx = gFieldState->actor.fieldPosition.x - sStartX;
        dy = gFieldState->actor.fieldPosition.y - sStartY;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dx + dy >= (24 << 8) || sFrames >= 24) {
            gNativeBusy = 0;
            gNativeDirection = 0;
            if (dx + dy < (4 << 8)) gNativeMoveLeft++;
            gFieldState->actor.speed = 0;
        } else held = gNativeDirection;
    }
    if ((gFieldState->flags & FIELD_FLAG_FREEZE_PLAYER) && !gNativeEnemyFrames)
        edge = pressed & (A_BUTTON | B_BUTTON);
    if (gNativeBusy == 2 && !sAttack) {
        if (sFrames < 16) held = gNativeDirection;
        if (player->state == FLD_STATE_LEDGE_CATCH || player->state == FLD_STATE_LEDGE_HANG) {
            held = raw & DPAD_ANY;
            edge = pressed & (B_BUTTON | DPAD_ANY);
        }
    }
    FIELD_KEYS_HELD = held;
    FIELD_KEYS_PRESSED = edge;
    FIELD_KEYS_REPEAT = 0;
    gFieldState->flags |= FIELD_FLAG_FREEZE_ENEMIES;
    if (gNativeEnemyFrames) {
        gFieldState->flags |= FIELD_FLAG_FREEZE_PLAYER;
    }
    gFieldState->flags &= ~FIELD_FLAG_ENEMY_FRAME_CHANGED;
    UpdateMapField();
    if (gNativeEnemyFrames) {
        gMapRoomState->flags &= ~(ROOM_FLAG_START_BATTLE | ROOM_FLAG_ENEMY_STRUCK);
        NativeEnemies();
        gNativeEnemyFrames--;
        if (!gNativeEnemyFrames) {
            gNativeMoveLeft = 3;
            gNativeActionLeft = 1;
            sPartyMove[0] = sPartyMove[1] = sPartyMove[2] = 3;
            sPartyAction[0] = sPartyAction[1] = sPartyAction[2] = 1;
                    gNativeGuard = 0;
            gFieldState->flags &= ~FIELD_FLAG_FREEZE_PLAYER;
        }
    } else NativeEnemies();
    /* No frame-driven spawns: the room owns a fixed seeded encounter. */
    ColliderUpdateAll();
    if (gNativeResult && !sTerminalSaveCleared) {
        vu8* ram = (vu8*)0x0e000000;
        u8 i;
        for (i = 0; i < 4; i++) ram[i] = ram[FIELD_SAVE_SIZE + i] = 0;
        sSaveSlot = -1;
        sGeneration = 0;
        gNativeSaveNotice = 0;
        sTerminalSaveCleared = 1;
    }
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    /* The original player controller supplies collision and camera for the
     * active member; draw Sora at his own stored position. */
    gFieldState->actor.fieldPosition = sPartyPos[0];
    if (gNativeParty) gMapRoomState->flags |= ROOM_FLAG_HIDE_PLAYER;
    DrawMapField();
    if (gNativeParty) {
        s16 x = (sPartyPos[0].x - gFieldState->x) >> 8;
        s16 y = (sPartyPos[0].y + sPartyPos[0].z - gFieldState->y) >> 8;
        if (sSoraIdleTiles)
            DrawSprite(x, y, gSor1fl00Frames[0], sSoraIdleTiles,
                player->palette, NULL, 0x800, -0x1004 - (sPartyPos[0].y >> 8) * 4);
        gMapRoomState->flags &= ~ROOM_FLAG_HIDE_PLAYER;
    }
    gFieldState->actor.fieldPosition = sPartyPos[gNativeParty];
    NativePartyDraw();
    NativeHud();
    if (gNativeBusy) sFrames++;
    if (gNativeBusy == 2 && sFrames >= 48 &&
        !(gFieldState->flags & FIELD_FLAG_PLAYER_JUMPING)) {
        gNativeBusy = 0;
        sAttack = 0;
    }
    if (gFieldState->flags & FIELD_FLAG_EXIT_ROOM) {
        if (gMapFloorState.room == 7 && gMapRoomState->doorRoom == TAC_WORLD_EXIT) {
            if (GetMapFloorRoom(7)->enemiesLeft == 0) {
                gNativeFloor++;
                if (gNativeFloor >= 3) {
                    gNativeResult = 2;
                    gFieldState->flags &= ~FIELD_FLAG_EXIT_ROOM;
                }
                else {
                    NativeBuildWorld();
                    ModeRequest(&sNativeMode, 0);
                }
            } else gFieldState->flags &= ~FIELD_FLAG_EXIT_ROOM;
        } else if (gMapRoomState->doorRoom < TAC_WORLD_ROOMS) {
            SetCurrentMapRoom(gMapRoomState->doorRoom, gMapRoomState->doorSide);
            ModeRequest(&sNativeMode, 0);
        } else gFieldState->flags &= ~FIELD_FLAG_EXIT_ROOM;
    }
}

void TacticsNativeMain(void) {

    InitSystem();
    gFrameCounter = 0;
    ResetGameState();
    SetupSoraNewGame();
    gGameState.progression.tutorialFlags = 0xffff;
    FieldDeckInit(&gNativeDeck);
    gNativeSeed = 0x434f4d;
    gNativeFloor = 0;
    NativeBuildWorld();
    gGameState.hp = gGameState.progression.maxHp;
    NativeReadSuspend();
    sNativeMode.name = "Tactics field";
    sNativeMode.init = NativeInit;
    sNativeMode.update = NativeUpdate;
    sNativeMode.exit = NativeExit;
    ModeRequest(&sNativeMode, 0);
    EnableVBlankIntr();
    for (;;) {
        UpdateKeyState();
        if (!(gFrameSyncFlags & FRAME_SYNC_FRAME_READY)) {
            ModeUpdate();
            gFrameSyncFlags |= FRAME_SYNC_FRAME_READY;
        }
        ApplyIntrCallbacks();
        VBlankIntrWait();
        gFrameCounter++;
    }
}
