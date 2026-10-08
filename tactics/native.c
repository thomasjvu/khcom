/* Native field tactics: retain the original CoM projection, collision and art.
 * US matching RAM layout is fixed by configure.py; these private key variables
 * are injected only after UpdateKeyState, never in the original matching build.
 */
#include "main.h"
#include "intr.h"
#include "engine_math.h"
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
#include "sprites_btl.h"
#include "sprites_sora.h"
#include "sprites_room.h"
#include "sprites_hum.h"
#include "sprites_cloud.h"
#include "task_descriptors.h"
#include "battle_actor.h"
#include "sprite_palettes.h"
#include "card_def_data.h"
#include "card_ids.h"
#include "sprites_card_pictures.h"
#include "field_deck.h"
#include "field_save.h"
#include "field_party.h"
#include "field_route.h"
#include "field_enemy.h"
#include "display.h"
#include "ui_font.h"
#include "character-assets/rally/rally_data.h"
#include "malloc.h"
#include "gba/macro.h"

typedef char NativeActorOffset[(offsetof(FieldState, actor) == 0x18) ? 1 : -1];
typedef char NativeDoorOffset[(offsetof(MapRoomState, doorRoom) == 0x0f) ? 1 : -1];
typedef char NativeHpOffset[(offsetof(GameState, hp) == 0x32) ? 1 : -1];

#define FIELD_KEYS_HELD (*(vu16*)0x02034000)
#define FIELD_KEYS_PRESSED (*(vu16*)0x02034002)
#define FIELD_KEYS_REPEAT (*(vu16*)0x02034004)
/* Verified alongside input symbols by the US hook installer. */
#define FIELD_PROP_PALETTES (*(vu8*)0x02034f79)

extern MapFloorState gMapFloorState;
extern MapRoomState* gMapRoomState;
extern void Mode_MapFld_0();
extern void Mode_MapFld_2();
extern void MapEnmUpdateAnim(MapEnmWork* work);
extern s32 MapEnmCheckAttacked(MapEnmWork* work);
extern void ColliderUpdateAll();
extern void* ColliderGetPool(u32 type);
extern void ColliderSetDisabled(Collider* collider, u8 disabled);
extern void MapSnapCamera(void);
extern TaskDesc gTaskDescMapGmkDmy;
extern TaskDesc gTaskDescMapSpark;

/* Public diagnostic state for emulator replay. */
u32 gNativeCommands;
u32 gNativeKills;
u16 gNativeMoveLeft;
u16 gNativeActionLeft;
u16 gNativeBusy;
u16 gNativeClimbing;
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
u16 gNativeReward, gNativeRewardChoice;
static u32 sRewardSeed;
u16 gNativeResult;
/* Native assets remain at their matching ROM addresses. Party visuals follow
 * recorded grounded positions, so followers never invent a floor height. */
typedef struct NativeFriend {
    AnimState anim;
    void* tiles;
    ObjPalette* palette;
    FldPos pos;
    u16 pose, timer, facing;
} NativeFriend;
static NativeFriend sFriends[2];
static void* sSoraIdleTiles;
static void* sPartyShadowTiles;
static ObjPalette* sPartyShadowPalette;
u16 gNativeFriendPose[2];
extern FieldRoster gNativeRoster;
static u8 NativeHero(u8 slot) {return gNativeRoster.deployed[slot];}
static const AnimDef sFriendAnims[4][5] = {
    {{gDonaFl00Frames,gDonaFl00Anims,gDonaFl00Tiles,0},
     {gDonaBtLl00Frames,gDonaBtLl00Anims,gDonaBtLl00Tiles,0},
     {gDonaFl00Frames,gDonaFl00Anims,gDonaFl00Tiles,1},
     {gDonaFl00Frames,gDonaFl00Anims,gDonaFl00Tiles,3},
     {gDonaFl00Frames,gDonaFl00Anims,gDonaFl00Tiles,3}},
    {{gGoofyFl00Frames,gGoofyFl00Anims,gGoofyFl00Tiles,0},
     {gGoofy16Frames,gGoofy16Anims,gGoofy16Tiles,0},
     {gGoofy01Frames,gGoofy01Anims,gGoofy01Tiles,0},
     {gGoofy05Frames,gGoofy05Anims,gGoofy05Tiles,0},
     {gGoofy05Frames,gGoofy05Anims,gGoofy05Tiles,1}},
    {{gCroudBt00Frames,gCroudBt00Anims,gCroudBt00Tiles,0},
     {gCroud01Frames,gCroud01Anims,gCroud01Tiles,2},
     {gCroudBt01Frames,gCroudBt01Anims,gCroudBt01Tiles,0},
     {gCroudBt02Frames,gCroudBt02Anims,gCroudBt02Tiles,0},
     {gCroudBt02Frames,gCroudBt02Anims,gCroudBt02Tiles,0}},
    {{(void*)sRallyFrames,(void*)sRallyAnims,(void*)sRallyTiles,0},
     {(void*)sRallyFrames,(void*)sRallyAnims,(void*)sRallyTiles,1},
     {(void*)sRallyFrames,(void*)sRallyAnims,(void*)sRallyTiles,2},
     {(void*)sRallyFrames,(void*)sRallyAnims,(void*)sRallyTiles,3},
     {(void*)sRallyFrames,(void*)sRallyAnims,(void*)sRallyTiles,4}}

};
static void NativePartyPose(u8 member) {
    NativeFriend* friend;
    if (!member) return;
    friend = &sFriends[member - 1];
    friend->pose = 1;
    friend->timer = 48;
    AnimChangeWithDef(sFriendAnims[NativeHero(member) - 1], &friend->anim, 1,
        ANIM_FLAG_LOOP, friend->tiles);
}
u16 gNativeProgressReward;
static u16 sProgressHero, sProgressKind;
static const CardDef* sRecruitCard;
static void* sRecruitTiles;
static ObjPalette* sRecruitPalette;
u16 gNativeAssembly;
static u16 sAssemblyChoice, sAssemblyResume;
static const CardDef* sAssemblyDonald;
static void* sAssemblyTiles;
static ObjPalette* sAssemblyPalette;
static const CardDef* sAssemblyOther;
static void* sAssemblyOtherTiles;
static ObjPalette* sAssemblyOtherPalette;
FieldRoster gNativeRoster;
static void* sReachTiles;
u16 gNativeReachCount;
u8 gNativeReachCost[FIELD_ROUTE_CELLS];
/* Small terrain-space diamond: UI geometry, alongside original game artwork. */
static const u32 sReachGraphic[8] = {0x000cc000,0x00c00c00,0x0c0000c0,0xc000000c,
    0xc000000c,0x0c0000c0,0x00c00c00,0x000cc000};
static FldPos sPartyPos[3];
static u16 sPartyMove[3], sPartyAction[3];
static u16 sCarryMove[3], sCarryAction[3];
static u8 sPartyAngle[3], sCarryAngle[3];
static u8 sCarryTurn, sCarryParty, sCarryGuard;
u16 gNativeParty;
FieldParty gNativePartyHealth;
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
/* Preview state is transient; suspend is only available outside a preview. */
u16 gNativePreview;
s16 gNativeRouteCost;
static s16 sCursorX, sCursorY;
static u16 sPlayerPath[FIELD_ROUTE_CELLS];
static u8 sPlayerEdge[FIELD_ROUTE_CELLS];
static u8 sPathIndex, sPathLength;
static s32 sPathStartX, sPathStartY;
static u8 sRoutePlayer;
static u16 sClimbPreviewDirection;
u16 gNativeClimbReachMask;
static void NativePreviewDraw(void);
static void NativePreviewInput(u16 pressed);
static u16 NativeRouteWalk(void);
static u16 NativeRouteClimb(FldWork* player);
static u8 NativeCureTarget(void);
static u16 NativeCureRecovery(u8 target, u8 value);
static Task* NativeFireTarget(void);
static void NativeCycleFireTarget(void);
static void NativeCycleCureTarget(void);
static void NativeCardIntentDraw(void);
s16 gNativeFireTarget;
s16 gNativeFireChoice;
u16 gNativeFireDamage;
extern u16 gNativeSkillDamage[6];
u16 gNativeCureTarget;
u16 gNativeCureHeal;
s16 gNativeCureChoice;

static Task* sEnemyTasks[6];
/* Guard Armor uses its original seven sprite components on the field actor.
 * The field task retains collision, hit detection and destruction ownership. */
