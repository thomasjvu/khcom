/* Native field prototype: retain the original CoM projection, collision and art.
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
u8 NativeChestOpen(MapGmk01Work* work) {
    if (!(work->placement->flags & GMK_FLAG_USED)) {
        work->placement->flags |= GMK_FLAG_USED;
        GetMapFloorRoom(gMapFloorState.room)->flags |= FLOOR_ROOM_FLAG_CHEST_OPENED;
        gNativeChests++;
        gGameState.hp += 12;
        if (gGameState.hp > gGameState.progression.maxHp)
            gGameState.hp = gGameState.progression.maxHp;
    }
    gMapRoomState->flags &= ~ROOM_FLAG_ATTACK_HIT;
    work->update = NULL;
    return 1;
}
void NativeEnemyContact(MapEnmWork* work) {
    if (gNativeEnemyFrames && !(work->flags & 0x4000)) {
        work->flags |= 0x4000;
        gGameState.hp -= 4;
        if (gGameState.hp <= 0) { gGameState.hp = 0; gNativeResult = 1; }
    }
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
    line[5] = '0' + gNativeMoveLeft;
    line[11] = '0' + gNativeActionLeft;
    line[16] = line[17] = line[18] = '0';
    while (hp >= 100) { line[16]++; hp -= 100; }
    while (hp >= 10) { line[17]++; hp -= 10; }
    line[18] += hp;
    NativeLabel(0, 0, line);
    NativeLabel(0, 8, gNativeResult == 1 ? "DEFEAT SELECT RETRY" : gNativeResult == 2 ? "RUN CLEAR SELECT RETRY" : gNativeEnemyFrames ? "ENEMY TURN" : "A HIT B JUMP START END");
    location[6] += gNativeFloor < 3 ? gNativeFloor : 2;
    location[13] += gMapFloorState.room >= 10;
    location[14] += gMapFloorState.room >= 10 ? gMapFloorState.room - 10 : gMapFloorState.room;
    NativeLabel(0, 16, location);
    DebugTextDraw(0);
    DebugTextClear();
}
static void NativeExit(void) {
    DebugTextFree();
    Mode_MapFld_2();
}

static void NativeInit(s32 arg) {
    const struct TacWorldRoom* room = &sWorld.rooms[gMapFloorState.room];
    MapFloorRoom* saved = GetMapFloorRoom(gMapFloorState.room);
    MapEnmArgs enemy;
    u8 i;
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
    for (i = 0; i < saved->enemiesLeft; i++) {
        MapEnmSetupArgs(&enemy, gMapEnmDefs[i & 1]);
        TaskCreate(&gFieldState->tasks4, gMapEnmDefs[i & 1]->desc, &enemy);
    }
    gNativeMoveLeft = 3;
    gNativeActionLeft = 1;
    gNativeBusy = 0;
    gNativeEnemyFrames = 0;
    gNativeDirection = 0;
    sFrames = 0;
    sAttack = 0;
    DebugTextInit(0, 0x2000, 0x800);
    DebugTextLoadPalette(0, gWhitePalette, 32, 15);
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
        if (sAttack && MapEnmCheckAttacked(work)) {
            work->flags |= MAP_ENM_FLAG_REMOVED;
            room = GetMapFloorRoom(gMapFloorState.room);
            if (room->enemiesLeft) room->enemiesLeft--;
            TaskKill(&gFieldState->tasks4, task);
            gNativeKills++;
        } else {
            MapEnmUpdateAnim(work);
        }
        node = next;
    }
    gMapRoomState->flags &= ~(ROOM_FLAG_ENEMY_STRUCK | ROOM_FLAG_START_BATTLE);
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
        gNativeFloor = 0;
        gNativeSeed += 0x9e3779b9;
        gNativeKills = gNativeChests = 0;
        gGameState.hp = gGameState.progression.maxHp;
        NativeBuildWorld();
        ModeRequest(&sNativeMode, 0);
        return;
    }
    if (!gNativeResult && !gNativeBusy && !gNativeEnemyFrames &&
        !(gFieldState->flags & (FIELD_FLAG_FREEZE_PLAYER | FIELD_FLAG_ROOM_CREATE))) {
        if (!(pressed & (A_BUTTON | B_BUTTON)) && (pressed & DPAD_ANY) && gNativeMoveLeft) {
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
            edge = pressed & (A_BUTTON | B_BUTTON);
            sAttack = (edge & A_BUTTON) != 0;
            gNativeDirection = sAttack ? 0 : raw & DPAD_ANY;
            if (gNativeDirection && gNativeMoveLeft) gNativeMoveLeft--;
            else gNativeDirection = 0;
            gNativeBusy = 2;
            sFrames = 0;
            gNativeActionLeft--;
            gNativeCommands++;
        } else if (pressed & START_BUTTON) {
            node = gFieldState->tasks4.head.activeHead;
            while (node) {
                task = node->owner;
                ((MapEnmWork*)task->work)->flags &= ~0x4000;
                node = node->next;
            }
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
        gFieldState->flags &= ~FIELD_FLAG_FREEZE_ENEMIES;
        gFieldState->flags |= FIELD_FLAG_FREEZE_PLAYER;
    }
    gFieldState->flags &= ~FIELD_FLAG_ENEMY_FRAME_CHANGED;
    UpdateMapField();
    if (gNativeEnemyFrames) {
        /* Contact is field damage, never a request for the battle scene. */
        if (gMapRoomState->flags & ROOM_FLAG_START_BATTLE) {
            gGameState.hp -= 4;
            if (gGameState.hp <= 0) { gGameState.hp = 0; gNativeResult = 1; }
            node = gFieldState->tasks4.head.activeHead;
            while (node) {
                task = node->owner;
                work = task->work;
                work->flags &= ~MAP_ENM_FLAG_REMOVED;
                ColliderSetDisabled(&work->collider, 0);
                node = node->next;
            }
        }
        gMapRoomState->flags &= ~(ROOM_FLAG_START_BATTLE | ROOM_FLAG_ENEMY_STRUCK);
        gNativeEnemyFrames--;
        if (!gNativeEnemyFrames) {
            gNativeMoveLeft = 3;
            gNativeActionLeft = 1;
            gFieldState->flags &= ~FIELD_FLAG_FREEZE_PLAYER;
        }
    } else NativeEnemies();
    /* No frame-driven spawns: the room owns a fixed seeded encounter. */
    ColliderUpdateAll();
    DrawMapField();
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
    gNativeSeed = 0x434f4d;
    gNativeFloor = 0;
    NativeBuildWorld();
    gGameState.hp = gGameState.progression.maxHp;
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