static AnimState sArmorAnim[7];
static void* sArmorTiles[7];
static ObjPalette* sArmorPalette;
static TaskDesc sArmorTaskDesc;
extern u8 gNativeEnemyCharge[6];
extern u8 gNativeEnemyKind[6];
static TaskDesc sMarlTaskDesc;
static AnimState sMarlAnim;
static void* sMarlTiles;
static ObjPalette* sMarlPalette;
static u16 sMarlImpact;
u16 gNativeMarlReady, gNativeMarlPose;
static const AnimDef sMarlDefs[3] = {
    {gMaruxhaIdleFrames,gMaruxhaIdleAnims,gMaruxhaIdleTiles,0},
    {gMaruxhaAtk4Frames,gMaruxhaAtk4Anims,gMaruxhaAtk4Tiles,1},
    {gMaruxhaAtk1Frames,gMaruxhaAtk1Anims,gMaruxhaAtk1Tiles,1}
};
static void NativeMarlDraw(void* arg) {
    MapEnmWork* work = arg;
    u8 pose = gNativeEnemyCharge[0] ? 1 : sMarlImpact ? 2 : 0;
    s16 x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
    s16 y = (work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8;
    u16 priority = -0x1004 - (work->obj.fieldPosition.y >> 8) * 4;
    if (pose != gNativeMarlPose) {
        AnimChangeWithDef(sMarlDefs, &sMarlAnim, pose, pose == 2 ? 0 : ANIM_FLAG_LOOP, sMarlTiles);
        gNativeMarlPose = pose;
    }
    if (sMarlImpact) sMarlImpact--;
    DrawSprite(x, y, AnimUpdate(&sMarlAnim), sMarlTiles, sMarlPalette, NULL,
        0x800 | (work->obj.fieldPosition.x < gFieldState->actor.fieldPosition.x ? SPRITE_FLAG_HFLIP : 0), priority);
    work->obj.shadowZ = work->obj.fieldPosition.ground;
    work->obj.shadowPriority = priority + 1;
    TaskPoolDraw(&work->tasks);
}
static void NativeMarlInit(void) {
    MapEnmWork* work;
    AnimHeader* anim;
    u16 size = 0, bytes, frame;
    u8 pose;
    gNativeMarlReady = gNativeMarlPose = sMarlImpact = 0;
    sMarlTiles = NULL;sMarlPalette = NULL;
    if (gNativeFloor != 2 || gMapFloorState.room != 7 || !sEnemyTasks[0] || gNativeEnemyKind[0] != 2) return;
    work = sEnemyTasks[0]->work;
    ReleaseObjTiles(work->tiles);work->tiles = NULL;
    ReleaseObjPalette(work->palette);work->palette = NULL;
    for (pose = 0; pose < 3; pose++) {
        anim = ((AnimHeader**)sMarlDefs[pose].anims)[sMarlDefs[pose].animId];
        for (frame = 0; frame < anim->frameCount; frame++) {
            bytes = GetSpriteTileBytes(((void**)sMarlDefs[pose].gfxTable)[anim->frames[frame].gfxIndex]);
            if (bytes > size) size = bytes;
        }
    }
    sMarlTiles = AllocObjTiles(size, gMaruxhaIdleTiles);
    sMarlPalette = LoadObjPalette(gMaruxhaPalette, 32);
    if (!sMarlTiles || !sMarlPalette) {
        if (sMarlTiles) ReleaseObjTiles(sMarlTiles);
        if (sMarlPalette) ReleaseObjPalette(sMarlPalette);
        sMarlTiles = NULL;sMarlPalette = NULL;
        work->tiles = AllocObjTiles(work->def->tileCount * 32, NULL);
        work->palette = LoadObjPalette(work->def->palette, 32);
        return;
    }
    work->palette = LoadObjPalette(gMaruxhaPalette, 32);
    AnimInit(&sMarlAnim, gMaruxhaIdleAnims, gMaruxhaIdleFrames);
    AnimStart(&sMarlAnim, 0, ANIM_FLAG_LOOP);
    sMarlTaskDesc = *sEnemyTasks[0]->desc;sMarlTaskDesc.draw = NativeMarlDraw;
    sEnemyTasks[0]->desc = &sMarlTaskDesc;gNativeMarlReady = 1;
}
static TaskDesc sCloudTaskDesc;
static AnimState sCloudAnim;
static void* sCloudTiles;
static ObjPalette* sCloudPalette;
static u16 sCloudImpact;
u16 gNativeCloudReady, gNativeCloudPose;
static const AnimDef sCloudDefs[3] = {
 {gCroudBt00Frames,gCroudBt00Anims,gCroudBt00Tiles,0},
 {gCroudBt03Frames,gCroudBt03Anims,gCroudBt03Tiles,0},
 {gCroud01Frames,gCroud01Anims,gCroud01Tiles,2}
};
static void NativeCloudDraw(void* arg) {
    MapEnmWork* work = arg;
    u8 pose = gNativeEnemyCharge[0] ? 1 : sCloudImpact ? 2 : 0;
    s16 x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
    s16 y = (work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8;
    u16 priority = -0x1004 - (work->obj.fieldPosition.y >> 8) * 4;
    if (pose != gNativeCloudPose) {
        AnimChangeWithDef(sCloudDefs, &sCloudAnim, pose, pose == 2 ? 0 : ANIM_FLAG_LOOP, sCloudTiles);
        gNativeCloudPose = pose;
    }
    if (sCloudImpact) sCloudImpact--;
    DrawSprite(x, y, AnimUpdate(&sCloudAnim), sCloudTiles, sCloudPalette, NULL,
        0x800 | (work->obj.fieldPosition.x < gFieldState->actor.fieldPosition.x ? SPRITE_FLAG_HFLIP : 0), priority);
    work->obj.shadowZ = work->obj.fieldPosition.ground;
    work->obj.shadowPriority = priority + 1;
    TaskPoolDraw(&work->tasks);
}
static void NativeCloudInit(void) {
    MapEnmWork* work;
    AnimHeader* anim;
    u16 size = 0, bytes, frame;
    u8 pose;
    gNativeCloudReady = gNativeCloudPose = sCloudImpact = 0;
    sCloudTiles = NULL;sCloudPalette = NULL;
    if (gNativeFloor != 0 || gMapFloorState.room != 9 || !sEnemyTasks[0] || gNativeEnemyKind[0] != 2) return;
    work = sEnemyTasks[0]->work;
    ReleaseObjTiles(work->tiles);work->tiles = NULL;
    ReleaseObjPalette(work->palette);work->palette = NULL;
    for (pose = 0; pose < 3; pose++) {
        anim = ((AnimHeader**)sCloudDefs[pose].anims)[sCloudDefs[pose].animId];
        for (frame = 0; frame < anim->frameCount; frame++) {
            bytes = GetSpriteTileBytes(((void**)sCloudDefs[pose].gfxTable)[anim->frames[frame].gfxIndex]);
            if (bytes > size) size = bytes;
        }
    }
    sCloudTiles = AllocObjTiles(size, gCroudBt00Tiles);
    sCloudPalette = LoadObjPalette(gCroudPalette, 32);
    if (!sCloudTiles || !sCloudPalette) {
        if (sCloudTiles) ReleaseObjTiles(sCloudTiles);
        if (sCloudPalette) ReleaseObjPalette(sCloudPalette);
        sCloudTiles = NULL;sCloudPalette = NULL;
        work->tiles = AllocObjTiles(work->def->tileCount * 32, NULL);
        work->palette = LoadObjPalette(work->def->palette, 32);
        return;
    }
    work->palette = LoadObjPalette(gCroudPalette, 32);
    AnimInit(&sCloudAnim, gCroudBt00Anims, gCroudBt00Frames);
    AnimStart(&sCloudAnim, 0, ANIM_FLAG_LOOP);
    sCloudTaskDesc = *sEnemyTasks[0]->desc;sCloudTaskDesc.draw = NativeCloudDraw;
    sEnemyTasks[0]->desc = &sCloudTaskDesc;gNativeCloudReady = 1;
}
static TaskDesc sJafarTaskDesc;
static void* sJafarTiles[2];
static ObjPalette* sJafarPalette;
static u16 sJafarImpact;
u16 gNativeJafarReady, gNativeJafarPose;
static void NativeJafarDraw(void* arg) {
    MapEnmWork* work = arg;
    s16 x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
    s16 y = (work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8;
    u16 priority = -0x1004 - (work->obj.fieldPosition.y >> 8) * 4;
    gNativeJafarPose = gNativeEnemyCharge[0] || sJafarImpact ? 1 : 0;
    if (sJafarImpact) sJafarImpact--;
    DrawSprite(x, y, gNativeJafarPose ? gJafferLampFrame0 : gJafferFl00Frame0,
        sJafarTiles[gNativeJafarPose], sJafarPalette, NULL, 0x800, priority);
    work->obj.shadowZ = work->obj.fieldPosition.ground;
    work->obj.shadowPriority = priority + 1;
    TaskPoolDraw(&work->tasks);
}
static void NativeJafarInit(void) {
    MapEnmWork* work;
    gNativeJafarReady = gNativeJafarPose = sJafarImpact = 0;
    sJafarTiles[0] = sJafarTiles[1] = NULL;sJafarPalette = NULL;
    if (gNativeFloor != 1 || gMapFloorState.room != 7 || !sEnemyTasks[0] || gNativeEnemyKind[0] != 2) return;
    work = sEnemyTasks[0]->work;
    ReleaseObjTiles(work->tiles);work->tiles = NULL;
    ReleaseObjPalette(work->palette);work->palette = NULL;
    sJafarPalette = LoadObjPalette(gJafferPalette, 32);
    sJafarTiles[0] = AllocObjTiles(GetSpriteTileBytes(gJafferFl00Frame0), gJafferFl00Tiles);
    sJafarTiles[1] = AllocObjTiles(GetSpriteTileBytes(gJafferLampFrame0), gJafferLampTiles);
    if (!sJafarPalette || !sJafarTiles[0] || !sJafarTiles[1]) {
        if (sJafarTiles[0]) ReleaseObjTiles(sJafarTiles[0]);
        if (sJafarTiles[1]) ReleaseObjTiles(sJafarTiles[1]);
        if (sJafarPalette) ReleaseObjPalette(sJafarPalette);
        sJafarTiles[0] = sJafarTiles[1] = NULL;sJafarPalette = NULL;
        work->tiles = AllocObjTiles(work->def->tileCount * 32, NULL);
        work->palette = LoadObjPalette(work->def->palette, 32);
        return;
    }
    work->palette = LoadObjPalette(gJafferPalette, 32);
    sJafarTaskDesc = *sEnemyTasks[0]->desc;
    sJafarTaskDesc.draw = NativeJafarDraw;
    sEnemyTasks[0]->desc = &sJafarTaskDesc;
    gNativeJafarReady = 1;
}
u16 gNativeBossReady;
u16 gNativeBossAllocation;
u16 gNativeBossPose;
static u16 sArmorImpact;
extern u8 gNativeEnemyCharge[6];
extern u16 gNativeEnemyHp[6];
u16 gNativeBossPhase;
u16 gNativeBossBreaks, gNativeBossEffects;
static u8 NativeArmorAnimId(u8 part, u8 pose) {
    if (!pose) return 0;
    if (part == 0) return 2;
    if (part == 2 || part == 3 || part == 6) return 1;
    return 0;
}
static const AnimDef sArmorDefs[7] = {
    {gBosGaTorsoFrames,gBosGaTorsoAnims,gBosGaTorsoTiles,0},
    {gBosGaHeadFrames,gBosGaHeadAnims,gBosGaHeadTiles,0},
    {gBosGaNearHandFrames,gBosGaNearHandAnims,gBosGaNearHandTiles,0},
    {gBosGaFarHandFrames,gBosGaFarHandAnims,gBosGaFarHandTiles,0},
    {gBosGaNearFootFrames,gBosGaNearFootAnims,gBosGaNearFootTiles,0},
    {gBosGaFarFootFrames,gBosGaFarFootAnims,gBosGaFarFootTiles,0},
    {gBosGaCollarFrames,gBosGaCollarAnims,gBosGaCollarTiles,0}
};
static void NativeArmorDraw(void* arg) {
    static const s16 dx[7] = {0,-10,10,-44,15,-13,0};
    static const s16 dy[7] = {-62,-90,-36,-61,-6,-15,-62};
    MapEnmWork* work = arg;
    u8 i, pose;
    s16 offset;
    s16 x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
    s16 y = (work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8;
    u16 priority = -0x1004 - (work->obj.fieldPosition.y >> 8) * 4;
    pose = gNativeEnemyCharge[0] ? 1 : sArmorImpact ? 2 : 0;
    gNativeBossPhase = FieldArmorPhase(gNativeEnemyHp[0]);
    if (pose != gNativeBossPose) {
        for (i = 0; i < 7; i++)
            AnimStart(&sArmorAnim[i], NativeArmorAnimId(i, pose), pose == 2 ? 0 : ANIM_FLAG_LOOP);
        gNativeBossPose = pose;
    }
    if (sArmorImpact) sArmorImpact--;
    for (i = 0; i < 7; i++) {
        if ((i == 3 && gNativeBossPhase >= 1) || (i == 2 && gNativeBossPhase >= 2)) continue;
        offset = 0;
        if (pose == 1) offset = i == 2 || i == 3 ? -12 : i == 4 || i == 5 ? 0 : 8;
        else if (pose == 2 && (i == 2 || i == 3)) offset = 10;
        DrawSprite(x + dx[i], y + dy[i] + offset, AnimUpdate(&sArmorAnim[i]),
            sArmorTiles[i], sArmorPalette, NULL, 0x800, priority);
    }
    work->obj.shadowZ = work->obj.fieldPosition.ground;
    work->obj.shadowPriority = priority + 1;
    TaskPoolDraw(&work->tasks);
}
static void NativeArmorInit(void) {
    u8 i, pose;
    u16 j, size, bytes;
    AnimHeader* idle;
    MapEnmWork* work;
    gNativeBossReady = 0;
    gNativeBossAllocation = 0;
    gNativeBossPose = sArmorImpact = 0;
    gNativeBossPhase = 0;
    gNativeBossBreaks = gNativeBossEffects = 0;
    sArmorPalette = NULL;
    for (i = 0; i < 7; i++) sArmorTiles[i] = NULL;
    if (gNativeFloor || gMapFloorState.room != 7 || !sEnemyTasks[0] || gNativeEnemyKind[0] != 2) return;
    work = sEnemyTasks[0]->work;
    ReleaseObjTiles(work->tiles);
    work->tiles = NULL;
    /* Replace the proxy's unique palette instead of consuming another of
     * the GBA's sixteen OBJ palette banks. */
    ReleaseObjPalette(work->palette);
    work->palette = NULL;
    sArmorPalette = LoadObjPalette(gBoss01objPalette, 32);
    if (!sArmorPalette) {
        gNativeBossAllocation = 1;
        work->palette = LoadObjPalette(work->def->palette, 32);
        work->tiles = AllocObjTiles(work->def->tileCount * 32, NULL);
        return;
    }
    for (i = 0; i < 7; i++) {
        /* Reserve only idle and the original crouch/orbit poses used by the
         * tactical windup and impact, leaving unused battle frames unloaded. */
        size = 0;
        for (pose = 0; pose < 2; pose++) {
            idle = ((AnimHeader**)sArmorDefs[i].anims)[NativeArmorAnimId(i, pose)];
            for (j = 0; j < idle->frameCount; j++) {
                bytes = GetSpriteTileBytes(((void**)sArmorDefs[i].gfxTable)[idle->frames[j].gfxIndex]);
                if (bytes > size) size = bytes;
            }
        }
        sArmorTiles[i] = AllocObjTiles(size, sArmorDefs[i].tiles);
        if (!sArmorTiles[i]) {
            gNativeBossAllocation = i + 2;
            for (j = 0; j < i; j++) {ReleaseObjTiles(sArmorTiles[j]);sArmorTiles[j] = NULL;}
            ReleaseObjPalette(sArmorPalette);
            sArmorPalette = NULL;
            work->palette = LoadObjPalette(work->def->palette, 32);
            work->tiles = AllocObjTiles(work->def->tileCount * 32, NULL);
            return;
        }
        AnimInit(&sArmorAnim[i], sArmorDefs[i].anims, sArmorDefs[i].gfxTable);
        AnimStart(&sArmorAnim[i], 0, ANIM_FLAG_LOOP);
    }
    /* Task destruction and room resource release each own one reference. */
    work->palette = LoadObjPalette(gBoss01objPalette, 32);
    sArmorTaskDesc = *sEnemyTasks[0]->desc;
    sArmorTaskDesc.draw = NativeArmorDraw;
    sEnemyTasks[0]->desc = &sArmorTaskDesc;
    gNativeBossReady = 1;
}
u16 gNativeEnemyHp[6];
u8 gNativeEnemyKind[6];
u8 gNativeEnemyCharge[6];
u16 gNativeBreaks;
u16 gNativeTurn;
static ObjPalette* sCardPalettes[4];
static const CardDef* sCards[4];
u16 gNativeGuard;
static u16 NativeJafarDamage(void) {return gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : 10;}
static int NativeWindupRange(u8 slot) {return sEnemyTasks[slot]->desc == &sMarlTaskDesc ? 112 : 96;}
static u16 NativeMarlDamage(u8 slot) {
    return gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : gNativeEnemyHp[slot] <= 28 ? 14 : 10;
}
static int NativeMarlHits(u8 slot, const FldPos* pos) {
    FldPos* enemy = &((MapEnmWork*)sEnemyTasks[slot]->work)->obj.fieldPosition;
    s32 dx = enemy->x - pos->x;
    s32 dy = enemy->y + enemy->z - pos->y - pos->z;
    s32 dz = enemy->z - pos->z;
    return dx >= -(96 << 8) && dx <= (96 << 8) && dy >= -(16 << 8) && dy <= (16 << 8) &&
        dz >= -(24 << 8) && dz <= (24 << 8);
}
static u16 NativeCloudDamage(u8 slot) {
    return gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : gNativeEnemyHp[slot] <= 20 ? 16 : 12;
}
static int NativeCloudHits(u8 slot, const FldPos* pos) {
    FldPos* enemy = &((MapEnmWork*)sEnemyTasks[slot]->work)->obj.fieldPosition;
    s32 dx = enemy->x - pos->x;
    s32 dy = enemy->y + enemy->z - pos->y - pos->z;
    s32 dz = enemy->z - pos->z;
    return dx >= -(64 << 8) && dx <= (64 << 8) && dy >= -(16 << 8) && dy <= (16 << 8) &&
        dz >= -(24 << 8) && dz <= (24 << 8);
}
static int NativeBlastRange(u8 slot) {
    return sEnemyTasks[slot]->desc == &sArmorTaskDesc ? FieldArmorRange(gNativeEnemyHp[slot]) :
        FieldEnemyBlastRange(gNativeFloor, gMapFloorState.room);
}
static int NativeBlastDamage(u8 slot) {
    return sEnemyTasks[slot]->desc == &sArmorTaskDesc ? FieldArmorDamage(gNativeEnemyHp[slot]) :
        FieldEnemyDamage(2, gNativeFloor);
}
static void NativeCarryTurn(void) {
    u8 i;
    sPartyMove[gNativeParty] = gNativeMoveLeft;
    sPartyAction[gNativeParty] = gNativeActionLeft;
    sPartyAngle[gNativeParty] = gFieldState->actor.angle;
    for (i = 0; i < 3; i++) {
        sCarryMove[i] = sPartyMove[i];
        sCarryAction[i] = sPartyAction[i];
        sCarryAngle[i] = sPartyAngle[i];
    }
    sCarryParty = gNativeParty;
    sCarryGuard = gNativeGuard;
    sCarryTurn = 1;
}
static void NativeRosterSync(void) {
    u8 i;
    for (i = 0; i < 3; i++) {
        gNativeRoster.heroHp[NativeHero(i)] = gNativePartyHealth.hp[i];
        gNativeRoster.heroMove[NativeHero(i)] = i == gNativeParty ? gNativeMoveLeft : sPartyMove[i];
        gNativeRoster.heroAction[NativeHero(i)] = i == gNativeParty ? gNativeActionLeft : sPartyAction[i];
        gNativeRoster.heroAngle[NativeHero(i)] = i == gNativeParty ? gFieldState->actor.angle : sPartyAngle[i];
    }
}
static void NativeRosterHealth(void) {
    u8 i;
    for (i = 0; i < 3; i++) {
        gNativePartyHealth.maxHp[i] = FieldHeroMaxHp(NativeHero(i));
        gNativePartyHealth.hp[i] = gNativeRoster.heroHp[NativeHero(i)];
        sPartyMove[i] = gNativeRoster.heroMove[NativeHero(i)];
        sPartyAction[i] = gNativeRoster.heroAction[NativeHero(i)];
        sPartyAngle[i] = gNativeRoster.heroAngle[NativeHero(i)];
    }
    gGameState.hp = gNativePartyHealth.hp[gNativeParty];
    gNativeMoveLeft = sPartyMove[gNativeParty];gNativeActionLeft = sPartyAction[gNativeParty];
}
static void NativeFriendsInit(void) {
    u8 i, hero, pose;
    u16 size, bytes, frame;
    AnimHeader* anim;
    const AnimDef* defs;
    for (i = 0; i < 2; i++) {
        hero = NativeHero(i + 1);defs = sFriendAnims[hero - 1];size = 0;
        for (pose = 0; pose < 5; pose++) {
            anim = ((AnimHeader**)defs[pose].anims)[defs[pose].animId];
            for (frame = 0; frame < anim->frameCount; frame++) {
                bytes = GetSpriteTileBytes(((void**)defs[pose].gfxTable)[anim->frames[frame].gfxIndex]);
                if (bytes > size) size = bytes;
            }
        }
        sFriends[i].tiles = AllocObjTiles(size, defs[0].tiles);
        sFriends[i].palette = LoadObjPalette(hero == FIELD_DONALD ? gDonaldPalette :
            hero == FIELD_GOOFY ? gGoofyPalette : hero == FIELD_RALLY ? sRallyPalette : gCroudPalette, 32);
        AnimInit(&sFriends[i].anim, defs[0].anims, defs[0].gfxTable);
        AnimStart(&sFriends[i].anim, defs[0].animId, ANIM_FLAG_LOOP);
        sFriends[i].pose = sFriends[i].timer = sFriends[i].facing = 0;
        gNativeFriendPose[i] = 0;sFriends[i].pos = gFieldState->actor.fieldPosition;
    }
}
static void NativeFriendsReload(void) {
    u8 i;
    for (i = 0; i < 2; i++) {
        if (sFriends[i].tiles) ReleaseObjTiles(sFriends[i].tiles);
        if (sFriends[i].palette) ReleaseObjPalette(sFriends[i].palette);
    }
    NativeFriendsInit();
}
static void NativePartyInit(void) {
    u16 i, j;
    static const u16 kinds[4] = {CARD_KIND_KINGDOM_KEY, CARD_KIND_FIRE, CARD_KIND_CURE, CARD_KIND_GUARD_ARMOR};
    sSoraIdleTiles = AllocObjTiles(0x500, gSor1fl00Tiles);
    sPartyShadowTiles = LoadObjTiles(gBtlShadowTiles, 0x100);
    sPartyShadowPalette = LoadObjPalette(gCommonObjPalette, 32);
    NativeFriendsInit();
    for (i = 0; i < 3; i++) {
        sPartyPos[i] = gFieldState->actor.fieldPosition;
        sPartyMove[i] = 3;
        sPartyAction[i] = 1;
        sPartyAngle[i] = gNativeRoster.heroAngle[NativeHero(i)];
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
    sReachTiles = LoadObjTiles((void*)sReachGraphic, 32);
    gNativeReachCount = 0;
    sValueTiles = LoadObjTiles(gCardValueDigitTiles, 0x1e0);
    sValuePalette = LoadObjPalette(gCard00Palette, 32);
    gNativeParty = 0;
    gFieldState->actor.angle = sPartyAngle[0];
    gGameState.hp = gNativePartyHealth.hp[0];
    gNativeGuard = 0;
}
extern u8 task_fld_sora_1(FldWork* work, void* task);
extern u8 FldSoraClimb(FldWork* work, void* task);
static void NativeGuardPose(void) {
    u8 i;
    for (i = 1; i < 3; i++) if (NativeHero(i) == FIELD_GOOFY && gNativePartyHealth.hp[i]) NativePartyPose(i);
}
static void NativePartyActivate(u8 slot) {
    Task* playerTask = gFieldState->tasks2.head.activeHead->owner;
    FldWork* player = playerTask->work;
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    sPartyMove[gNativeParty] = gNativeMoveLeft;
    sPartyAction[gNativeParty] = gNativeActionLeft;
    sPartyAngle[gNativeParty] = gFieldState->actor.angle;
    gNativeParty = slot;
    if (player->state != FLD_STATE_GROUND) {
        player->state = FLD_STATE_GROUND;
        player->timer = player->vz = 0;
        playerTask->update = (TaskUpdateFunc)task_fld_sora_1;
        gFieldState->flags &= ~FIELD_FLAG_PLAYER_JUMPING;
    }
    gGameState.hp = gNativePartyHealth.hp[gNativeParty];
    gFieldState->actor.fieldPosition = sPartyPos[gNativeParty];
    gFieldState->actor.angle = sPartyAngle[gNativeParty];
    gFieldState->actor.speed = 0;
    gNativeMoveLeft = sPartyMove[gNativeParty];
    gNativeActionLeft = sPartyAction[gNativeParty];
}
static void NativePartySelect(void) {
    NativePartyActivate(FieldPartyNext(&gNativePartyHealth, gNativeParty));
}
static int NativeRecruitEligible(void) {
    return gNativeFloor == 0 && gMapFloorState.room == 9 &&
        gNativeRoster.phase == FIELD_REWARD && gNativeRoster.reward == FIELD_REWARD_BOSS &&
        !(gNativeRoster.unlocked & (1 << FIELD_CLOUD));
}
static void NativeRecruitFree(void) {
    if (sRecruitTiles) ReleaseObjTiles(sRecruitTiles);
    if (sRecruitPalette) ReleaseObjPalette(sRecruitPalette);
    sRecruitTiles = NULL;sRecruitPalette = NULL;sRecruitCard = NULL;
}
static void NativeRecruitInit(void) {
    u16 i;
    if (!NativeRecruitEligible() || sRecruitCard) return;
    if (sCardTiles[1]) ReleaseObjTiles(sCardTiles[1]);
    if (sCardPalettes[1]) ReleaseObjPalette(sCardPalettes[1]);
    sCardTiles[1] = NULL;sCardPalettes[1] = NULL;
    for (i = 0; i < 950; i++) if (gCardDefs[i].kind == CARD_KIND_CLOUD && gCardDefs[i].value == 5) {
        sRecruitCard = &gCardDefs[i];break;
    }
    if (sRecruitCard) {
        sRecruitTiles = LoadObjTiles(sRecruitCard->tiles2, 0x200);
        sRecruitPalette = LoadObjPalette(sRecruitCard->palette2, 32);
    }
}
static void NativeRecruitFinish(void) {
    if (!sRecruitCard && !sRecruitTiles && !sRecruitPalette) return;
    NativeRecruitFree();
    if (sCards[1]) {
        sCardTiles[1] = LoadObjTiles(sCards[1]->tiles2, 0x200);
        sCardPalettes[1] = LoadObjPalette(sCards[1]->palette2, 32);
    }
}
static void NativeAssemblyFree(void) {
    if (sAssemblyTiles) ReleaseObjTiles(sAssemblyTiles);
    if (sAssemblyPalette) ReleaseObjPalette(sAssemblyPalette);
    if (sAssemblyOtherTiles) ReleaseObjTiles(sAssemblyOtherTiles);
    if (sAssemblyOtherPalette) ReleaseObjPalette(sAssemblyOtherPalette);
    sAssemblyTiles = sAssemblyOtherTiles = NULL;
    sAssemblyPalette = sAssemblyOtherPalette = NULL;
}
static const CardDef* NativeHeroCard(u8 hero) {
    u16 i;
    u16 kind = hero == FIELD_DONALD ? CARD_KIND_DONALD_DUCK :
        hero == FIELD_GOOFY ? CARD_KIND_GOOFY : CARD_KIND_CLOUD;
    if (hero == FIELD_RALLY) return NULL;
    for (i = 0; i < 950; i++) if (gCardDefs[i].kind == kind && gCardDefs[i].value == 5) return &gCardDefs[i];
    return NULL;
}
static void NativeAssemblyCards(void) {
    NativeAssemblyFree();
    sAssemblyDonald = NativeHeroCard(NativeHero(1));
    sAssemblyOther = NativeHeroCard(NativeHero(2));
    if (sAssemblyDonald) {
        sAssemblyTiles = LoadObjTiles(sAssemblyDonald->tiles2, 0x200);
        sAssemblyPalette = LoadObjPalette(sAssemblyDonald->palette2, 32);
    }
    if (sAssemblyOther) {
        sAssemblyOtherTiles = LoadObjTiles(sAssemblyOther->tiles2, 0x200);
        sAssemblyOtherPalette = LoadObjPalette(sAssemblyOther->palette2, 32);
    }
}
static void NativeAssemblyInit(void) {
    u8 i;
    sAssemblyChoice = gNativeParty;
    sAssemblyDonald = sAssemblyOther = NULL;
    sAssemblyTiles = sAssemblyOtherTiles = NULL;
    sAssemblyPalette = sAssemblyOtherPalette = NULL;
    if (!gNativeAssembly) return;
    /* Setup cards borrow two combat-card banks until deployment. */
    for (i = 1; i < 4; i += 2) {
        if (sCardTiles[i]) ReleaseObjTiles(sCardTiles[i]);
        if (sCardPalettes[i]) ReleaseObjPalette(sCardPalettes[i]);
        sCardTiles[i] = NULL;sCardPalettes[i] = NULL;
    }
    NativeAssemblyCards();
}
static void NativeAssemblyCycle(int direction) {
    int hero, step;
    if (!sAssemblyChoice) return;
    NativeRosterSync();
    hero = NativeHero(sAssemblyChoice);
    for (step = 0; step < FIELD_HEROES - 1; step++) {
        hero = 1 + (hero - 1 + (direction > 0 ? 1 : FIELD_HEROES - 2)) % (FIELD_HEROES - 1);
        if (gNativeRoster.unlocked & (1 << hero)) break;
    }
    if (FieldRosterDeploy(&gNativeRoster, sAssemblyChoice, hero)) {
        NativeRosterHealth();
        gFieldState->actor.angle = sPartyAngle[gNativeParty];
        if (!gNativePartyHealth.hp[gNativeParty]) NativePartySelect();
        NativeFriendsReload();NativeAssemblyCards();
    }
}
static void NativeAssemblyCommit(void) {
    u8 i;
    if (!gNativePartyHealth.hp[sAssemblyChoice]) return;
    while (gNativeParty != sAssemblyChoice) NativePartySelect();
    NativeAssemblyFree();
    for (i = 1; i < 4; i += 2) if (sCards[i]) {
        sCardTiles[i] = LoadObjTiles(sCards[i]->tiles2, 0x200);
        sCardPalettes[i] = LoadObjPalette(sCards[i]->palette2, 32);
    }
    FieldRosterBegin(&gNativeRoster, gMapFloorState.room);
    gNativeAssembly = 0;
}
static void NativeAssemblyDraw(void) {
    u8 i;
    const CardDef* card;
    void* tiles;
    ObjPalette* palette;
    if (gNativeProgressReward && sProgressKind == 4 && sRecruitCard && sRecruitTiles && sRecruitPalette)
        DrawSprite(120, 72, sRecruitCard->gfx2, sRecruitTiles, sRecruitPalette, NULL, 0, 1);
    if (!gNativeAssembly) return;
    for (i = 0; i < 3; i++) {
        card = i == 1 ? sAssemblyDonald : i == 2 ? sAssemblyOther : sCards[0];
        tiles = i == 1 ? sAssemblyTiles : i == 2 ? sAssemblyOtherTiles : sCardTiles[0];
        palette = i == 1 ? sAssemblyPalette : i == 2 ? sAssemblyOtherPalette : sCardPalettes[0];
        if (i && NativeHero(i) == FIELD_RALLY)
            DrawSprite(68 + i * 52, i == sAssemblyChoice ? 92 : 105,
                (void*)sRallyFrames[0], sFriends[i - 1].tiles,
                sFriends[i - 1].palette, NULL, 0, 1);
        if (card && tiles && palette) DrawSprite(68 + i * 52, i == sAssemblyChoice ? 65 : 78,
            card->gfx2, tiles, palette, NULL, 0, 1);
    }
}
static void NativePartyDraw(void) {
    u8 i;
    s16 x, y;
    int card;
    u8 kind, pose;
    FldWork* player = ((Task*)gFieldState->tasks2.head.activeHead->owner)->work;
    for (i = 0; i < 2; i++) {
        if (!gNativePartyHealth.hp[i + 1]) continue;
        /* Animate only actual horizontal travel. Preview cursors and rejected
         * commands cannot make a stationary party member run in place. */
        pose = sFriends[i].pos.x != sPartyPos[i + 1].x ||
            sFriends[i].pos.y != sPartyPos[i + 1].y ? 2 : 0;
        if (sPartyPos[i + 1].x != sFriends[i].pos.x)
            sFriends[i].facing = sPartyPos[i + 1].x > sFriends[i].pos.x;
        sFriends[i].pos = sPartyPos[i + 1];
        if (sFriends[i].timer) sFriends[i].timer--;
        if (sFriends[i].timer || (NativeHero(i + 1) == FIELD_GOOFY && gNativeGuard && sFriends[i].pose == 1)) pose = 1;
        if (gNativeParty == i + 1) {
            if (player->state == FLD_STATE_JUMP_START || player->state == FLD_STATE_JUMP_RISE) pose = 3;
            else if (player->state == FLD_STATE_FALL) pose = 4;
        }
        if (NativeHero(i + 1) == FIELD_RALLY) {
            u8 angle = gNativeParty == i + 1 ? gFieldState->actor.angle : sPartyAngle[i + 1];
            u8 bank = (angle > 0 && angle < 128 ? 2 : 0) + (angle < 64 || angle > 192 ? 1 : 0);
            AnimDef def = sFriendAnims[FIELD_RALLY - 1][pose];
            def.animId = bank * 5 + pose;
            sFriends[i].facing = bank >= 2;
            AnimChangeWithDef(&def, &sFriends[i].anim, 0,
                pose >= 3 ? 0 : ANIM_FLAG_LOOP, sFriends[i].tiles);
            sFriends[i].pose = pose;
        } else if (sFriends[i].pose != pose) {
            sFriends[i].pose = pose;
            AnimChangeWithDef(sFriendAnims[NativeHero(i + 1) - 1], &sFriends[i].anim, pose,
                pose >= 3 ? 0 : ANIM_FLAG_LOOP, sFriends[i].tiles);
        }
        gNativeFriendPose[i] = pose;
        /* Skip the original scripted jump windup: native physics already
         * owns launch timing. Hold the matching original airborne frame. */
        if (NativeHero(i + 1) == FIELD_DONALD && pose >= 3) AnimSetFrame(&sFriends[i].anim, pose == 3 ? 3 : 2);
        AnimUpdate(&sFriends[i].anim);
        x = (sFriends[i].pos.x - gFieldState->x) >> 8;
        y = (sFriends[i].pos.y + sFriends[i].pos.z - gFieldState->y) >> 8;
        /* Offset only coincident starting sprites; subsequent positions use
         * the actual trail including ledge and jump heights. */
        if (gNativeParty != i + 1 &&
            sFriends[i].pos.x == gFieldState->actor.fieldPosition.x &&
            sFriends[i].pos.y == gFieldState->actor.fieldPosition.y)
            x += i ? 24 : -24;
        /* Keep the original shadow on the standing surface while the body
         * rises and falls. Display offsets move body and shadow together. */
        if (sPartyShadowTiles && sPartyShadowPalette)
            DrawSprite(x, (sFriends[i].pos.y + sFriends[i].pos.ground - gFieldState->y) >> 8,
                gBtlShadowFrames[0], sPartyShadowTiles, sPartyShadowPalette, NULL, 0x800,
                -0x1003 - (sFriends[i].pos.y >> 8) * 4);
        if (sFriends[i].tiles && sFriends[i].palette)
            DrawSprite(x, y, AnimGetGfx(&sFriends[i].anim), sFriends[i].tiles,
                sFriends[i].palette, NULL, 0x800 | (sFriends[i].facing ? SPRITE_FLAG_HFLIP : 0),
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
    if (gNativeReward && gNativeDeck.count < FIELD_DECK_MAX) {
        for (i = 0; i < 3; i++) {
            kind = (sRewardSeed % 4 + i) % 4;
            x = 76 + i * 44;
            y = i == gNativeRewardChoice ? 65 : 76;
            if (sCardTiles[kind] && sCardPalettes[kind])
                DrawSprite(x, y, sCards[kind]->gfx2, sCardTiles[kind], sCardPalettes[kind], NULL, 0, 2);
            if (sValueTiles && sValuePalette)
                DrawSprite(x - 3, y - 4, gCardValueDigitFrames[5 + ((sRewardSeed >> 8) + i) % 5],
                    sValueTiles, sValuePalette, NULL, 0, 1);
        }
    }
    for (i = 0; i < gNativeDeck.stocked; i++) {
        card = gNativeDeck.stock[i];kind = gNativeDeck.kind[card];
        if (sCardTiles[kind] && sCardPalettes[kind])
            DrawSprite(164 + i * 27, 49, sCards[kind]->gfx2,
                sCardTiles[kind], sCardPalettes[kind], NULL, 0, 2);
        if (sValueTiles && sValuePalette)
            DrawSprite(161 + i * 27, 45, gCardValueDigitFrames[gNativeDeck.value[card]],
                sValueTiles, sValuePalette, NULL, 0, 1);
    }

}
static void NativePartyFree(void) {
    u8 i;
    NativeAssemblyFree();
    NativeRecruitFree();
    if (sReachTiles) ReleaseObjTiles(sReachTiles);
    sReachTiles = NULL;gNativeReachCount = 0;
    if (sCloudTiles) ReleaseObjTiles(sCloudTiles);
    if (sCloudPalette) ReleaseObjPalette(sCloudPalette);
    gNativeCloudReady = 0;
    if (sMarlTiles) ReleaseObjTiles(sMarlTiles);
    if (sMarlPalette) ReleaseObjPalette(sMarlPalette);
    gNativeMarlReady = 0;
    for (i = 0; i < 2; i++) if (sJafarTiles[i]) ReleaseObjTiles(sJafarTiles[i]);
    if (sJafarPalette) ReleaseObjPalette(sJafarPalette);
    gNativeJafarReady = 0;
    for (i = 0; i < 7; i++) if (sArmorTiles[i]) ReleaseObjTiles(sArmorTiles[i]);
    if (sArmorPalette) ReleaseObjPalette(sArmorPalette);
    gNativeBossReady = 0;
    for (i = 0; i < 2; i++) {
        ReleaseObjTiles(sFriends[i].tiles);
        ReleaseObjPalette(sFriends[i].palette);
    }
    ReleaseObjTiles(sSoraIdleTiles);
    ReleaseObjTiles(sPartyShadowTiles);
    ReleaseObjPalette(sPartyShadowPalette);
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
        sRewardSeed = GetMapFloorRoom(gMapFloorState.room)->seed;
        gNativeRewardChoice = 0;
        gNativeReward = 1;
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
    /* Keep OBJ palette banks available for the controllable party and all
     * four card illustrations. Mandatory base-floor props are placed first. */
    if (finder != 13 && FIELD_PROP_PALETTES >= 2) return 0;
    if (finder < 14 && gMapGmkSpotFuncs[finder](pos)) return 1;
    /* Optional decorations must retain their original footprint constraints.
     * Only mandatory base-floor objects use finder 13 and ignore failure. */
    if (finder != 13) return 0;
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
u8 NativeDoorWaitHit(MapDoorWork* work) {
    /* The generated graph already owns every room. Native doors also accept
     * sword hits after opening, which would start an unhandled synthesis menu. */
    (void)work;
    return 1;
}
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
    if (!sResume) gNativeRoster.cleared = 0;
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
        if (!sResume) sSuspend.roomCached[i] = 0;
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
    for (i = 0; i < 3; i++) gNativePartyHealth.hp[i] = sSuspend.partyHp[i];
    gGameState.hp = sSuspend.hp;
    gNativeDeck = sSuspend.deck;
    gNativeRoster = sSuspend.roster;
    for (i = 0; i < 3; i++) {
        gNativeRoster.heroHp[NativeHero(i)] = sSuspend.partyHp[i];
        gNativeRoster.heroMove[NativeHero(i)] = sSuspend.move[i];
        gNativeRoster.heroAction[NativeHero(i)] = sSuspend.action[i];
    }
    NativeRosterHealth();
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
    if (sSaveSlot >= 0) {sResume = 1;NativeRestoreWorld();}
}
static void NativeCaptureEncounter(void) {
    u8 i, j, n = 0, room = gMapFloorState.room;
    MapEnmWork* enemy;
    FieldEncounter* record;
    for (i = 0; i < 6; i++) {
        record = &sSuspend.encounters[room][i];
        record->hp = record->kind = 0;
        for (j = 0; j < 4; j++) record->pos[j] = 0;
    }
    for (i = 0; i < 6; i++) if (sEnemyTasks[i]) {
        enemy = sEnemyTasks[i]->work;
        record = &sSuspend.encounters[room][n++];
        for (j = 0; j < 4; j++) record->pos[j] = ((s32*)&enemy->obj.fieldPosition)[j] >> 8;
        record->hp = gNativeEnemyHp[i];
        record->kind = gNativeEnemyKind[i] | (gNativeEnemyCharge[i] << 3);
    }
    sSuspend.roomCached[room] = 1;
}
static void NativeWriteSuspend(void) {
    u8 i, j;
    u16 k;
    s16 slot = sSaveSlot == 0 ? 1 : 0;
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
    {
        Task* playerTask = gFieldState->tasks2.head.activeHead->owner;
        FldWork* player = playerTask->work;
        sSuspend.climbing = player->state == FLD_STATE_CLIMB;
        sSuspend.climbTarget = sSuspend.climbing ? player->targetZ : 0;
        sSuspend.climbAngle = sSuspend.climbing ? gFieldState->actor.angle : 0;
    }
    sSuspend.turn = gNativeTurn;
    sSuspend.kills = gNativeKills;
    sSuspend.chests = gNativeChests;
    sSuspend.deck = gNativeDeck;
    NativeRosterSync();
    sSuspend.roster = gNativeRoster;
    for (i = 0; i < TAC_WORLD_ROOMS; i++) {
        sSuspend.roomFlags[i] = GetMapFloorRoom(i)->flags;
        sSuspend.roomEnemies[i] = GetMapFloorRoom(i)->enemiesLeft;
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) sSuspend.partyPos[i][j] = ((s32*)&sPartyPos[i])[j];
        sSuspend.move[i] = sPartyMove[i];
        sSuspend.action[i] = sPartyAction[i];
        sSuspend.partyHp[i] = gNativePartyHealth.hp[i];
    }
    NativeCaptureEncounter();
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

static const u16 sHudPalette[16] = {
    0, 0x1462, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff,
    0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff
};
extern void MapEnmSetAnim(MapEnmWork* work, u8 index, u16 flags);
u16 gNativeMenu, gNativeMenuChoice;
static u32* sHudTiles;
static u16* sHudScreen;
static volatile u8 sHudPending;
extern void (*gModeVBlankCallback)();
static void (*sPreviousVBlank)();
static void NativeLabel(u8 x, u8 y, const char* text) {
    int tile = (y / 8) * 32 + x / 8 + 1;
    int column = x / 8;
    int row = (gNativeMenu || gNativeProgressReward) ? y / 8 : y == 24 ? 19 : y == 32 ? 18 : y / 8;
    int glyph, i;
    u32 pixels;
    int pixel;
    if (!sHudTiles || !sHudScreen) return;
    /* A shorter replacement prompt must erase the prior label tail. The
     * reserved blank tile is shared; later labels can still fill this row. */
    for (i = column; i < 30; i++) sHudScreen[row * 32 + i] = 0xf000;
    while (*text && column < 30 && tile < 225) {
        glyph = *text >= '0' && *text <= '9' ? *text - '0' + 1 :
            *text >= 'A' && *text <= 'Z' ? *text - 'A' + 11 : 0;
        for (i = 0; i < 8; i++) {
            pixels = 0x11111111;
            if (i < 7) for (pixel = 0; pixel < 5; pixel++)
                if (sUiGlyphs[glyph][i] & (1 << (4 - pixel)))
                    pixels |= 2 << ((pixel + 1) * 4);
            sHudTiles[tile * 8 + i] = pixels;
        }
        sHudScreen[row * 32 + column] = tile | 0xf000;
        text++; column++; tile++;
    }
}
static void NativeHudUpload(void) {
    if (!sHudPending || !sHudTiles || !sHudScreen) return;
    /* Defer a late frame rather than write through the visible scanout. */
    if (REG_VCOUNT < 160 || REG_VCOUNT > 208) return;
    /* Upload after the original VBlank display transfer. No visible-frame
     * VRAM writes, and the font stays within its 16 KiB BG character bank. */
    CpuFastCopy(sHudTiles, GetBgCharBase(0), 225 * 32);
    CpuFastCopy(sHudScreen, GetBgScreenBase(0), 2048);
    sHudPending = 0;
}
static void NativeHudVBlank(void) {
    if (sPreviousVBlank) sPreviousVBlank();
    NativeHudUpload();
}
static s32 NativeAbs(s32 n);
/* Prefer a member attackable now; otherwise pursue the closest living member.
 * Preview and resolution share this decision, including height eligibility. */
static u8 NativeEnemyTarget(u8 slot, s32* best, s32* height) {
    MapEnmWork* work = sEnemyTasks[slot]->work;
    FldPos pos;
    s32 distance, dz;
    u8 j, target = 0, canAttack = 0, eligible;
    *best = 0x7fffffff;
    *height = 0;
    for (j = 0; j < 3; j++) {
        if (!gNativePartyHealth.hp[j]) continue;
        pos = j == gNativeParty ? gFieldState->actor.fieldPosition : sPartyPos[j];
        distance = NativeAbs(work->obj.fieldPosition.x - pos.x) +
            NativeAbs(work->obj.fieldPosition.y + work->obj.fieldPosition.z - pos.y - pos.z);
        dz = NativeAbs(work->obj.fieldPosition.z - pos.z);
        eligible = distance <= ((gNativeEnemyKind[slot] == 2 ? NativeWindupRange(slot) : FieldEnemyRange(gNativeEnemyKind[slot])) << 8) &&
            dz <= ((gNativeEnemyKind[slot] == 2 ? 32 : FieldEnemyHeight(gNativeEnemyKind[slot])) << 8);
        if ((eligible && !canAttack) || (eligible == canAttack && distance < *best)) {
            target = j;*best = distance;*height = dz;canAttack = eligible;
        }
    }
    return target;
}
/* The preview uses the same metric and thresholds as enemy resolution. */
u16 gNativeThreats[3];
static void NativePreviewThreats(void) {
    u8 i, j, target;
    s32 best, distance, dz;
    MapEnmWork* work;
    FldPos pos;
    for (j = 0; j < 3; j++) gNativeThreats[j] = 0;
    for (i = 0; i < 6; i++) if (sEnemyTasks[i]) {
        work = sEnemyTasks[i]->work;
        if (sEnemyTasks[i]->desc == &sCloudTaskDesc) {
            if (gNativeEnemyCharge[i]) for (j = 0; j < 3; j++) {
                if (!gNativePartyHealth.hp[j]) continue;
                pos = j == gNativeParty ? gFieldState->actor.fieldPosition : sPartyPos[j];
                if (NativeCloudHits(i, &pos)) gNativeThreats[j] += NativeCloudDamage(i);
            }
            continue;
        }
        if (sEnemyTasks[i]->desc == &sMarlTaskDesc) {
            if (gNativeEnemyCharge[i]) for (j = 0; j < 3; j++) {
                if (!gNativePartyHealth.hp[j]) continue;
                pos = j == gNativeParty ? gFieldState->actor.fieldPosition : sPartyPos[j];
                if (NativeMarlHits(i, &pos)) gNativeThreats[j] += NativeMarlDamage(i);
            }
            continue;
        }
        if (sEnemyTasks[i]->desc == &sJafarTaskDesc) {
            target = NativeEnemyTarget(i, &best, &dz);
            if (gNativeEnemyCharge[i] && best <= (96 << 8) && dz <= (32 << 8))
                gNativeThreats[target] += NativeJafarDamage();
            continue;
        }
        if (gNativeEnemyKind[i] == 2) {
            if (gNativeEnemyCharge[i]) for (j = 0; j < 3; j++) {
                if (!gNativePartyHealth.hp[j]) continue;
                pos = j == gNativeParty ? gFieldState->actor.fieldPosition : sPartyPos[j];
                distance = NativeAbs(work->obj.fieldPosition.x - pos.x) +
                    NativeAbs(work->obj.fieldPosition.y + work->obj.fieldPosition.z - pos.y - pos.z);
                dz = NativeAbs(work->obj.fieldPosition.z - pos.z);
                if (distance <= (NativeBlastRange(i) << 8) && dz <= (24 << 8))
                    gNativeThreats[j] += gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : NativeBlastDamage(i);
            }
            continue;
        }
        target = NativeEnemyTarget(i, &best, &dz);
        if (best <= (FieldEnemyRange(gNativeEnemyKind[i]) << 8) && dz <= (FieldEnemyHeight(gNativeEnemyKind[i]) << 8))
            gNativeThreats[target] += gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : FieldEnemyDamage(gNativeEnemyKind[i], gNativeFloor);
    }
}
static void NativeIntentDraw(void) {
    u8 i;
    u16 damage;
    s16 x, y;
    FldPos* pos;
    if (!sValueTiles || !sValuePalette || gNativeEnemyFrames) return;
    for (i = 0; i < 3; i++) if (gNativePartyHealth.hp[i] && gNativeThreats[i]) {
        damage = gNativeThreats[i] > 99 ? 99 : gNativeThreats[i];
        pos = &sPartyPos[i];
        x = (pos->x - gFieldState->x) >> 8;
        y = ((pos->y + pos->z - gFieldState->y) >> 8) - 28;
        if (damage >= 10) DrawSprite(x - 6, y, gCardValueDigitFrames[damage / 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
        DrawSprite(x + 2, y, gCardValueDigitFrames[damage % 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
    }
}
static void NativeHud(void) {
    char line[] = "MOVE 3 ACT 1 HP 000";
    char location[] = "FLOOR 1 ROOM 00";
    char health[] = "S00 D00 G00";
    char threat[] = "NEXT S00 D00 G00";
    char stock[] = "STOCK 0 A SLEIGHT";
    char sleight[] = "KEY 00 A SLEIGHT";
    int sleightKind, sleightValue;
    static const char* const effects[4] = {"KEY", "FIR", "CUR", "GRD"};
    u16 hp = gNativeAssembly ? gNativePartyHealth.hp[sAssemblyChoice] : gGameState.hp;
    u8 charging = 0;
    int card = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
    static const char* const names[4] = {"KEYBLADE A PLAY", "FIRE A PLAY", "CURE A PLAY", "GUARD A PLAY"};
    u16 i;
    /* Keep completed tiles immutable until VBlank consumes them. Clearing
     * pending at the next draw can otherwise starve uploads indefinitely. */
    if (!sHudTiles || !sHudScreen || sHudPending) return;
    for (i = 0; i < 1024; i++) sHudScreen[i] = 0xf000;
    for (i = 0; i < 225 * 8; i++) sHudTiles[i] = 0x11111111;
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

    line[5] = '0' + (gNativeAssembly && sAssemblyChoice!=gNativeParty ? sPartyMove[sAssemblyChoice] : gNativeMoveLeft);
    line[11] = '0' + (gNativeAssembly && sAssemblyChoice!=gNativeParty ? sPartyAction[sAssemblyChoice] : gNativeActionLeft);
    line[16] = line[17] = line[18] = '0';
    while (hp >= 100) { line[16]++; hp -= 100; }
    while (hp >= 10) { line[17]++; hp -= 10; }
    line[18] += hp;
    NativeLabel(0, 0, line);
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] && gNativeEnemyCharge[i]) charging = 1;
    gNativeCureTarget = NativeCureTarget();
    NativeLabel(0, 8, gNativePreview ? (gNativeRouteCost < 0 ? "OUT OF REACH B CANCEL" : gNativeRouteCost == 0 ? "AT ORIGIN B CANCEL" : gNativeRouteCost > gNativeMoveLeft ? "TOO FAR B CANCEL" : gNativePreview == 3 ? "A DESCEND MOVE B CANCEL" : gNativePreview == 2 ? (sClimbPreviewDirection == DPAD_UP ? "A CLIMB B CANCEL" : "A DESCEND B CANCEL") : "A MOVE B CANCEL") : gNativeResult == 1 ? "DEFEAT SELECT RETRY" : gNativeResult == 2 ? "RUN CLEAR SELECT RETRY" : gNativeEnemyFrames ? "ENEMY TURN" : gNativeClimbing ? "CLIMB D PAD B DROP" : charging ? (gNativeBossReady ? (gNativeBossPhase == 2 ? "BODY STRIKE 48" : gNativeBossPhase == 1 ? "ONE HAND SLAM 64" : "GUARD ARMOR SLAM 80") : gNativeJafarReady ? "JAFAR SPELL 96" : gNativeCloudReady ? "CLOUD CROSS SLASH 64" : gNativeMarlReady ? (gNativeEnemyHp[0] <= 28 ? "MARLUXIA RAGE SCYTHE" : "MARLUXIA SCYTHE") : "GUARDIAN CHARGING") : card < 0 ? "EMPTY L R RELOAD" : names[gNativeDeck.kind[card]]);
    if (!gNativePreview && !gNativeResult && !gNativeEnemyFrames &&
        !gNativeClimbing && !charging && card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_CURE)
        NativeLabel(0, 8, NativeHero(gNativeCureTarget) == FIELD_SORA ? "CURE SORA A PLAY" :
            NativeHero(gNativeCureTarget) == FIELD_DONALD ? "CURE DONALD A PLAY" : NativeHero(gNativeCureTarget) == FIELD_GOOFY ? "CURE GOOFY A PLAY" : NativeHero(gNativeCureTarget) == FIELD_RALLY ? "CURE RALLY A PLAY" : "CURE CLOUD A PLAY");
    if (!gNativePreview && !gNativeResult && !gNativeEnemyFrames &&
        !gNativeClimbing && !charging && card >= 0 && !gNativeDeck.stocked &&
        gNativeDeck.kind[card] == FIELD_CARD_FIRE && gNativeActionLeft)
        NativeLabel(0, 8, gNativeFireTarget < 0 ? "FIRE NO TARGET" : !gNativeFireDamage ? "FIRE CARD BREAK" : "FIRE R B TARGET A");
    if (!gNativePreview && !gNativeResult && !gNativeEnemyFrames && !gNativeClimbing &&
        !charging && card >= 0 && !gNativeDeck.stocked && gNativeActionLeft &&
        gNativeDeck.kind[card] == FIELD_CARD_KEY && gNativeParty)
        NativeLabel(0, 8, NativeHero(gNativeParty) == FIELD_DONALD ? "DONALD MAGIC R B A" : NativeHero(gNativeParty) == FIELD_GOOFY ? "GOOFY SPIN A PLAY" : NativeHero(gNativeParty) == FIELD_RALLY ? "RALLY STRIKE A PLAY" : "CLOUD SLASH R B A");
    location[6] += gNativeFloor < 3 ? gNativeFloor : 2;
    location[13] += gMapFloorState.room >= 10;
    location[14] += gMapFloorState.room >= 10 ? gMapFloorState.room - 10 : gMapFloorState.room;
    if (gNativePreview) {
        char route[] = "ROUTE 0 MOVE 0";
        route[6] = gNativeRouteCost >= 0 && gNativeRouteCost < 10 ? '0' + gNativeRouteCost : 'X';
        route[13] += gNativeMoveLeft;
        NativeLabel(0, 16, route);
    } else NativeLabel(0, 16, location);
    NativeLabel(128, 16, NativeHero(gNativeParty) == FIELD_SORA ? "SORA" : NativeHero(gNativeParty) == FIELD_DONALD ? "DONALD" : NativeHero(gNativeParty) == FIELD_GOOFY ? "GOOFY" : NativeHero(gNativeParty) == FIELD_RALLY ? "RALLY" : "CLOUD");
    for (i = 0; i < 3; i++) {
        health[i * 4] = threat[5 + i * 4] = NativeHero(i) == FIELD_SORA ? 'S' : NativeHero(i) == FIELD_DONALD ? 'D' : NativeHero(i) == FIELD_GOOFY ? 'G' : NativeHero(i) == FIELD_RALLY ? 'R' : 'C';
        health[1 + i * 4] = '0' + gNativePartyHealth.hp[i] / 10;
        health[2 + i * 4] = '0' + gNativePartyHealth.hp[i] % 10;
    }
    NativeLabel(0, 32, gNativeSaveNotice ?
        (gNativeSaveNotice == 1 ? "SAVED" : "SAVE FAILED") : health);
    for (i = 0; i < 3; i++) {
        threat[6 + i * 4] = '0' + gNativeThreats[i] / 10;
        threat[7 + i * 4] = '0' + gNativeThreats[i] % 10;
    }
    stock[6] += gNativeDeck.stocked;
    if (FieldDeckSleightPreview(&gNativeDeck, &sleightKind, &sleightValue)) {
        for (i = 0; i < 3; i++) sleight[i] = effects[sleightKind][i];
        sleight[4] += sleightValue / 10;
        sleight[5] += sleightValue % 10;
        NativeLabel(0, 24, sleight);
        if (!gNativePreview && !gNativeResult && !gNativeEnemyFrames &&
            !gNativeClimbing && !charging) {
            int recipe = FieldDeckRecipe(&gNativeDeck);
            NativeLabel(0, 8, recipe == 1 ? "TRIPLE KEY A PLAY" : recipe == 2 ? "FIRAGA A PLAY" :
                recipe == 3 ? "CURAGA A PLAY" : recipe == 4 ? "AEGIS A PLAY" : "A SLEIGHT L B CANCEL");
        }
    } else NativeLabel(0, 24, gNativeDeck.stocked ? stock : threat);
    if (gNativePreview==2 || gNativePreview==3)
        NativeLabel(0,24,gNativePreview==2 ? "R FLOOR CURSOR" : "R HEIGHT CURSOR");
    if (!gNativeReward && !gNativePreview && !gNativeResult && !gNativeEnemyFrames &&
        !gNativeClimbing && !charging && !gNativeActionLeft)
        NativeLabel(0, 8, "ACT SPENT START TURN");
    if (gNativeReward) {
        NativeLabel(0, 8, gNativeDeck.count < FIELD_DECK_MAX ? "CHEST CHOOSE L R A" : "DECK FULL A HEAL");
        NativeLabel(0, 24, "PARTY HEAL 12");
    }
    if (gNativeAssembly) {
        NativeLabel(0, 8, "ROUND SETUP L R A");
        NativeLabel(0, 24, "UP DOWN CHANGE PARTY");
        {
            char bonuses[] = "POWER 0 KEY 0 FIR 0 CUR 0";
            u8 hero=NativeHero(sAssemblyChoice);
            bonuses[6]+=(char)gNativeRoster.power[hero];
            bonuses[12]+=(gNativeRoster.sleights[hero]&1) ? 4 : 0;
            bonuses[18]+=(gNativeRoster.sleights[hero]&2) ? 4 : 0;
            bonuses[24]+=(gNativeRoster.sleights[hero]&4) ? 4 : 0;
            NativeLabel(0,16,bonuses);
            NativeLabel(0,32,hero==FIELD_SORA ? "SORA KEYBLADE CLOSE RANGE" :
                hero==FIELD_DONALD ? "DONALD MAGIC CURE BONUS" :
                hero==FIELD_GOOFY ? "GOOFY SPIN SHIELD GUARD" : hero==FIELD_RALLY ? "RALLY MELEE CURE PLUS 4" : "CLOUD SWORD TARGET RANGE 64");
        }
    }
    if (gNativeProgressReward) {
        NativeLabel(0, 8, "CLEAR REWARD L R A");
        NativeLabel(0, 24, sProgressKind == 0 ? "POWER PLUS 1" :
            sProgressKind == 1 ? "KEY SLEIGHT PLUS 4" :
            sProgressKind == 2 ? "FIRE SLEIGHT PLUS 4" : "CURE SLEIGHT PLUS 4");
        NativeLabel(0, 32, NativeHero(sProgressHero) == FIELD_SORA ? "SORA UP DOWN CHOOSE" :
            NativeHero(sProgressHero) == FIELD_DONALD ? "DONALD UP DOWN CHOOSE" : NativeHero(sProgressHero) == FIELD_GOOFY ? "GOOFY UP DOWN CHOOSE" : NativeHero(sProgressHero) == FIELD_RALLY ? "RALLY UP DOWN CHOOSE" : "CLOUD UP DOWN CHOOSE");
        if (sProgressKind == 0 && gNativeRoster.power[NativeHero(sProgressHero)] >= 8)
            NativeLabel(0, 8, "POWER MAX CHANGE HERO");
        else if (sProgressKind > 0 && sProgressKind < 4 &&
            (gNativeRoster.sleights[NativeHero(sProgressHero)] & (1 << (sProgressKind - 1))))
            NativeLabel(0, 8, "OWNED CHOOSE ANOTHER");
    }
    if (gNativeAssembly && !gNativePartyHealth.hp[sAssemblyChoice]) NativeLabel(0, 8, "KO CHOOSE OTHER HERO");
    if (gNativeProgressReward && sProgressKind == 4 && NativeRecruitEligible()) {
        NativeLabel(0, 8, "CLOUD RECRUIT A");
        NativeLabel(0, 24, "UNLOCK CLOUD CARD");
        NativeLabel(0, 32, "POWER OR RECRUIT");
    }
    if (gNativeProgressReward && !gNativeMenu) {
        static const char* const rewards[5] = {"POWER PLUS 1", "KEY SLEIGHT PLUS 4", "FIRE SLEIGHT PLUS 4", "CURE SLEIGHT PLUS 4", "RECRUIT CLOUD"};
        u8 hero = NativeHero(sProgressHero);
        u8 options = NativeRecruitEligible() ? 5 : 4;
        gWin0V = 64;
        for (i = 0; i < 1024; i++) sHudScreen[i] = 0xf000;
        NativeLabel(0, 0, hero == FIELD_SORA ? "REWARD FOR SORA" : hero == FIELD_DONALD ? "REWARD FOR DONALD" :
            hero == FIELD_GOOFY ? "REWARD FOR GOOFY" : hero == FIELD_RALLY ? "REWARD FOR RALLY" : "REWARD FOR CLOUD");
        for (i = 0; i < options; i++) {
            NativeLabel(0, (i + 1) * 8, i == sProgressKind ? "X" : " ");
            NativeLabel(16, (i + 1) * 8, rewards[i]);
            if (!i && gNativeRoster.power[hero] >= 8) NativeLabel(184, 8, "MAX");
            else if (i > 0 && i < 4 && (gNativeRoster.sleights[hero] & (1 << (i - 1))))
                NativeLabel(184, (i + 1) * 8, "OWNED");
        }
        NativeLabel(0, 48, "UP DOWN  L R HERO  A CHOOSE");
    }
    if (!gNativeMenu && !gNativeAssembly && !gNativePreview && !gNativeProgressReward &&
        !gNativeReward && !gNativeEnemyFrames && !gNativeResult && !gNativeBusy && !gNativeSaveNotice)
        NativeLabel(0, 32, "SELECT COMMANDS");
    if (gNativeMenu) {
        static const char* const commands[6] = {"MOVE", "ATTACK", "SKILLS", "PARTY", "END TURN", "SUSPEND"};
        gWin0V = 64;
        for (i = 0; i < 1024; i++) sHudScreen[i] = 0xf000;
        NativeLabel(0, 0, gNativeMenu >= 3 ? "TARGET  UP DOWN  A CONFIRM" : gNativeMenu == 2 ? "SKILLS  L R CHOOSE  A NEXT" : "COMMANDS  UP DOWN A  B BACK");
        if (gNativeMenu == 9) {
            NativeLabel(0, 0, "LEDGE  UP DOWN A SELECT");
            NativeLabel(0, 16, gNativeMenuChoice ? " " : "X");
            NativeLabel(16, 16, "CLIMB UP");
            NativeLabel(0, 24, gNativeMenuChoice ? "X" : " ");
            NativeLabel(16, 24, "DROP DOWN");
            NativeLabel(0, 40, "JUMP COST ALREADY PAID");
            NativeLabel(0, 48, "A SELECT  B BACK");
        } else if (gNativeMenu == 8) {
            NativeLabel(0, 0, "PARTY  UP DOWN A SELECT");
            for (i = 0; i < 3; i++) {
                char status[] = "HP 00 MP0 ACT0";
                u8 hero = NativeHero(i);
                status[3] += gNativePartyHealth.hp[i] / 10;
                status[4] += gNativePartyHealth.hp[i] % 10;
                status[8] += i == gNativeParty ? gNativeMoveLeft : sPartyMove[i];
                status[13] += i == gNativeParty ? gNativeActionLeft : sPartyAction[i];
                NativeLabel(0, 16 + i * 8, i == gNativeMenuChoice ? "X" : " ");
                NativeLabel(8, 16 + i * 8, hero == FIELD_SORA ? "SORA" : hero == FIELD_DONALD ? "DONALD" :
                    hero == FIELD_GOOFY ? "GOOFY" : hero == FIELD_RALLY ? "RALLY" : "CLOUD");
                NativeLabel(72, 16 + i * 8, gNativePartyHealth.hp[i] ? status : "KO");
            }
            NativeLabel(0, 48, gNativePartyHealth.hp[gNativeMenuChoice] ? "A SWITCH  B BACK" : "KO CANNOT TAKE A TURN");
        } else if (gNativeMenu == 7) {
            NativeLabel(0, 0, "END PARTY TURN  A CONFIRM");
            for (i = 0; i < 3; i++) {
                char budget[] = "MP 0 ACT 0";
                u8 hero = NativeHero(i);
                budget[3] += i == gNativeParty ? gNativeMoveLeft : sPartyMove[i];
                budget[9] += i == gNativeParty ? gNativeActionLeft : sPartyAction[i];
                NativeLabel(0, 16 + i * 8, hero == FIELD_SORA ? "SORA" : hero == FIELD_DONALD ? "DONALD" :
                    hero == FIELD_GOOFY ? "GOOFY" : hero == FIELD_RALLY ? "RALLY" : "CLOUD");
                NativeLabel(64, 16 + i * 8, gNativePartyHealth.hp[i] ? budget : "KO");
            }
            NativeLabel(0, 40, "ENEMIES ACT NEXT");
            NativeLabel(0, 48, "A END TURN  B BACK");
        } else if (gNativeMenu == 6) {
            NativeLabel(0, 0, "JUMP  D PAD DIRECTION");
            NativeLabel(0, 16, gNativeDirection ? "MOVING JUMP" : "STANDING JUMP");
            NativeLabel(0, 24, gNativeDirection ? "COST ONE MOVE AND ACTION" : "COST ONE ACTION");
            NativeLabel(0, 32, "NO LANDING PREVIEW");
            NativeLabel(0, 40, (gNativeDirection & DPAD_UP) ?
                ((gNativeDirection & DPAD_LEFT) ? "NORTH WEST" : (gNativeDirection & DPAD_RIGHT) ? "NORTH EAST" : "NORTH") :
                (gNativeDirection & DPAD_DOWN) ? ((gNativeDirection & DPAD_LEFT) ? "SOUTH WEST" :
                (gNativeDirection & DPAD_RIGHT) ? "SOUTH EAST" : "SOUTH") :
                (gNativeDirection & DPAD_LEFT) ? "WEST" : gNativeDirection ? "EAST" : "STRAIGHT UP");
            NativeLabel(0, 48, !gNativeActionLeft ? "ACTION ALREADY SPENT" :
                gNativeDirection && !gNativeMoveLeft ? "NO MOVE LEFT  B BACK" : "A JUMP  B BACK TO MOVE");
        } else if (gNativeMenu == 5) {
            char returned[] = "RECOVER 00 DISCARDED";
            u8 discarded = 0;
            u8 n;
            for (n = 0; n < gNativeDeck.count; n++) if (gNativeDeck.pile[n] == 2) discarded++;
            returned[8] += discarded / 10;returned[9] += discarded % 10;
            NativeLabel(0, 0, "RELOAD CARDS  A CONFIRM");
            NativeLabel(0, 16, "RETURN DISCARDED CARDS");
            NativeLabel(0, 24, discarded ? "COST ONE ACTION" : "NOTHING TO RELOAD");
            NativeLabel(0, 32, returned);
            NativeLabel(0, 40, gNativeActionLeft ? "A RELOAD  B BACK" : "ACTION ALREADY SPENT");
        } else if (gNativeMenu >= 3) {
            char amount[] = "RECOVER 00";
            NativeLabel(0, 16, card < 0 ? "NO CARD" : names[gNativeDeck.kind[card]]);
            if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY && gNativeDeck.stocked != 3) {
                u8 hero = NativeHero(gNativeParty);
                NativeLabel(0, 16, hero == FIELD_DONALD ? "DONALD MAGIC" : hero == FIELD_GOOFY ? "GOOFY SHIELD SPIN" :
                    hero == FIELD_CLOUD ? "CLOUD SWORD" : hero == FIELD_RALLY ? "RALLY STRIKE" : "SORA KEYBLADE");
            }
            if (gNativeDeck.stocked == 3 && FieldDeckSleightPreview(&gNativeDeck, &sleightKind, &sleightValue)) {
                NativeLabel(0, 16, sleightKind == FIELD_CARD_CURE ? "PARTY CURE SLEIGHT" : sleightKind == FIELD_CARD_GUARD ? "PARTY GUARD SLEIGHT" : "AREA ATTACK SLEIGHT");
                NativeLabel(0, 24, "PREVIEW VALUES ON MAP");
                NativeLabel(0, 32, "A USE THREE STOCKED CARDS");
            } else if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_CURE) {
                u8 hero = NativeHero(gNativeCureTarget);
                NativeLabel(0, 24, hero == FIELD_SORA ? "TARGET SORA" : hero == FIELD_DONALD ? "TARGET DONALD" :
                    hero == FIELD_GOOFY ? "TARGET GOOFY" : hero == FIELD_RALLY ? "TARGET RALLY" : "TARGET CLOUD");
                amount[8] += gNativeCureHeal / 10;amount[9] += gNativeCureHeal % 10;
                NativeLabel(0, 32, amount);
            } else if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY && NativeHero(gNativeParty) == FIELD_GOOFY) {
                char targets[] = "TARGETS 0";
                u8 target;
                for (target = 0; target < 6; target++) if (gNativeSkillDamage[target]) targets[8]++;
                NativeLabel(0, 0, "SHIELD SPIN  A CONFIRM");
                NativeLabel(0, 24, "AREA DAMAGE ON MAP");
                NativeLabel(0, 32, targets);
            } else if (card >= 0 && (gNativeDeck.kind[card] == FIELD_CARD_FIRE ||
                (gNativeDeck.kind[card] == FIELD_CARD_KEY && (NativeHero(gNativeParty) == FIELD_DONALD || NativeHero(gNativeParty) == FIELD_CLOUD)))) {
                char damage[] = "DAMAGE 00";
                NativeLabel(0, 24, gNativeFireTarget < 0 ? "NO ELIGIBLE TARGET" : "SELECTED ENEMY ON MAP");
                damage[7] += gNativeFireDamage / 10;damage[8] += gNativeFireDamage % 10;
                NativeLabel(0, 32, damage);
            } else if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY) {
                NativeLabel(0, 0, "ATTACK  D PAD FACE  A USE");
                NativeLabel(0, 24, "FACE ENEMY BEFORE CONFIRM");
            } else if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_GUARD) {
                NativeLabel(0, 0, "GUARD  A CONFIRM");
                NativeLabel(0, 16, "PARTY PROTECTION");
                NativeLabel(0, 24, NativeHero(gNativeParty) == FIELD_GOOFY ? "PARTY BLOCKS ALL DAMAGE" : "PARTY TAKES ONE PER HIT");
                NativeLabel(0, 32, "UNTIL NEXT ENEMY TURN ENDS");
                NativeLabel(0, 40, "COST ONE CARD AND ACTION");
            } else NativeLabel(0, 24, "CONFIRM SELECTED ACTION");
            NativeLabel(0, 48, gNativeActionLeft ? "B BACK WITHOUT SPENDING" : "ACTION ALREADY SPENT");
        } else if (gNativeMenu == 2) {
            static const char* const skills[4] = {"KEYBLADE", "FIRE", "CURE", "GUARD"};
            char detail[] = "VALUE 0   STOCK 0 OF 3";
            char piles[] = "HAND 00 DRAW 00 DISCARD 00";
            u8 counts[3] = {0,0,0};
            u8 n;
            for (n = 0; n < gNativeDeck.count; n++)
                if (gNativeDeck.pile[n] < 3) counts[gNativeDeck.pile[n]]++;
            piles[5] += counts[1] / 10;piles[6] += counts[1] % 10;
            piles[13] += counts[0] / 10;piles[14] += counts[0] % 10;
            piles[24] += counts[2] / 10;piles[25] += counts[2] % 10;
            NativeLabel(0, 8, piles);
            detail[6] = card < 0 ? ' ' : '0' + gNativeDeck.value[card];
            detail[16] += gNativeDeck.stocked;
            NativeLabel(0, 16, card < 0 ? "NO CARDS  DOWN RELOAD" : skills[gNativeDeck.kind[card]]);
            if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY) {
                u8 hero = NativeHero(gNativeParty);
                NativeLabel(0, 16, hero == FIELD_DONALD ? "DONALD MAGIC" : hero == FIELD_GOOFY ? "GOOFY SHIELD SPIN" :
                    hero == FIELD_CLOUD ? "CLOUD SWORD" : hero == FIELD_RALLY ? "RALLY STRIKE" : "SORA KEYBLADE");
            }
            NativeLabel(0, 24, gNativeDeck.stocked == 3 ? "THREE STOCKED  A SLEIGHT" : gNativeDeck.stocked ? "UP STOCK  DOWN CLEAR" : "UP STOCK  DOWN RELOAD");
            NativeLabel(0, 32, "B BACK  L R CARD");
            NativeLabel(0, 40, detail);
            NativeLabel(0, 48, gNativeActionLeft ? "PLAY COSTS ONE ACTION" : "ACTION ALREADY SPENT");
        } else {
            int hasKey = 0;
            u8 handIndex;
            for (handIndex = 0; handIndex < 5; handIndex++) {
                int handCard = FieldDeckHand(&gNativeDeck, handIndex);
                if (handCard >= 0 && gNativeDeck.kind[handCard] == FIELD_CARD_KEY) hasKey = 1;
            }
            for (i = 0; i < 6; i++) {
                NativeLabel(0, (i + 1) * 8, i == gNativeMenuChoice ? "X" : " ");
                NativeLabel(16, (i + 1) * 8, commands[i]);
            }
            NativeLabel(104, 8, gNativeMoveLeft ? "CHOOSE TILE" : "NO MOVE LEFT");
            NativeLabel(104, 16, !gNativeActionLeft ? "ACTION SPENT" :
                gNativeDeck.stocked ? "STOCKED HAND" : hasKey ? "CHOOSE FACING" : "NO KEY CARD");
            NativeLabel(104, 24, gNativeActionLeft ? "CARDS AND COMBOS" : "ACTION SPENT");
            NativeLabel(104, 32, "SWITCH HERO");
            NativeLabel(104, 40, "ENEMIES ACT");
            NativeLabel(104, 48, "SAVE AND RESUME");
        }
    }
    if (gNativePreview == 1 && !gNativeMenu) NativeLabel(0, 32, "R JUMP  B BACK");
    if (!gNativeMenu && gNativeBusy == 2) {
        FldWork* player = ((Task*)gFieldState->tasks2.head.activeHead->owner)->work;
        if (player->state == FLD_STATE_LEDGE_CATCH || player->state == FLD_STATE_LEDGE_HANG) {
            NativeLabel(0, 8, "LEDGE  UP CLIMB  B DROP");
            NativeLabel(0, 32, player->state == FLD_STATE_LEDGE_HANG ? "SELECT LEDGE COMMANDS" : "CATCHING LEDGE");
        }
    }
    if (!gNativeMenu && !gNativeAssembly && !gNativeProgressReward && !gNativeReward &&
        !gNativePreview && !gNativeResult && !gNativeClimbing && gNativeBusy != 2) {
        for (i = 0; i < 1024; i++) sHudScreen[i] = 0xf000;
        gWin0V = 18;
        gWin1V = (144 << 8) | 160;
        NativeLabel(0, 0, line);
        NativeLabel(168, 0, NativeHero(gNativeParty) == FIELD_SORA ? "SORA" : NativeHero(gNativeParty) == FIELD_DONALD ? "DONALD" :
            NativeHero(gNativeParty) == FIELD_GOOFY ? "GOOFY" : NativeHero(gNativeParty) == FIELD_RALLY ? "RALLY" : "CLOUD");
        NativeLabel(0, 8, gNativeSaveNotice ? (gNativeSaveNotice == 1 ? "SAVED" : "SAVE FAILED") :
            gNativeEnemyFrames ? "ENEMIES THEN PARTY" : gNativeBusy ? "MOVING  PARTY THEN ENEMIES" : "PARTY THEN ENEMIES  SELECT");
    }
    sHudPending = 1;
}
static void NativeExit(void) {
    sHudPending = 0;
    if (gModeVBlankCallback == NativeHudVBlank) gModeVBlankCallback = sPreviousVBlank;
    if (sHudTiles) EwramFree(sHudTiles);
    if (sHudScreen) EwramFree(sHudScreen);
    sHudTiles = NULL;
    sHudScreen = NULL;
    NativePartyFree();
    DebugTextFree();
    Mode_MapFld_2();
}

static void NativeInit(s32 arg) {
    const struct TacWorldRoom* room = &sWorld.rooms[gMapFloorState.room];
    MapFloorRoom* saved = GetMapFloorRoom(gMapFloorState.room);
    MapEnmArgs enemy;
    ListNode* node;
    ListNode* next;
    Task* task;
    u8 i, kind;
    gMapForm.layout = room->layout;
    gMapForm.minWidth = gMapForm.maxWidth = room->width;
    gMapForm.minHeight = room->heightMin;
    gMapForm.maxHeight = room->heightMax;
    gMapForm.minDepth = room->depthMin;
    gMapForm.maxDepth = room->depthMax;
    gMapChkUseParams = 1;
    Mode_MapFld_0();
    /* Vanilla reserves all unused scenery tiles with an invisible dummy.
     * Native rooms never add frame-driven scenery; return this reservation
     * to the shared OBJ budget for party, cards and multipart bosses. */
    node = gFieldState->tasks.head.activeHead;
    while (node) {
        next = node->next;
        task = node->owner;
        if (task->desc == &gTaskDescMapGmkDmy) TaskKill(&gFieldState->tasks, task);
        node = next;
    }
    gMapChkUseParams = 0;
    gNativeRoomVisits++;
    /* Spawn once from the room seed, never from idle frame timing. */
    SeedRandom(room->seed ^ 0x454e4d);
    for (i = 0; i < 6; i++) {sEnemyTasks[i] = NULL; gNativeEnemyHp[i] = 0;gNativeEnemyCharge[i] = 0;}
    for (i = 0; i < saved->enemiesLeft && i < 6; i++) {
        kind = sSuspend.roomCached[gMapFloorState.room] ?
            sSuspend.encounters[gMapFloorState.room][i].kind & 7 :
            FieldEnemyKind(room->seed, gNativeFloor, gMapFloorState.room, i);
        gNativeEnemyKind[i] = kind;
        gNativeEnemyCharge[i] = sSuspend.roomCached[gMapFloorState.room] ?
            sSuspend.encounters[gMapFloorState.room][i].kind >> 3 : 0;
        MapEnmSetupArgs(&enemy, gMapEnmDefs[kind]);
        /* Airborne vanilla spawns enter from z=-160 and their real-time AI
         * lowers them later. Tactical actors do not run that entrance AI;
         * begin on the assigned standing surface so every role is reachable. */
        enemy.pos.z = enemy.pos.ground;
        sEnemyTasks[i] = TaskCreate(&gFieldState->tasks4, gMapEnmDefs[kind]->desc, &enemy);
        gNativeEnemyHp[i] = FieldEnemyHp(kind, gNativeFloor);
    }
    gNativeMoveLeft = 3;
    gNativeActionLeft = 1;
    gNativeBusy = 0;
    gNativeReward = gNativeRewardChoice = 0;
    gNativeFireChoice = gNativeCureChoice = -1;
    gNativeEnemyFrames = 0;
    gNativeDirection = 0;
    sFrames = 0;
    sAttack = 0;
    gNativePreview = 0;
    gNativeMenu = gNativeMenuChoice = 0;
    gNativeClimbReachMask = 0;
    sPathLength = 0;
    sAssemblyResume = sResume;
    NativePartyInit();
    NativeArmorInit();
    NativeJafarInit();
    NativeMarlInit();
    NativeCloudInit();
    if (sCarryTurn) {
        for (i = 0; i < 3; i++) {
            sPartyMove[i] = sCarryMove[i];
            sPartyAction[i] = sCarryAction[i];
            sPartyAngle[i] = sCarryAngle[i];
        }
        gNativeParty = sCarryParty;
        gFieldState->actor.angle = sPartyAngle[gNativeParty];
        gNativeGuard = sCarryGuard;
        if (gNativeGuard == 2) NativeGuardPose();
        gGameState.hp = gNativePartyHealth.hp[gNativeParty];
        gFieldState->actor.fieldPosition = sPartyPos[gNativeParty];
        gNativeMoveLeft = sPartyMove[gNativeParty];
        gNativeActionLeft = sPartyAction[gNativeParty];
        sCarryTurn = 0;
    }
    if (sResume) {
        u8 j;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 4; j++) ((s32*)&sPartyPos[i])[j] = sSuspend.partyPos[i][j];
            sPartyMove[i] = sSuspend.move[i];
            sPartyAction[i] = sSuspend.action[i];
            sPartyAngle[i] = gNativeRoster.heroAngle[NativeHero(i)];
        }
        gNativeParty = sSuspend.party;
        gFieldState->actor.angle = sPartyAngle[gNativeParty];
        gGameState.hp = gNativePartyHealth.hp[gNativeParty];
        gNativeGuard = sSuspend.guard;
        if (gNativeGuard == 2) NativeGuardPose();
        gFieldState->actor.fieldPosition = sPartyPos[gNativeParty];
        /* Scenery collision is culled against the camera. Restore the view
         * before rebuilding contacts, or a saved prop top appears unsupported
         * for the first player updates while the entrance camera catches up. */
        gFieldState->x2 = gFieldState->actor.fieldPosition.x;
        gFieldState->y2 = gFieldState->actor.fieldPosition.y + gFieldState->actor.fieldPosition.z;
        MapSnapCamera();
        TaskPoolUpdate(&gFieldState->tasks);
        {
            FldWork* player = ((Task*)gFieldState->tasks2.head.activeHead->owner)->work;
            ListNode* support = ((ListPool*)ColliderGetPool(6))->activeHead;
            /* Prop updates are paused during the room fade. A regenerated
             * platform may still carry its entrance-view culling flag. */
            while (support) {
                Collider* collider = support->owner;
                s32 dx = (gFieldState->actor.fieldPosition.x - collider->x) >> 4;
                s32 dy = (gFieldState->actor.fieldPosition.y * 2 - collider->y) >> 4;
                s32 radius = collider->radius >> 4;
                if ((collider->flags & COLLIDER_FLAG_IS_PLATFORM) &&
                    collider->z - collider->height == gFieldState->actor.fieldPosition.z &&
                    dx > -radius && dx < radius && dy > -radius && dy < radius &&
                    dx * dx + dy * dy < radius * radius)
                    ColliderSetDisabled(collider, 0);
                support = support->next;
            }
            ColliderSetPosition(&player->collider, gFieldState->actor.fieldPosition.x,
                gFieldState->actor.fieldPosition.y, gFieldState->actor.fieldPosition.z);
            ColliderUpdateAll();
        }
        gNativeMoveLeft = sPartyMove[gNativeParty];
        gNativeActionLeft = sPartyAction[gNativeParty];
        if (sSuspend.climbing) {
            Task* playerTask = gFieldState->tasks2.head.activeHead->owner;
            FldWork* player = playerTask->work;
            player->state = FLD_STATE_CLIMB;
            player->timer = 1;
            player->targetZ = sSuspend.climbTarget;
            playerTask->update = (TaskUpdateFunc)FldSoraClimb;
            gFieldState->actor.angle = sSuspend.climbAngle;
        }
        sResume = 0;
    }
    if (sSuspend.roomCached[gMapFloorState.room]) {
        u8 j;
        for (i = 0; i < saved->enemiesLeft; i++) {
            MapEnmWork* work = sEnemyTasks[i]->work;
            FieldEncounter* record = &sSuspend.encounters[gMapFloorState.room][i];
            for (j = 0; j < 4; j++) ((s32*)&work->obj.fieldPosition)[j] = record->pos[j] * 256;
            /* Repair the old fixed ceiling entrance in format-8 encounters;
             * ordinary saved positions and partial damage remain exact. */
            if ((gNativeEnemyKind[i] == 1 || gNativeEnemyKind[i] == 3) &&
                work->obj.fieldPosition.z == -0xA000)
                work->obj.fieldPosition.z = work->obj.fieldPosition.ground;
            gNativeEnemyHp[i] = record->hp;
            ColliderSetPosition(&work->collider, work->obj.fieldPosition.x,
                work->obj.fieldPosition.y, work->obj.fieldPosition.z);
        }
    }
    gNativeAssembly = sAssemblyResume ? gNativeRoster.phase == FIELD_ASSEMBLY : 1;
    if (gNativeAssembly) {gNativeRoster.phase = FIELD_ASSEMBLY;gNativeRoster.reward = 0;}
    gNativeProgressReward = gNativeRoster.phase == FIELD_REWARD;
    sProgressHero = sProgressKind = 0;
    NativeAssemblyInit();
    NativeRecruitInit();
    sHudPending = 0;
    sHudTiles = EwramAlloc(225 * 32);
    sHudScreen = EwramAlloc(2048);
    DebugTextInit(0, 0x2000, 0x800);
    DebugTextLoadPalette(0, sHudPalette, 32, 15);
    sPreviousVBlank = gModeVBlankCallback;
    gModeVBlankCallback = NativeHudVBlank;
}

static void NativeDamageEnemy(Task* task, u16 damage) {
    u8 i;
    MapFloorRoom* room;
    damage += gNativeRoster.power[NativeHero(gNativeParty)];
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] == task) {
        if (sPlayedValue && sPlayedValue < 3 + gNativeFloor) {
            gNativeBreaks++;
            return;
        }
        if (damage < gNativeEnemyHp[i]) {
            u8 phase = FieldArmorPhase(gNativeEnemyHp[i]);
            MapEnmWork* work = task->work;
            gNativeEnemyHp[i] -= damage;
            if (task->desc == &sArmorTaskDesc && FieldArmorPhase(gNativeEnemyHp[i]) > phase)
                if (TaskCreate(&work->tasks, &gTaskDescMapSpark, &work->obj)) gNativeBossBreaks++;
            return;
        }
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
/* Keep the route workspace in reserved RAM, outside the small native stack. */
static FieldRoute sEnemyRoute;
static FieldTacticsRoute sPlayerRoute;
static u8 sPlayerLinks[FIELD_ROUTE_CELLS];
static u8 sPlayerLinksKnown[11];
static u32 sPlayerGeometry;
static u16 sPlayerMoveBudget,sPlayerActionBudget;
static FldPos sRoutePos[FIELD_ROUTE_CELLS];
static u8 sRouteValid[FIELD_ROUTE_CELLS];
static Task* sRouteActor;
static ListPool* sRouteObstacles;
static int NativeRoutePropsClear(const FldPos* pos) {
    ListNode* node = sRouteObstacles->activeHead;
    Collider* collider;
    s32 dx, dy, dz, radius;
    s32 actorRadius = sRouteActor ? ((MapEnmWork*)sRouteActor->work)->collider.radius : 1024;
    s32 actorHeight = sRouteActor ? ((MapEnmWork*)sRouteActor->work)->collider.height : 8192;
    while (node) {
        if (!(node->flags & LIST_NODE_FLAG_SKIP)) {
            collider = node->owner;
            radius = collider->radius + actorRadius;
            dx = NativeAbs(pos->x - collider->x);
            dy = NativeAbs(pos->y * 2 - collider->y);
            dz = pos->z - collider->z;
            if (dx < radius && dy < radius && dz < actorHeight && -dz < collider->height) {
                /* Scale before squaring, as native collision does, to keep
                 * fixed-point circle arithmetic inside signed 32-bit range. */
                dx >>= 4; dy >>= 4; radius >>= 4;
                if (dx * dx + dy * dy < radius * radius) return 0;
            }
        }
        node = node->next;
    }
    return 1;
}
static int NativeRouteClear(FldPos pos) {
    if (IsFldPosBlocked(&pos) || !NativeRoutePropsClear(&pos)) return 0;
    if (!sRoutePlayer) return 1;
    /* Match the native controller's six-pixel front/back footprint. */
    pos.y -= 1536;
    if (IsFldPosBlocked(&pos) || GetFldPosGround(&pos) != pos.ground) return 0;
    pos.y += 3072;
    if (IsFldPosBlocked(&pos) || GetFldPosGround(&pos) != pos.ground) return 0;
    return 1;
}
static int NativeRouteActorHits(const FldPos* other, const FldPos* pos, const FldPos* origin) {
    int from[3], to[3];
    static const int half[3] = {12 << 8, 6 << 8, 16 << 8};
    to[0] = pos->x - other->x;
    to[1] = pos->y + pos->z - other->y - other->z;
    to[2] = pos->z - other->z;
    if (!origin) return FieldRouteSegmentBox(to, to, half);
    from[0] = origin->x - other->x;
    from[1] = origin->y + origin->z - other->y - other->z;
    from[2] = origin->z - other->z;
    /* Room entry places party members together. Permit leaving an existing
     * overlap, but the separate destination check still requires a free tile. */
    if (NativeAbs(from[0]) < half[0] && NativeAbs(from[1]) < half[1] && NativeAbs(from[2]) < half[2]) return 0;
    return FieldRouteSegmentBox(from, to, half);
}
static int NativeRouteActorsClear(const FldPos* pos, const FldPos* origin) {
    MapEnmWork* other;
    u8 i;
    for (i = 0; i < 3; i++) if (gNativePartyHealth.hp[i] && (!sRoutePlayer || i != gNativeParty) &&
        NativeRouteActorHits(&sPartyPos[i], pos, origin)) return 0;
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] && sEnemyTasks[i] != sRouteActor) {
        other = sEnemyTasks[i]->work;
        if (NativeRouteActorHits(&other->obj.fieldPosition, pos, origin)) return 0;
    }
    return 1;
}
static int NativeRouteEdge(int from, int to, void* context) {
    FldPos probe;
    u8 sample;
    (void)context;
    if (!sRouteValid[to] ||
        NativeAbs(sRoutePos[to].z - sRoutePos[from].z) > (sRoutePlayer ? 0 : (16 << 8))) return 0;
    if (!NativeRouteActorsClear(&sRoutePos[to], &sRoutePos[from])) return 0;
    /* Sweep quarter segments, including diagonal edges. The native actor's
     * front/back footprint must fit all along the projected movement. */
    for (sample = 1; sample < 4; sample++) {
        probe = sRoutePos[from];
        probe.x += (sRoutePos[to].x - probe.x) * sample / 4;
        probe.y += (sRoutePos[to].y + sRoutePos[to].z - probe.y - probe.z) * sample / 4;
        probe.z += (sRoutePos[to].z - probe.z) * sample / 4;
        probe.ground += (sRoutePos[to].ground - probe.ground) * sample / 4;
        probe.y -= probe.z - sRoutePos[from].z;
        if (!NativeRouteClear(probe)) return 0;
    }
    if (!NativeRouteActorsClear(&sRoutePos[to], NULL)) return 0;
    return 1;
}
/* Native geometry owns each directed link. Keep the resource-aware planner
 * authoritative for preview costs and the exact path consumed by the actor. */
static int NativeTacticsLinks(int from, int index, int* to, int* movement,
    int* action, int* kind, void* context) {
    static const int offset[8] = {-1,1,-9,9,-10,-8,8,10};
    static const int dx[8] = {-1,1,0,0,-1,1,-1,1};
    static const int dy[8] = {0,0,-1,1,-1,-1,1,1};
    int direction,count=0,x,y;
    (void)context;
    /* Walking links always cost one. Boundary nodes cannot add a reachable
     * destination, so avoid expensive native collision probes there. */
    if (sPlayerRoute.move[from]>=gNativeMoveLeft) return 0;
    if (!(sPlayerLinksKnown[from/8] & (1 << (from%8)))) {
        sPlayerLinksKnown[from/8]|=1<<(from%8);
        sPlayerLinks[from]=0;
        for (direction=0;direction<8;direction++) {
            x=from%9+dx[direction];y=from/9+dy[direction];
            if (x<0||x>=9||y<0||y>=9) continue;
            if (NativeRouteEdge(from,x+y*9,NULL)) sPlayerLinks[from]|=1<<direction;
        }
    }
    for (direction=0;direction<8;direction++) {
        if (!(sPlayerLinks[from] & (1 << direction))) continue;
        if (count++!=index) continue;
        *to=from+offset[direction];*movement=1;*action=0;*kind=FIELD_EDGE_WALK;
        return 1;
    }
    return 0;
}
static u32 NativeRouteFingerprint(void) {
    u32 hash=2166136261u;
    int i;
    ListNode* node=((ListPool*)ColliderGetPool(6))->activeHead;
    Collider* collider;
    FldPos* pos;
    /* Cache only while geometry and budgets stay unchanged. Confirmation
     * always performs full collision validation, regardless of this hash. */
#define ROUTE_HASH(value) hash=((hash<<5)|(hash>>27))^(u32)(value)
    pos=&gFieldState->actor.fieldPosition;
    ROUTE_HASH(pos->x);ROUTE_HASH(pos->y);ROUTE_HASH(pos->z);ROUTE_HASH(pos->ground);
    ROUTE_HASH(gNativeParty);
    ROUTE_HASH(gNativePreview);ROUTE_HASH(gFieldState->actor.angle);
    for (i=0;i<3;i++) {
        pos=&sPartyPos[i];
        ROUTE_HASH(gNativePartyHealth.hp[i]);
        ROUTE_HASH(pos->x);ROUTE_HASH(pos->y);ROUTE_HASH(pos->z);
    }
    for (i=0;i<6;i++) {
        ROUTE_HASH(sEnemyTasks[i]);
        if (sEnemyTasks[i]) {
            pos=&((MapEnmWork*)sEnemyTasks[i]->work)->obj.fieldPosition;
            ROUTE_HASH(pos->x);ROUTE_HASH(pos->y);ROUTE_HASH(pos->z);
        }
    }
    while (node) {
        collider=node->owner;
        ROUTE_HASH(node->flags);
        ROUTE_HASH(collider->x);ROUTE_HASH(collider->y);ROUTE_HASH(collider->z);
        ROUTE_HASH(collider->radius);ROUTE_HASH(collider->height);
        node=node->next;
    }
#undef ROUTE_HASH
    return hash;
}
/* Preserve native ground beneath a prop while previewing its standing top.
 * Only centers inside the original collider footprint are admitted; walking
 * off an edge still uses the separate native drop/jump controller. */
static int NativeRoutePropTop(const FldPos* pos) {
    ListNode* node=sRouteObstacles->activeHead;
    Collider* collider;
    s32 dx,dy,radius;
    while (node) {
        if (!(node->flags & LIST_NODE_FLAG_SKIP)) {
            collider=node->owner;
            if (pos->z==collider->z-collider->height) {
                dx=NativeAbs(pos->x-collider->x)>>4;
                dy=NativeAbs(pos->y*2-collider->y)>>4;
                radius=collider->radius>>4;
                if (dx<radius && dy<radius && dx*dx+dy*dy<radius*radius) return 1;
            }
        }
        node=node->next;
    }
    return 0;
}
static void NativeBuildRoute(FldPos origin, Task* task) {
    FldPos* pos;
    MapCell* cell;
    int i;
    s32 floor;
    sRouteActor = task;
    sRouteObstacles = ColliderGetPool(6);
    for (i = 0; i < FIELD_ROUTE_CELLS; i++) {
        pos = &sRoutePos[i];
        *pos = origin;
        pos->x += (i % 9 - 4) * 4096;
        pos->y += (i / 9 - 4) * 2048;
        sRouteValid[i] = 0;
        if (pos->x < 0 || pos->y + pos->z < 0 ||
            pos->x >= (gMapRoomState->cols << 13) ||
            pos->y + pos->z >= (gMapRoomState->rows << 12)) continue;
        cell = MapCellAtPos(pos->x, pos->y + pos->z);
        if (!cell) continue;
        floor = sRoutePlayer ? GetFldPosGround(pos) : GetFldPosFloor(pos);
        /* An upper ledge may be walkable above a void lower surface.
         * Reject the sampled floor, not the unrelated surface below it. */
        if (floor == 0x100000 || floor == -0x100000) continue;
        if (sRoutePlayer && origin.z<origin.ground && NativeRoutePropTop(pos)) {
            /* z is the top height; ground remains the terrain below it. */
            pos->ground=GetFldPosGround(pos);
        } else {
            pos->y += pos->z - floor;
            pos->z = pos->ground = floor;
        }
        if (NativeRouteClear(*pos)) sRouteValid[i] = 1;
    }
}
static int NativeEnemyRoute(Task* task, const FldPos* target) {
    MapEnmWork* work = task->work;
    FldPos origin = work->obj.fieldPosition;
    sRoutePlayer = 0;
    NativeBuildRoute(origin, task);
    return FieldRouteStep(&sEnemyRoute, (target->x - origin.x) / 4096,
        (target->y + target->z - origin.y - origin.z) / 2048, NativeRouteEdge, NULL);
}
static int NativeClimbLinks(int from,int index,int* to,int* movement,
    int* action,int* kind,void* context) {
    int direction,destination,count=0;
    int x,y;
    static const int dx[8]={-1,1,0,0,-1,1,-1,1};
    static const int dy[8]={0,0,-1,1,-1,-1,1,1};
    FldPos landing;
    u8 angle=gFieldState->actor.angle+128;
    (void)context;
    if (from>=7) {
        if (sPlayerRoute.move[from]>=gNativeMoveLeft) return 0;
        if (!(sPlayerLinksKnown[from/8] & (1<<(from%8)))) {
            sPlayerLinksKnown[from/8]|=1<<(from%8);sPlayerLinks[from]=0;
            for (direction=0;direction<8;direction++) {
                x=(from-7)%7+dx[direction];y=(from-7)/7+dy[direction];
                if (x<0||x>=7||y<0||y>=7) continue;
                if (NativeRouteEdge(from,7+x+y*7,NULL)) sPlayerLinks[from]|=1<<direction;
            }
        }
        for (direction=0;direction<8;direction++) if (sPlayerLinks[from] & (1<<direction)) {
            if (count++!=index) continue;
            *to=from+dx[direction]+dy[direction]*7;
            *movement=1;*action=0;*kind=FIELD_EDGE_WALK;return 1;
        }
        return 0;
    }
    for (direction=-1;direction<=1;direction+=2) {
        destination=from+direction;
        if (destination<0||destination>=7||!sRouteValid[destination]||
            NativeAbs(sRoutePos[destination].z-sRoutePos[from].z)<=48) continue;
        if (sRouteValid[destination]==2) {
            landing=sRoutePos[destination];
            landing.x-=SIN(angle)*10;landing.y+=COS(angle)*10;
            if (!NativeRouteActorsClear(&landing,&sRoutePos[from])||
                !NativeRouteActorsClear(&sRoutePos[destination],&landing)||
                !NativeRouteClear(sRoutePos[destination])) continue;
        } else if (!NativeRouteActorsClear(&sRoutePos[destination],&sRoutePos[from])) continue;
        if (!NativeRouteActorsClear(&sRoutePos[destination],NULL)) continue;
        /* The center of the landing grid aliases the final descent waypoint;
         * no free synthetic link or duplicate movement charge is introduced. */
        if (gNativePreview==3 && sRouteValid[destination]==2) {
            if (!sRouteValid[31]) continue;
            destination=31;
        }
        if (count++!=index) continue;
        *to=destination;*movement=1;*action=0;*kind=FIELD_EDGE_CLIMB;
        return 1;
    }
    return 0;
}
static void NativeClimbFloor(FldPos origin,int connected) {
    int i;
    FldPos* pos;
    for (i=7;i<56;i++) {
        pos=&sRoutePos[i];*pos=origin;
        pos->x+=((i-7)%7-3)*4096;pos->y+=((i-7)/7-3)*2048;
        sRouteValid[i]=0;
        if (!connected || pos->x<0 || pos->y+pos->z<0 ||
            pos->x>=(gMapRoomState->cols<<13) || pos->y+pos->z>=(gMapRoomState->rows<<12)) continue;
        if (GetFldPosGround(pos)==origin.z && NativeRouteClear(*pos)) sRouteValid[i]=1;
    }
    if (sRouteValid[31]) sRouteValid[31]=2;
}
static void NativePreviewInput(u16 pressed) {
    int i,action,cost,length,floorNode,target,routeCost=-1,reachCount=0;
    int opened=!gNativePreview;
    u32 geometry;
    FldWork* player = ((Task*)gFieldState->tasks2.head.activeHead->owner)->work;
    s32 lowerLimit,upperLimit;
    MapCell* stair;
    if (player->state == FLD_STATE_CLIMB) {
        if (!gNativePreview) {sCursorX=sCursorY=0;gNativePreview=2;}
        if (pressed & (B_BUTTON | SELECT_BUTTON)) {gNativePreview = 0;return;}
        if (pressed & R_BUTTON) {gNativePreview=gNativePreview==2 ? 3 : 2;sCursorX=sCursorY=0;opened=1;}
        if ((pressed & DPAD_UP) && sCursorY>-3) sCursorY--;
        if ((pressed & DPAD_DOWN) && sCursorY<3) sCursorY++;
        if (gNativePreview==3) {
            if ((pressed & DPAD_LEFT) && sCursorX>-3) sCursorX--;
            if ((pressed & DPAD_RIGHT) && sCursorX<3) sCursorX++;
        }
        sClimbPreviewDirection=gNativePreview==3 ? DPAD_DOWN : sCursorY<0 ? DPAD_UP : sCursorY>0 ? DPAD_DOWN : 0;
        sPartyPos[gNativeParty]=gFieldState->actor.fieldPosition;
        geometry=NativeRouteFingerprint();
        if (opened || (pressed & A_BUTTON) || geometry!=sPlayerGeometry ||
            sPlayerMoveBudget!=gNativeMoveLeft || sPlayerActionBudget!=gNativeActionLeft) {
            lowerLimit = gFieldState->actor.fieldPosition.ground;
            if ((player->collider.standFlags & COLLIDER_STAND_OVER_PLATFORM) &&
                player->collider.platformZ < lowerLimit) lowerLimit = player->collider.platformZ;
            stair=MapCellAtPos(player->targetX,player->targetY+gFieldState->actor.fieldPosition.ground);
            upperLimit=stair && (stair->flags & MAP_CELL_FLAG_STAIRS) ? stair->upperZ : -0x100000;
            sRoutePlayer=1;sRouteActor=NULL;sRouteObstacles=ColliderGetPool(6);
            for (i=0;i<7;i++) {
                sRoutePos[i]=gFieldState->actor.fieldPosition;
                sRoutePos[i].z=((player->targetZ>>12)+i-3)*4096;
                if (i>3 && sRoutePos[i].z>lowerLimit) sRoutePos[i].z=lowerLimit;
                sRouteValid[i]=sRoutePos[i].z>=upperLimit &&
                    (i<=3 || (i==4 ? sRoutePos[i].z>gFieldState->actor.fieldPosition.z+48 :
                        sRoutePos[i].z>sRoutePos[i-1].z+48));
                if (sRouteValid[i] && i>3 && sRoutePos[i].z==lowerLimit) {
                    u8 angle=gFieldState->actor.angle+128;
                    sRouteValid[i]=2;sRoutePos[i].ground=lowerLimit;
                    sRoutePos[i].x+=SIN(angle)*10;sRoutePos[i].y-=COS(angle)*10;
                }
                if (sRouteValid[i] && i<3 && sRoutePos[i].z==upperLimit) sRouteValid[i]=3;
            }
            sRouteValid[3]=1;floorNode=-1;
            for (i=4;i<7;i++) if (sRouteValid[i]==2) {floorNode=i;break;}
            if (gNativePreview==3) {
                if (floorNode>=0) NativeClimbFloor(sRoutePos[floorNode],1);
                else {
                    FldPos floor=gFieldState->actor.fieldPosition;
                    u8 angle=gFieldState->actor.angle+128;
                    floor.z=floor.ground=lowerLimit;
                    floor.x+=SIN(angle)*10;floor.y-=COS(angle)*10;
                    NativeClimbFloor(floor,0);
                }
            }
            for (i=0;i<11;i++) sPlayerLinksKnown[i]=0;
            gNativeClimbReachMask=0;
            if (FieldTacticsSearch(&sPlayerRoute,gNativePreview==3 ? 56 : 7,3,gNativeMoveLeft,0,NativeClimbLinks,NULL)>=0) {
                for (i=0;i<7;i++) if (i!=3 &&
                    sPlayerRoute.move[gNativePreview==3 && i==floorNode ? 31 : i]!=255)
                    gNativeClimbReachMask|=1<<((NativeAbs(i-3)-1)*2+(i>3));
            }
            for (i=0;i<FIELD_ROUTE_CELLS;i++) {
                gNativeReachCost[i]=gNativePreview==3 && sPlayerRoute.nodes && i<49 ? sPlayerRoute.move[i+7] : 255;
                if (gNativeReachCost[i]!=255 && gNativeReachCost[i]) reachCount++;
            }
            gNativeReachCount=reachCount;
            sPlayerGeometry=geometry;sPlayerMoveBudget=gNativeMoveLeft;sPlayerActionBudget=gNativeActionLeft;
        }
        target=gNativePreview==3 ? 7+(sCursorY+3)*7+sCursorX+3 : sCursorY+3;
        length=FieldTacticsPath(&sPlayerRoute,target,sPlayerPath,sPlayerEdge,
            FIELD_ROUTE_CELLS,&cost,&action);
        gNativeRouteCost=length>=0 ? cost : -1;
        if ((pressed & A_BUTTON) && gNativeRouteCost>0 && gNativeRouteCost<=gNativeMoveLeft) {
            gNativeMoveLeft-=gNativeRouteCost;
            gNativeDirection = sClimbPreviewDirection;
            sPathLength=length;sPathIndex=0;sPathStartX=gFieldState->actor.fieldPosition.z;
            gNativeCommands++;
            gNativePreview = 0;
            gNativeBusy = 5;
            sFrames = 0;
        }
        return;
    }
    if (gNativePreview>1) {gNativePreview=0;opened=1;}
    if (!gNativePreview) {
        gNativePreview = 1;
        sCursorX = sCursorY = 0;
    }
    if (pressed & (B_BUTTON | SELECT_BUTTON)) {gNativePreview = 0; return;}
    if ((pressed & R_BUTTON) && player->state == FLD_STATE_GROUND) {
        gNativePreview = 0;gNativeMenu = 6;gNativeDirection = 0;return;
    }
    if ((pressed & DPAD_LEFT) && sCursorX > -4) sCursorX--;
    if ((pressed & DPAD_RIGHT) && sCursorX < 4) sCursorX++;
    if ((pressed & DPAD_UP) && sCursorY > -4) sCursorY--;
    if ((pressed & DPAD_DOWN) && sCursorY < 4) sCursorY++;
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    geometry=NativeRouteFingerprint();
    if (opened || (pressed & A_BUTTON) || geometry!=sPlayerGeometry ||
        sPlayerMoveBudget!=gNativeMoveLeft || sPlayerActionBudget!=gNativeActionLeft) {
        sRoutePlayer = 1;
        NativeBuildRoute(gFieldState->actor.fieldPosition, NULL);
        for (i=0;i<11;i++) sPlayerLinksKnown[i]=0;
        FieldTacticsSearch(&sPlayerRoute,FIELD_ROUTE_CELLS,40,gNativeMoveLeft,
            gNativeActionLeft ? 1 : 0,NativeTacticsLinks,NULL);
        sPlayerGeometry=geometry;sPlayerMoveBudget=gNativeMoveLeft;sPlayerActionBudget=gNativeActionLeft;
        for (i=0;i<FIELD_ROUTE_CELLS;i++) {
            gNativeReachCost[i]=sPlayerRoute.nodes && sPlayerRoute.move[i]<=gNativeMoveLeft ? sPlayerRoute.move[i] : 255;
            if (gNativeReachCost[i]!=255 && gNativeReachCost[i]) reachCount++;
        }
        gNativeReachCount=reachCount;
    }
    if (FieldTacticsPath(&sPlayerRoute,(sCursorY+4)*9+sCursorX+4,
        sPlayerPath,sPlayerEdge,FIELD_ROUTE_CELLS,&cost,&action)>=0) routeCost=cost;
    gNativeRouteCost=routeCost;
    if ((pressed & A_BUTTON) && gNativeRouteCost > 0 && gNativeRouteCost <= gNativeMoveLeft) {
        sPathLength = gNativeRouteCost;
        sPathIndex = 0;
        sPathStartX = gFieldState->actor.fieldPosition.x;
        sPathStartY = gFieldState->actor.fieldPosition.y;
        gNativeMoveLeft -= sPathLength;
        gNativePreview = 0;
        gNativeBusy = 3;
        sFrames = 0;
        gNativeCommands++;
    }
}
static u16 NativeRouteWalk(void) {
    FldPos* target;
    FldPos* actor = &gFieldState->actor.fieldPosition;
    s32 dx, dy;
    if (sPathIndex >= sPathLength) {
        gNativeBusy = 0;
        gFieldState->actor.speed = 0;
        return 0;
    }
    target = &sRoutePos[sPlayerPath[sPathIndex]];
    dx = target->x - actor->x;
    dy = target->y + target->z - actor->y - actor->z;
    if (NativeAbs(dx) <= 512 && NativeAbs(dy) <= 512) {
        sPathIndex++;
        sPathStartX = actor->x;
        sPathStartY = actor->y;
        sFrames = 0;
        gFieldState->actor.speed = 0;
        return 0;
    }
    if (sFrames >= 40) {
        /* The original controller remains authoritative. A blocked route
         * stops safely and refunds segments that were never started. */
        gNativeMoveLeft += sPathLength - sPathIndex - 1;
        if (NativeAbs(actor->x - sPathStartX) + NativeAbs(actor->y - sPathStartY) < (4 << 8))
            gNativeMoveLeft++;
        gNativeBusy = 0;
        gFieldState->actor.speed = 0;
        return 0;
    }
    /* A projected diagonal is sixteen pixels across and eight deep.
     * Follow its swept line instead of cutting an L around the waypoint. */
    if (NativeAbs(dx) > 512 && NativeAbs(dy) > 512)
        return (dx < 0 ? DPAD_LEFT : DPAD_RIGHT) | (dy < 0 ? DPAD_UP : DPAD_DOWN);
    if (NativeAbs(dx) > NativeAbs(dy)) return dx < 0 ? DPAD_LEFT : DPAD_RIGHT;
    return dy < 0 ? DPAD_UP : DPAD_DOWN;
}
static u16 NativeRouteClimb(FldWork* player) {
    FldPos* actor=&gFieldState->actor.fieldPosition;
    if (player->state==FLD_STATE_GROUND && sPathIndex+1<sPathLength &&
        sPlayerEdge[sPathIndex]==FIELD_EDGE_CLIMB && sPlayerEdge[sPathIndex+1]==FIELD_EDGE_WALK) {
        /* Continue the same paid route after the original controller lands.
         * Validate the next edge from its actual landing, never teleport to
         * a predicted waypoint or renew the member's movement budget. */
        sRoutePos[sPlayerPath[sPathIndex]]=*actor;
        sRoutePlayer=1;sRouteActor=NULL;sRouteObstacles=ColliderGetPool(6);
        if (!NativeRouteEdge(sPlayerPath[sPathIndex],sPlayerPath[sPathIndex+1],NULL)) {
            gNativeMoveLeft+=sPathLength-sPathIndex-1;
            gNativeBusy=0;gNativeDirection=0;gFieldState->actor.speed=0;
            return 0;
        }
        sPathIndex++;sPathStartX=actor->x;sPathStartY=actor->y;
        sFrames=0;gNativeBusy=3;gNativeDirection=0;gFieldState->actor.speed=0;
        return 0;
    }
    if (player->state==FLD_STATE_GROUND || sFrames>=180) {
        /* Native climb-over/landing can end a route before its remaining
         * vertical waypoints. Only the segment already started is charged. */
        if (sPathIndex<sPathLength) {
            gNativeMoveLeft+=sPathLength-sPathIndex-1;
            if (sFrames>=180 && NativeAbs(actor->z-sPathStartX)<1024) gNativeMoveLeft++;
        }
        gNativeBusy=0;gNativeDirection=0;gFieldState->actor.speed=0;
        return 0;
    }
    if (player->state!=FLD_STATE_CLIMB) return 0;
    if (sPathIndex<sPathLength && NativeAbs(actor->z-sRoutePos[sPlayerPath[sPathIndex]].z)<=48) {
        /* The original easing stops slightly short of a floor. Keep the
         * final direction held so its boundary transition can land/detach;
         * this is part of the last segment, not an extra charged command. */
        if (sRouteValid[sPlayerPath[sPathIndex]]>1) return gNativeDirection;
        sPathIndex++;sPathStartX=actor->z;sFrames=0;
    }
    if (sPathIndex>=sPathLength) {
        gNativeBusy=0;gNativeDirection=0;gFieldState->actor.speed=0;
        return 0;
    }
    return gNativeDirection;
}
static void NativePreviewDraw(void) {
    int i, node, count;
    s16 x, y;
    FldPos* pos;
    if (!gNativePreview || !sValueTiles || !sValuePalette) return;
    if (gNativePreview == 2 || gNativePreview == 3) {
        if (sReachTiles) for (i = 0; i < 7; i++) if (i!=3 &&
            (gNativeClimbReachMask & (1 << ((NativeAbs(i-3)-1)*2+(i>3))))) {
            x = (sRoutePos[i].x - gFieldState->x) >> 8;
            y = (sRoutePos[i].y + sRoutePos[i].z - gFieldState->y) >> 8;
            DrawSprite(x - 12, y - 7, gCardValueDigitFrames[0], sReachTiles,
                sValuePalette, NULL, 0x800, 0);
        }
        if (gNativePreview==3) {
            if (sReachTiles) for (i=7;i<56;i++) if (gNativeReachCost[i-7]!=255) {
                pos=&sRoutePos[i];
                x=(pos->x-gFieldState->x)>>8;y=(pos->y+pos->z-gFieldState->y)>>8;
                DrawSprite(x-12,y-7,gCardValueDigitFrames[0],sReachTiles,sValuePalette,NULL,0x800,0);
            }
            count=gNativeRouteCost>0 && gNativeRouteCost<=gNativeMoveLeft ? gNativeRouteCost : 0;
            for (i=0;i<(count ? count : 1);i++) {
                node=count ? sPlayerPath[i] : 7+(sCursorY+3)*7+sCursorX+3;
                pos=&sRoutePos[node];
                x=(pos->x-gFieldState->x)>>8;y=(pos->y+pos->z-gFieldState->y)>>8;
                DrawSprite(x,y,gCardValueDigitFrames[count ? i+1 : 0],sValueTiles,sValuePalette,NULL,0,0);
            }
            return;
        }
        pos=&sRoutePos[sCursorY+3];
        x = (pos->x - gFieldState->x) >> 8;
        y = (pos->y + pos->z - gFieldState->y) >> 8;
        DrawSprite(x, y, gCardValueDigitFrames[gNativeRouteCost>0 && gNativeRouteCost<4 ? gNativeRouteCost : 0], sValueTiles,
            sValuePalette, NULL, 0, 0);
        return;
    }
    if (sReachTiles) for (i = 0; i < FIELD_ROUTE_CELLS; i++) if (gNativeReachCost[i] != 255 && gNativeReachCost[i]) {
        pos = &sRoutePos[i];
        x = (pos->x - gFieldState->x) >> 8;
        y = (pos->y + pos->z - gFieldState->y) >> 8;
        DrawSprite(x - 12, y - 7, gCardValueDigitFrames[0], sReachTiles,
            sValuePalette, NULL, 0x800, -0x1003 - (pos->y >> 8) * 4);
    }
    count = gNativeRouteCost > 0 && gNativeRouteCost <= gNativeMoveLeft ? gNativeRouteCost : 0;
    for (i = 0; i < (count ? count : 1); i++) {
        node = count ? sPlayerPath[i] : (sCursorY + 4) * 9 + sCursorX + 4;
        pos = &sRoutePos[node];
        x = (pos->x - gFieldState->x) >> 8;
        y = (pos->y + pos->z - gFieldState->y) >> 8;
        DrawSprite(x, y, gCardValueDigitFrames[count ? i + 1 : 0],
            sValueTiles, sValuePalette, NULL, 0, 0);
    }
}

static void NativeEnemyTurn(void) {
    u8 i, j, closest;
    s32 distance, best, dz;
    int step;
    FldPos pos;
    MapEnmWork* work;
    sPartyPos[gNativeParty] = gFieldState->actor.fieldPosition;
    gNativeTurn++;
    for (i = 0; i < 6 && !gNativeResult; i++) if (sEnemyTasks[i]) {
        work = sEnemyTasks[i]->work;
        closest = NativeEnemyTarget(i, &best, &dz);
        if (gNativeEnemyKind[i] == 2 && gNativeEnemyCharge[i]) {
            if (sEnemyTasks[i]->desc == &sCloudTaskDesc) {
                sCloudImpact = 24;
                for (j = 0; j < 3; j++) if (gNativePartyHealth.hp[j] && NativeCloudHits(i, &sPartyPos[j]))
                    if (FieldPartyDamage(&gNativePartyHealth, j, NativeCloudDamage(i))) gNativeResult = 1;
                gNativeEnemyCharge[i] = 0;
                continue;
            }
            if (sEnemyTasks[i]->desc == &sMarlTaskDesc) {
                sMarlImpact = 24;
                for (j = 0; j < 3; j++) if (gNativePartyHealth.hp[j] && NativeMarlHits(i, &sPartyPos[j]))
                    if (FieldPartyDamage(&gNativePartyHealth, j, NativeMarlDamage(i))) gNativeResult = 1;
                gNativeEnemyCharge[i] = 0;
                continue;
            }
            if (sEnemyTasks[i]->desc == &sJafarTaskDesc) {
                sJafarImpact = 24;
                if (best <= (96 << 8) && dz <= (32 << 8))
                    if (FieldPartyDamage(&gNativePartyHealth, closest, NativeJafarDamage())) gNativeResult = 1;
                gNativeEnemyCharge[i] = 0;
                continue;
            }
            if (sEnemyTasks[i]->desc == &sArmorTaskDesc) sArmorImpact = 24;
            for (j = 0; j < 3; j++) if (gNativePartyHealth.hp[j]) {
                distance = NativeAbs(work->obj.fieldPosition.x - sPartyPos[j].x) +
                    NativeAbs(work->obj.fieldPosition.y + work->obj.fieldPosition.z - sPartyPos[j].y - sPartyPos[j].z);
                if (distance <= (NativeBlastRange(i) << 8) && NativeAbs(work->obj.fieldPosition.z - sPartyPos[j].z) <= (24 << 8))
                    if (FieldPartyDamage(&gNativePartyHealth, j, gNativeGuard == 2 ? 0 :
                        gNativeGuard ? 1 : NativeBlastDamage(i))) gNativeResult = 1;
            }
            gNativeEnemyCharge[i] = 0;
            continue;
        }
        if (gNativeEnemyKind[i] == 2 && best <= (NativeWindupRange(i) << 8) && dz <= (32 << 8)) {
            gNativeEnemyCharge[i] = 1;
            continue;
        }
        if (gNativeEnemyKind[i] != 2 && best <= (FieldEnemyRange(gNativeEnemyKind[i]) << 8) &&
            dz <= (FieldEnemyHeight(gNativeEnemyKind[i]) << 8)) {
            if (FieldPartyDamage(&gNativePartyHealth, closest,
                gNativeGuard == 2 ? 0 : gNativeGuard ? 1 : FieldEnemyDamage(gNativeEnemyKind[i], gNativeFloor)))
                gNativeResult = 1;
        } else {
            step = NativeEnemyRoute(sEnemyTasks[i], &sPartyPos[closest]);
            if (step < 0) continue;
            pos = sRoutePos[step];
            work->obj.fieldPosition = pos;
            ColliderSetPosition(&work->collider, pos.x, pos.y, pos.z);
        }
    }
    gGameState.hp = gNativePartyHealth.hp[gNativeParty];
}

static void NativeEnemies(void) {
    ListNode* node = gFieldState->tasks4.head.activeHead;
    ListNode* next;
    Task* task;
    MapEnmWork* work;
    ListNode* effect;
    gNativeBossEffects = 0;
    while (node != NULL) {
        next = node->next;
        task = node->owner;
        work = task->work;
        /* Child tasks own only shadows and visual sparks. Advance those
         * while keeping the parent enemy's real-time AI frozen. */
        TaskPoolUpdate(&work->tasks);
        if (task->desc == &sArmorTaskDesc) {
            effect = work->tasks.head.activeHead;
            while (effect) {
                if (((Task*)effect->owner)->desc == &gTaskDescMapSpark) gNativeBossEffects++;
                effect = effect->next;
            }
        }
        if (!(work->flags & 0x8000)) {
            work->flags |= 0x8000;
            if (task->desc != &sArmorTaskDesc && task->desc != &sJafarTaskDesc && task->desc != &sMarlTaskDesc && task->desc != &sCloudTaskDesc) MapEnmSetAnim(work, 1, 1);
            ColliderSetDisabled(&work->collider, 0);
        }
        if (sAttack && !(work->flags & 0x2000) && MapEnmCheckAttacked(work)) {
            work->flags |= 0x2000;
            NativeDamageEnemy(task, sPlayedValue ? 5 + sPlayedValue / 2 : 3);
        } else if (task->desc != &sArmorTaskDesc && task->desc != &sJafarTaskDesc && task->desc != &sMarlTaskDesc && task->desc != &sCloudTaskDesc) {
            MapEnmUpdateAnim(work);
        }
        node = next;
    }
    gMapRoomState->flags &= ~(ROOM_FLAG_ENEMY_STRUCK | ROOM_FLAG_START_BATTLE);
}

static s32 NativeFireDistance(Task* task) {
    MapEnmWork* work = task->work;
    s32 dx = NativeAbs(work->obj.fieldPosition.x - gFieldState->actor.fieldPosition.x);
    s32 dy = NativeAbs(work->obj.fieldPosition.y - gFieldState->actor.fieldPosition.y);
    s32 dz = NativeAbs(work->obj.fieldPosition.z - gFieldState->actor.fieldPosition.z);
    return dz <= (24 << 8) ? dx + dy : 128 << 8;
}
static Task* NativeRangedTarget(int range) {
    ListNode* node = gFieldState->tasks4.head.activeHead;
    Task* target = NULL;
    Task* task;
    s32 best = range << 8, distance;
    if (gNativeFireChoice >= 0 && gNativeFireChoice < 6 &&
        sEnemyTasks[gNativeFireChoice] &&
        NativeFireDistance(sEnemyTasks[gNativeFireChoice]) < best)
        return sEnemyTasks[gNativeFireChoice];
    gNativeFireChoice = -1;
    while (node) {
        task = node->owner;
        distance = NativeFireDistance(task);
        if (distance < best) {best = distance;target = task;}
        node = node->next;
    }
    return target;
}
static Task* NativeFireTarget(void) {return NativeRangedTarget(128);}
static void NativeCloudSlash(void) {
    Task* target = NativeRangedTarget(64);
    if (target) NativeDamageEnemy(target, 12 + sPlayedValue);
}
static void NativeCycleFireTarget(void) {
    int card = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
    int range = card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY && NativeHero(gNativeParty) == FIELD_CLOUD ? 64 : 128;
    Task* current = NativeRangedTarget(range);
    int i, slot = -1;
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] && sEnemyTasks[i] == current) slot = i;
    for (i = 1; i <= 6; i++) {
        int next = (slot + i) % 6;
        if (sEnemyTasks[next] && NativeFireDistance(sEnemyTasks[next]) < (range << 8)) {
            gNativeFireChoice = next;
            return;
        }
    }
    gNativeFireChoice = -1;
}
static void NativeFire(void) {
    Task* target = NativeFireTarget();
    if (target) NativeDamageEnemy(target, 6 + sPlayedValue + (NativeHero(gNativeParty) == FIELD_DONALD ? 3 : 0));
}

u16 gNativeSkillDamage[6];
static int NativeShieldHits(u8 slot) {
    FldPos* pos = &((MapEnmWork*)sEnemyTasks[slot]->work)->obj.fieldPosition;
    return NativeAbs(pos->x - gFieldState->actor.fieldPosition.x) +
        NativeAbs(pos->y - gFieldState->actor.fieldPosition.y) <= (48 << 8) &&
        NativeAbs(pos->z - gFieldState->actor.fieldPosition.z) <= (24 << 8);
}
static void NativeShieldSpin(void) {
    u8 i;
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] && NativeShieldHits(i))
        NativeDamageEnemy(sEnemyTasks[i], 4 + sPlayedValue);
}

static void NativeCardIntentDraw(void) {
    Task* target;
    MapEnmWork* work;
    int card = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
    u8 i;
    u16 damage;
    s16 x, y;
    gNativeFireTarget = -1;
    gNativeFireDamage = 0;
    gNativeCureHeal = 0;
    for (i = 0; i < 6; i++) gNativeSkillDamage[i] = 0;
    if (gNativeResult || gNativeEnemyFrames || gNativeBusy || gNativePreview ||
        !gNativeActionLeft || card < 0 || gNativeDeck.stocked || gNativeReward) return;
    if (gNativeDeck.kind[card] == FIELD_CARD_CURE) {
        FldPos* pos;
        i = NativeCureTarget();
        gNativeCureHeal = NativeCureRecovery(i, gNativeDeck.value[card]);
        if (!sValueTiles || !sValuePalette) return;
        pos = &sPartyPos[i];
        x = (pos->x - gFieldState->x) >> 8;
        y = ((pos->y + pos->z - gFieldState->y) >> 8) - 40;
        if (gNativeCureHeal >= 10) DrawSprite(x - 6, y, gCardValueDigitFrames[gNativeCureHeal / 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
        DrawSprite(x + 2, y, gCardValueDigitFrames[gNativeCureHeal % 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
        return;
    }
    if (NativeHero(gNativeParty) == FIELD_GOOFY && gNativeDeck.kind[card] == FIELD_CARD_KEY) {
        for (i = 0; i < 6; i++) if (sEnemyTasks[i] && NativeShieldHits(i)) {
            damage = 4 + gNativeDeck.value[card] + gNativeRoster.power[NativeHero(gNativeParty)];
            if (gNativeDeck.value[card] && gNativeDeck.value[card] < 3 + gNativeFloor) damage = 0;
            if (damage > gNativeEnemyHp[i]) damage = gNativeEnemyHp[i];
            gNativeSkillDamage[i] = damage;
            if (!sValueTiles || !sValuePalette) continue;
            work = sEnemyTasks[i]->work;
            x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
            y = ((work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8) - 28;
            if (damage >= 10) DrawSprite(x - 6, y, gCardValueDigitFrames[damage / 10],
                sValueTiles, sValuePalette, NULL, 0, 0);
            DrawSprite(x + 2, y, gCardValueDigitFrames[damage % 10],
                sValueTiles, sValuePalette, NULL, 0, 0);
        }
        return;
    }
    if (gNativeDeck.kind[card] != FIELD_CARD_FIRE &&
        !((NativeHero(gNativeParty) == FIELD_DONALD || NativeHero(gNativeParty) == FIELD_CLOUD) && gNativeDeck.kind[card] == FIELD_CARD_KEY)) return;
    target = NativeRangedTarget(gNativeDeck.kind[card] == FIELD_CARD_KEY && NativeHero(gNativeParty) == FIELD_CLOUD ? 64 : 128);
    if (!target) return;
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] == target) {
        gNativeFireTarget = i;
        damage = (gNativeDeck.kind[card] == FIELD_CARD_KEY && NativeHero(gNativeParty) == FIELD_CLOUD ? 12 : 6) + gNativeDeck.value[card] + (NativeHero(gNativeParty) == FIELD_DONALD ? 3 : 0) + gNativeRoster.power[NativeHero(gNativeParty)];
        if (gNativeDeck.value[card] && gNativeDeck.value[card] < 3 + gNativeFloor) damage = 0;
        if (damage > gNativeEnemyHp[i]) damage = gNativeEnemyHp[i];
        gNativeFireDamage = damage;
        if (!sValueTiles || !sValuePalette) return;
        work = target->work;
        x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
        y = ((work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8) - 28;
        if (damage >= 10) DrawSprite(x - 6, y, gCardValueDigitFrames[damage / 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
        DrawSprite(x + 2, y, gCardValueDigitFrames[damage % 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
        return;
    }
}

static int NativeCureEligible(u8 member) {
    if (member == gNativeParty) return 1;
    return NativeAbs(sPartyPos[member].x - gFieldState->actor.fieldPosition.x) +
        NativeAbs(sPartyPos[member].y - gFieldState->actor.fieldPosition.y) <= (96 << 8) &&
        NativeAbs(sPartyPos[member].z - gFieldState->actor.fieldPosition.z) <= (24 << 8);
}
static u8 NativeCureTarget(void) {
    u8 i, target = gNativeParty;
    u16 missing = 0, amount;
    if (gNativeCureChoice >= 0 && gNativeCureChoice < 3 && NativeCureEligible(gNativeCureChoice))
        return gNativeCureChoice;
    gNativeCureChoice = -1;
    for (i = 0; i < 3; i++) {
        if (!NativeCureEligible(i)) continue;
        amount = gNativePartyHealth.maxHp[i] - gNativePartyHealth.hp[i];
        if (amount > missing) {missing = amount;target = i;}
    }
    return target;
}
static void NativeCycleCureTarget(void) {
    u8 current = NativeCureTarget(), i, next;
    for (i = 1; i <= 3; i++) {
        next = (current + i) % 3;
        if (NativeCureEligible(next)) {gNativeCureChoice = next;return;}
    }
}
static u16 NativeCureRecovery(u8 target, u8 value) {
    u16 amount = 8 + value + (NativeHero(gNativeParty) == FIELD_DONALD ? 8 : NativeHero(gNativeParty) == FIELD_RALLY ? 4 : 0);
    u16 missing = gNativePartyHealth.maxHp[target] - gNativePartyHealth.hp[target];
    return amount < missing ? amount : missing;
}
static void NativeCure(void) {
    u8 target = NativeCureTarget();
    FieldPartyHeal(&gNativePartyHealth, target, NativeCureRecovery(target, sPlayedValue));
    gGameState.hp = gNativePartyHealth.hp[gNativeParty];
}

u16 gNativeSleightDamage[6];
u16 gNativeSleightHeal[3];
static u16 NativeSleightRecovery(u8 member, int value, int recipe) {
    u16 amount = 12 + value + (recipe ? 8 : 0) +
        (NativeHero(gNativeParty) == FIELD_DONALD ? 8 : NativeHero(gNativeParty) == FIELD_RALLY ? 4 : 0) +
        (recipe && (gNativeRoster.sleights[NativeHero(gNativeParty)] & 4) ? 4 : 0);
    u16 missing = gNativePartyHealth.maxHp[member] - gNativePartyHealth.hp[member];
    return amount < missing ? amount : missing;
}
static int NativeSleightHits(u8 slot, int kind) {
    MapEnmWork* work = sEnemyTasks[slot]->work;
    return NativeAbs(work->obj.fieldPosition.x - gFieldState->actor.fieldPosition.x) +
        NativeAbs(work->obj.fieldPosition.y - gFieldState->actor.fieldPosition.y) <=
        ((kind == FIELD_CARD_FIRE ? 144 : 64) << 8) &&
        NativeAbs(work->obj.fieldPosition.z - gFieldState->actor.fieldPosition.z) <= (24 << 8);
}
static u16 NativeSleightPower(int kind, int value, int recipe) {
    return 8 + value + (recipe ? 6 : 0) + (NativeHero(gNativeParty) == FIELD_DONALD && kind == FIELD_CARD_FIRE ? 4 : 0) +
        (kind < 3 && recipe && (gNativeRoster.sleights[NativeHero(gNativeParty)] & (1 << kind)) ? 4 : 0);
}
static void NativeSleightIntentDraw(void) {
    int kind, value;
    u8 i;
    u16 damage;
    s16 x, y;
    MapEnmWork* work;
    for (i = 0; i < 6; i++) gNativeSleightDamage[i] = 0;
    for (i = 0; i < 3; i++) gNativeSleightHeal[i] = 0;
    if (gNativeResult || gNativeEnemyFrames || gNativeBusy || gNativePreview || gNativeReward ||
        !gNativeActionLeft || !FieldDeckSleightPreview(&gNativeDeck, &kind, &value) ||
        kind == FIELD_CARD_GUARD) return;
    if (kind == FIELD_CARD_CURE) {
        for (i = 0; i < 3; i++) {
            FldPos* pos = &sPartyPos[i];
            damage = NativeSleightRecovery(i, value, FieldDeckRecipe(&gNativeDeck));
            gNativeSleightHeal[i] = damage;
            if (!sValueTiles || !sValuePalette) continue;
            x = (pos->x - gFieldState->x) >> 8;
            y = ((pos->y + pos->z - gFieldState->y) >> 8) - 40;
            if (damage >= 10) DrawSprite(x - 6, y, gCardValueDigitFrames[damage / 10],
                sValueTiles, sValuePalette, NULL, 0, 0);
            DrawSprite(x + 2, y, gCardValueDigitFrames[damage % 10],
                sValueTiles, sValuePalette, NULL, 0, 0);
        }
        return;
    }
    for (i = 0; i < 6; i++) if (sEnemyTasks[i] && NativeSleightHits(i, kind)) {
        damage = NativeSleightPower(kind, value, FieldDeckRecipe(&gNativeDeck)) + gNativeRoster.power[NativeHero(gNativeParty)];
        if (value && value < 3 + gNativeFloor) damage = 0;
        if (damage > gNativeEnemyHp[i]) damage = gNativeEnemyHp[i];
        gNativeSleightDamage[i] = damage;
        if (!sValueTiles || !sValuePalette) continue;
        work = sEnemyTasks[i]->work;
        x = (work->obj.fieldPosition.x - gFieldState->x) >> 8;
        y = ((work->obj.fieldPosition.y + work->obj.fieldPosition.z - gFieldState->y) >> 8) - 28;
        if (damage >= 10) DrawSprite(x - 6, y, gCardValueDigitFrames[damage / 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
        DrawSprite(x + 2, y, gCardValueDigitFrames[damage % 10],
            sValueTiles, sValuePalette, NULL, 0, 0);
    }
}
u16 gNativeSleights;
static void NativeSleight(void) {
    int kind, value;
    int recipe = FieldDeckRecipe(&gNativeDeck);
    u8 i;
    if (!FieldDeckSleight(&gNativeDeck, &kind, &value)) return;
    sPlayedValue = value;
    NativePartyPose(gNativeParty);
    if (kind == FIELD_CARD_CURE) {
        for (i = 0; i < 3; i++) FieldPartyHeal(&gNativePartyHealth, i, NativeSleightRecovery(i, value, recipe));
        gGameState.hp = gNativePartyHealth.hp[gNativeParty];
    } else if (kind == FIELD_CARD_GUARD) gNativeGuard = 2;
    else {
        for (i = 0; i < 6; i++) if (sEnemyTasks[i] && NativeSleightHits(i, kind))
            NativeDamageEnemy(sEnemyTasks[i], NativeSleightPower(kind, value, recipe));
    }
    gNativeSleights++;
    gNativeActionLeft = 0;
    gNativeCommands++;
}

static void NativeSyncHealth(void) {
    if (gGameState.hp > gNativePartyHealth.maxHp[gNativeParty])
        gGameState.hp = gNativePartyHealth.maxHp[gNativeParty];
    if (gGameState.hp < 0) gGameState.hp = 0;
    gNativePartyHealth.hp[gNativeParty] = gGameState.hp;
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
    /* Legacy field prizes read/write active HP; clamp to this member
     * and mirror it into the authoritative party state. */
    NativeSyncHealth();
    if (gNativeResult && (pressed & SELECT_BUTTON)) {
        gNativeResult = 0;
        sTerminalSaveCleared = 0;
        sCarryTurn = 0;
        gNativeFloor = 0;
        gNativeSeed += 0x9e3779b9;
        gNativeKills = gNativeChests = 0;
        gGameState.hp = gGameState.progression.maxHp;
        FieldPartyInit(&gNativePartyHealth);
        FieldRosterInit(&gNativeRoster);
        FieldDeckInit(&gNativeDeck);
        NativeBuildWorld();
        ModeRequest(&sNativeMode, 0);
        return;
    }
    if (gNativeProgressReward) {
        u16 options = NativeRecruitEligible() ? 5 : 4;
        NativeRecruitInit();
        if (pressed & (L_BUTTON | DPAD_LEFT)) sProgressHero = (sProgressHero + 2) % 3;
        else if (pressed & (R_BUTTON | DPAD_RIGHT)) sProgressHero = (sProgressHero + 1) % 3;
        if (pressed & DPAD_UP) sProgressKind = (sProgressKind + options - 1) % options;
        else if (pressed & DPAD_DOWN) sProgressKind = (sProgressKind + 1) % options;
        if ((pressed & A_BUTTON) && !gNativeBusy && !gNativeEnemyFrames &&
            (sProgressKind == 4 ? FieldRosterRecruit(&gNativeRoster, FIELD_CLOUD) :
                FieldRosterUpgrade(&gNativeRoster, NativeHero(sProgressHero), sProgressKind))) {
            gNativeProgressReward = 0;NativeRecruitFinish();
        }
        else if ((raw & (START_BUTTON | SELECT_BUTTON)) == (START_BUTTON | SELECT_BUTTON) &&
            (pressed & (START_BUTTON | SELECT_BUTTON)) && !gNativeBusy && !gNativeEnemyFrames) NativeWriteSuspend();
        pressed = 0;
    }
    if (gNativeAssembly) {
        if (pressed & (L_BUTTON | DPAD_LEFT)) sAssemblyChoice = (sAssemblyChoice + 2) % 3;
        else if (pressed & (R_BUTTON | DPAD_RIGHT)) sAssemblyChoice = (sAssemblyChoice + 1) % 3;
        if (pressed & DPAD_UP) NativeAssemblyCycle(1);
        else if (pressed & DPAD_DOWN) NativeAssemblyCycle(-1);
        if (pressed & (A_BUTTON | START_BUTTON)) NativeAssemblyCommit();
        pressed = 0;
    }
    if (gNativeReward) {
        if (pressed & (L_BUTTON | DPAD_LEFT)) gNativeRewardChoice = (gNativeRewardChoice + 2) % 3;
        else if (pressed & (R_BUTTON | DPAD_RIGHT)) gNativeRewardChoice = (gNativeRewardChoice + 1) % 3;
        if (pressed & A_BUTTON) {
            u32 rewardSeed = (sRewardSeed % 4 + gNativeRewardChoice) % 4;
            rewardSeed |= (((sRewardSeed >> 8) + gNativeRewardChoice) % 5) << 8;
            FieldDeckReward(&gNativeDeck, rewardSeed);
            FieldPartyHeal(&gNativePartyHealth, 0, 12);
            FieldPartyHeal(&gNativePartyHealth, 1, 12);
            FieldPartyHeal(&gNativePartyHealth, 2, 12);
            gGameState.hp = gNativePartyHealth.hp[gNativeParty];
            gNativeReward = 0;
        }
        /* Choosing a reward cannot also commit a field action or save an
         * opened chest before its reward has been granted. */
        pressed = 0;
    }
    if (!gNativeResult && gNativeBusy == 2 && player->state == FLD_STATE_LEDGE_HANG) {
        if (pressed & SELECT_BUTTON) {
            gNativeMenu = gNativeMenu == 9 ? 0 : 9;gNativeMenuChoice = 0;
            raw = pressed = 0;
        } else if (gNativeMenu == 9) {
            if (pressed & B_BUTTON) {gNativeMenu = 0;raw = pressed = 0;}
            else if (pressed & A_BUTTON) {
                raw = pressed = gNativeMenuChoice ? B_BUTTON : DPAD_UP;
                gNativeMenu = 0;gNativeMenuChoice = 0;
            } else {
                if (pressed & (DPAD_UP | DPAD_DOWN)) gNativeMenuChoice ^= 1;
                raw = pressed = 0;
            }
        }
    }
    if (!gNativeProgressReward && !gNativeAssembly && !gNativeReward && !gNativeResult && !gNativeBusy && !gNativeEnemyFrames &&
        !(gFieldState->flags & (FIELD_FLAG_FREEZE_PLAYER | FIELD_FLAG_ROOM_CREATE))) {
        /* The command panel shares the authoritative native actions and
         * budgets with shortcuts. Menu navigation never commits a field step. */
        if (!gNativePreview && !(raw & (L_BUTTON | START_BUTTON)) && (pressed & SELECT_BUTTON)) {
            gNativeMenu = gNativeMenu ? 0 : 1;
            pressed = 0;
        } else if (gNativeMenu) {
            if ((pressed & B_BUTTON) && gNativeMenu == 6) {
                gNativeMenu = 0;NativePreviewInput(0);pressed = 0;
            } else if ((pressed & B_BUTTON) && gNativeMenu == 8) {
                gNativeMenu = 1;gNativeMenuChoice = 3;pressed = 0;
            } else if (pressed & B_BUTTON) {gNativeMenu = gNativeMenu == 4 || gNativeMenu == 7 ? 1 : gNativeMenu >= 3 ? 2 : gNativeMenu == 2 ? 1 : 0;pressed = 0;}
            else if (gNativeMenu == 8) {
                if (pressed & DPAD_UP) gNativeMenuChoice = (gNativeMenuChoice + 2) % 3;
                else if (pressed & DPAD_DOWN) gNativeMenuChoice = (gNativeMenuChoice + 1) % 3;
                if ((pressed & A_BUTTON) && gNativePartyHealth.hp[gNativeMenuChoice] && player->state == FLD_STATE_GROUND) {
                    NativePartyActivate(gNativeMenuChoice);gNativeMenu = 0;gNativeMenuChoice = 3;
                }
                pressed = 0;
            } else if (gNativeMenu == 7) {
                if (pressed & A_BUTTON) {gNativeMenu = 0;raw = pressed = START_BUTTON;}
                else pressed = 0;
            } else if (gNativeMenu == 6) {
                if (pressed & DPAD_ANY) gNativeDirection = raw & DPAD_ANY;
                if ((pressed & A_BUTTON) && gNativeActionLeft && (!gNativeDirection || gNativeMoveLeft)) {
                    gNativeMenu = 0;raw = B_BUTTON | gNativeDirection;pressed = B_BUTTON;
                } else pressed = 0;
            } else if (gNativeMenu == 5) {
                if ((pressed & A_BUTTON) && gNativeActionLeft && FieldDeckReload(&gNativeDeck)) {
                    gNativeActionLeft = 0;
                    gNativeMenu = 0;
                }
                pressed = 0;
            } else if (gNativeMenu >= 3) {
                int card = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
                int ranged = gNativeDeck.stocked != 3 && card >= 0 && (gNativeDeck.kind[card] == FIELD_CARD_FIRE ||
                    (gNativeDeck.kind[card] == FIELD_CARD_KEY &&
                     (NativeHero(gNativeParty) == FIELD_DONALD || NativeHero(gNativeParty) == FIELD_CLOUD)));
                if ((pressed & (DPAD_UP | DPAD_DOWN)) && card >= 0 && gNativeDeck.stocked != 3) {
                    if (gNativeDeck.kind[card] == FIELD_CARD_CURE) NativeCycleCureTarget();
                    else if (ranged) NativeCycleFireTarget();
                }
                if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY && !ranged &&
                    gNativeDeck.stocked != 3 && (pressed & DPAD_ANY)) {
                    gFieldState->actor.angle = (pressed & DPAD_LEFT) ? 192 :
                        (pressed & DPAD_RIGHT) ? 64 : (pressed & DPAD_UP) ? 0 : 128;
                }
                if ((pressed & A_BUTTON) && (card >= 0 || gNativeDeck.stocked == 3) && gNativeActionLeft &&
                    (!ranged || gNativeFireTarget >= 0)) {gNativeMenu = 0;raw = pressed = A_BUTTON;}
                else pressed = 0;
            }
            else if (gNativeMenu == 2) {
                if (pressed & DPAD_UP) FieldDeckStock(&gNativeDeck);
                if (pressed & DPAD_DOWN) {
                    if (gNativeDeck.stocked) FieldDeckCancelStock(&gNativeDeck);
                    else gNativeMenu = 5;
                }
                if (pressed & (DPAD_LEFT | L_BUTTON)) FieldDeckCycle(&gNativeDeck, -1);
                if (pressed & (DPAD_RIGHT | R_BUTTON)) FieldDeckCycle(&gNativeDeck, 1);
                if ((pressed & A_BUTTON) && (FieldDeckHand(&gNativeDeck, gNativeDeck.selected) >= 0 || gNativeDeck.stocked == 3) &&
                    gNativeActionLeft) gNativeMenu = 3;
                pressed = 0;
            } else if (pressed & A_BUTTON) {
                u16 command = gNativeMenuChoice;
                gNativeMenu = 0;
                if (command == 0) {NativePreviewInput(0);pressed = 0;}
                else if (command == 1) {
                    u8 n;
                    if (gNativeDeck.stocked) {gNativeMenu = 2;pressed = 0;}
                    else {

                    for (n = 0; n < 5; n++) {
                        int card = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
                        if (card >= 0 && gNativeDeck.kind[card] == FIELD_CARD_KEY) break;
                        FieldDeckCycle(&gNativeDeck, 1);
                    }
                    gNativeMenu = n < 5 && gNativeActionLeft ? 4 : 1;
                    pressed = 0;
                    }
                } else if (command == 2) {gNativeMenu = 2;pressed = 0;}
                else if (command == 3) {gNativeMenu = 8;gNativeMenuChoice = gNativeParty;pressed = 0;}
                else if (command == 4) {gNativeMenu = 7;pressed = 0;}
                else {NativeWriteSuspend();pressed = 0;}
            } else {
                if (pressed & DPAD_UP) gNativeMenuChoice = (gNativeMenuChoice + 5) % 6;
                else if (pressed & DPAD_DOWN) gNativeMenuChoice = (gNativeMenuChoice + 1) % 6;
                pressed = 0;
            }
        }
        if (gNativePreview || ((raw & L_BUTTON) && (pressed & DPAD_ANY) &&
            (player->state == FLD_STATE_GROUND || player->state == FLD_STATE_CLIMB))) {
            NativePreviewInput(pressed);
        } else if (player->state == FLD_STATE_GROUND && (raw & R_BUTTON) && (pressed & DPAD_ANY) &&
            !(raw & (L_BUTTON | A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON))) {
            u16 direction = raw & DPAD_ANY;
            if (direction & DPAD_UP) direction &= ~DPAD_DOWN;
            if (direction & DPAD_LEFT) direction &= ~DPAD_RIGHT;
            gFieldState->actor.speed = 0;
            gFieldState->actor.angle = (direction & DPAD_UP) ?
                ((direction & DPAD_LEFT) ? 211 : (direction & DPAD_RIGHT) ? 45 : 0) :
                (direction & DPAD_DOWN) ? ((direction & DPAD_LEFT) ? 173 : (direction & DPAD_RIGHT) ? 83 : 128) :
                (direction & DPAD_LEFT) ? 192 : 64;
            gNativeCommands++;
        } else if (player->state == FLD_STATE_CLIMB && (pressed & DPAD_ANY) && gNativeMoveLeft) {
            gNativeDirection = raw & DPAD_ANY;
            gNativeMoveLeft--;
            gNativeCommands++;
            gNativeBusy = 4;
            sFrames = 0;
        } else if ((raw & (START_BUTTON | SELECT_BUTTON)) == (START_BUTTON | SELECT_BUTTON) &&
            (pressed & (START_BUTTON | SELECT_BUTTON))) {
            NativeWriteSuspend();
        } else if ((raw & L_BUTTON) && (pressed & A_BUTTON)) {
            if (gNativeDeck.stocked < 3) FieldDeckStock(&gNativeDeck);
        } else if ((raw & L_BUTTON) && (pressed & B_BUTTON)) {
            FieldDeckCancelStock(&gNativeDeck);
        } else if (((raw & R_BUTTON) && (pressed & B_BUTTON)) ||
            ((raw & B_BUTTON) && (pressed & R_BUTTON))) {
            int selected = FieldDeckHand(&gNativeDeck, gNativeDeck.selected);
            if (selected >= 0 && !gNativeDeck.stocked &&
                (gNativeDeck.kind[selected] == FIELD_CARD_FIRE ||
                 ((NativeHero(gNativeParty) == FIELD_DONALD || NativeHero(gNativeParty) == FIELD_CLOUD) && gNativeDeck.kind[selected] == FIELD_CARD_KEY))) NativeCycleFireTarget();
            else if (selected >= 0 && !gNativeDeck.stocked &&
                gNativeDeck.kind[selected] == FIELD_CARD_CURE) NativeCycleCureTarget();
        } else if ((raw & L_BUTTON) && (pressed & SELECT_BUTTON) && player->state == FLD_STATE_GROUND) {
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
            if ((pressed & A_BUTTON) && gNativeDeck.stocked == 3) {
                NativeSleight();
            } else if (pressed & A_BUTTON) {
                int card = FieldDeckPlay(&gNativeDeck);
                if (card >= 0) {
                    u8 kind = gNativeDeck.kind[card];
                    NativePartyPose(gNativeParty);
                    sPlayedValue = gNativeDeck.value[card];
                    if (kind == FIELD_CARD_CURE) {
                        NativeCure();
                    } else if (kind == FIELD_CARD_GUARD) {
                        gNativeGuard = NativeHero(gNativeParty) == FIELD_GOOFY ? 2 : 1;
                    } else if (kind == FIELD_CARD_FIRE) {
                        NativeFire();
                    } else if (NativeHero(gNativeParty) == FIELD_DONALD) {
                        NativeFire();
                    } else if (NativeHero(gNativeParty) == FIELD_GOOFY) {
                        NativeShieldSpin();
                    } else if (NativeHero(gNativeParty) == FIELD_CLOUD) {
                        NativeCloudSlash();
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
            if (gNativeDirection && gNativeMoveLeft) {
                gNativeMoveLeft--;
                /* A committed moving jump starts with native running speed;
                 * otherwise short tactical steps leave every jump at rest. */
                if (player->state == FLD_STATE_GROUND) gFieldState->actor.speed = 0x266;
            } else gNativeDirection = 0;
            sStartX = gFieldState->actor.fieldPosition.x;
            sStartY = gFieldState->actor.fieldPosition.y;
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
    /* A native stair attachment must finish its vertical segment before
     * another command can replace the controller's target. */
    if (gNativeBusy == 2 && !sAttack && player->state == FLD_STATE_CLIMB) {
        gNativeBusy = 4;
        gNativeDirection = 0;
        gFieldState->flags &= ~FIELD_FLAG_PLAYER_JUMPING;
        sFrames = 0;
    }
    if (gNativeBusy == 1 && player->state == FLD_STATE_CLIMB) {
        gNativeBusy = 4;
        sFrames = 0;
    }
    if (gNativeBusy == 4) {
        if (!sFrames) held = gNativeDirection;
        else if (player->state == FLD_STATE_GROUND ||
            (player->state == FLD_STATE_CLIMB &&
                NativeAbs(player->targetZ - gFieldState->actor.fieldPosition.z) <= 48)) {
            gNativeBusy = 0;
            gNativeDirection = 0;
            gFieldState->actor.speed = 0;
        }
    }
    if (gNativeBusy == 3) held = NativeRouteWalk();
    if (gNativeBusy == 5) held = NativeRouteClimb(player);
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
        /* One press commits a full-height native ascent. Movement is bounded
         * in world space, so falling to a lower floor cannot buy extra travel. */
        if (player->state == FLD_STATE_GROUND || player->state == FLD_STATE_JUMP_START ||
            player->state == FLD_STATE_JUMP_RISE || player->state == FLD_STATE_FALL) {
            dx = NativeAbs(gFieldState->actor.fieldPosition.x - sStartX);
            dy = NativeAbs(gFieldState->actor.fieldPosition.y - sStartY);
            if (dx + dy * 2 < (32 << 8)) held = gNativeDirection;
            else gFieldState->actor.speed = 0;
            if (player->state == FLD_STATE_JUMP_START || player->state == FLD_STATE_JUMP_RISE)
                held |= B_BUTTON;
        }
        if (player->state == FLD_STATE_LEDGE_CATCH || player->state == FLD_STATE_LEDGE_HANG) {
            held = raw & DPAD_ANY;
            edge = pressed & (B_BUTTON | DPAD_ANY);
            /* Dropping abandons the jump's prior horizontal command, or
             * the following fall can drive straight back into this ledge. */
            if ((edge & (B_BUTTON | DPAD_DOWN)) ||
                (gFieldState->actor.angle == 211 && (edge & DPAD_RIGHT)) ||
                (gFieldState->actor.angle == 45 && (edge & DPAD_LEFT))) gNativeDirection = 0;
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
    gNativeClimbing = player->state == FLD_STATE_CLIMB || player->state == FLD_STATE_CLIMB_OVER;
    NativeSyncHealth();
    if (gNativeEnemyFrames) {
        gMapRoomState->flags &= ~(ROOM_FLAG_START_BATTLE | ROOM_FLAG_ENEMY_STRUCK);
        NativeEnemies();
        gNativeEnemyFrames--;
        if (!gNativeEnemyFrames) {
            if (!gNativePartyHealth.hp[gNativeParty] && !gNativeResult) NativePartySelect();
            gNativeMoveLeft = 3;
            gNativeActionLeft = 1;
            sPartyMove[0] = sPartyMove[1] = sPartyMove[2] = 3;
            sPartyAction[0] = sPartyAction[1] = sPartyAction[2] = 1;
            {u8 hero;for (hero = 0; hero < FIELD_HEROES; hero++) {
                gNativeRoster.heroMove[hero] = 3;gNativeRoster.heroAction[hero] = 1;
            }}
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
    if (!gNativeResult && !gNativeAssembly && gNativeRoster.phase == FIELD_BATTLE &&
        GetMapFloorRoom(gMapFloorState.room)->enemiesLeft == 0)
        gNativeProgressReward = FieldRosterClear(&gNativeRoster, gMapFloorState.room == 7 || (gNativeFloor == 0 && gMapFloorState.room == 9)) != FIELD_REWARD_NONE;
    if (gNativeProgressReward) NativeRecruitInit();
    NativeRosterSync();
    NativePreviewThreats();
    NativePartyDraw();
    NativeAssemblyDraw();
    NativeIntentDraw();
    NativeCardIntentDraw();
    NativeSleightIntentDraw();
    NativePreviewDraw();
    NativeHud();
    if (gNativeBusy) sFrames++;
    if (gNativeBusy == 2 && sFrames >= 48 &&
        !(gFieldState->flags & FIELD_FLAG_PLAYER_JUMPING)) {
        gNativeBusy = 0;
        sAttack = 0;
    }
    /* The native actor can still detect the doorway while standing at the
     * final exit. A terminal run must not keep advancing its floor counter. */
    if (gNativeResult || gNativeReward || gNativeProgressReward || gNativeAssembly) gFieldState->flags &= ~FIELD_FLAG_EXIT_ROOM;
    if (gFieldState->flags & FIELD_FLAG_EXIT_ROOM) {
        if (gMapFloorState.room == 7 && gMapRoomState->doorRoom == TAC_WORLD_EXIT) {
            if (GetMapFloorRoom(7)->enemiesLeft == 0) {
                gNativeFloor++;
                if (gNativeFloor >= 3) {
                    gNativeResult = 2;
                    gFieldState->flags &= ~FIELD_FLAG_EXIT_ROOM;
                }
                else {
                    NativeRosterSync();
                    FieldRosterRest(&gNativeRoster);
                    NativeRosterHealth();
                    gNativeGuard = 0;
                    NativeCarryTurn();
                    NativeBuildWorld();
                    ModeRequest(&sNativeMode, 0);
                }
            } else gFieldState->flags &= ~FIELD_FLAG_EXIT_ROOM;
        } else if (gMapRoomState->doorRoom < TAC_WORLD_ROOMS) {
            NativeCaptureEncounter();
            NativeCarryTurn();
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
    FieldPartyInit(&gNativePartyHealth);
        FieldRosterInit(&gNativeRoster);
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
