/* Thumb tail veneers; four-argument calls preserve r3 via ip. */
.section .text
.thumb
.balign 4
.global TacFar___divsi3
.thumb_func
TacFar___divsi3:
 ldr r3, 1f
 bx r3
.balign 4
1: .word __divsi3 + 1
.balign 4
.global TacFar___udivsi3
.thumb_func
TacFar___udivsi3:
 ldr r3, 1f
 bx r3
.balign 4
1: .word __udivsi3 + 1
.balign 4
.global TacFar___modsi3
.thumb_func
TacFar___modsi3:
 ldr r3, 1f
 bx r3
.balign 4
1: .word __modsi3 + 1
.balign 4
.global TacFar___umodsi3
.thumb_func
TacFar___umodsi3:
 ldr r3, 1f
 bx r3
.balign 4
1: .word __umodsi3 + 1
.balign 4
.global TacFar_memcpy
.thumb_func
TacFar_memcpy:
 ldr r3, 1f
 bx r3
.balign 4
1: .word memcpy + 1
.balign 4
.global TacFar_m4aSongNumStart
.thumb_func
TacFar_m4aSongNumStart:
 ldr r3, 1f
 bx r3
.balign 4
1: .word m4aSongNumStart + 1
.balign 4
.global TacFar_m4aSoundInit
.thumb_func
TacFar_m4aSoundInit:
 ldr r3, 1f
 bx r3
.balign 4
1: .word m4aSoundInit + 1
.balign 4
.global TacFar_m4aSoundVSyncOn
.thumb_func
TacFar_m4aSoundVSyncOn:
 ldr r3, 1f
 bx r3
.balign 4
1: .word m4aSoundVSyncOn + 1
.balign 4
.global TacFar_m4aSoundVSync
.thumb_func
TacFar_m4aSoundVSync:
 ldr r3, 1f
 bx r3
.balign 4
1: .word m4aSoundVSync + 1
.balign 4
.global TacFar_m4aSoundMain
.thumb_func
TacFar_m4aSoundMain:
 ldr r3, 1f
 bx r3
.balign 4
1: .word m4aSoundMain + 1

.balign 4
.global TacFar_InitSystem
.thumb_func
TacFar_InitSystem:
 ldr r3, 1f
 bx r3
.balign 4
1: .word InitSystem + 1


.balign 4
.global TacFar_ResetGameState
.thumb_func
TacFar_ResetGameState:
 ldr r3, 1f
 bx r3
.balign 4
1: .word ResetGameState + 1


.balign 4
.global TacFar_SetupSoraNewGame
.thumb_func
TacFar_SetupSoraNewGame:
 ldr r3, 1f
 bx r3
.balign 4
1: .word SetupSoraNewGame + 1


.balign 4
.global TacFar_EnterFloorWorld
.thumb_func
TacFar_EnterFloorWorld:
 ldr r3, 1f
 bx r3
.balign 4
1: .word EnterFloorWorld + 1


.balign 4
.global TacFar_ModeRequest
.thumb_func
TacFar_ModeRequest:
 ldr r3, 1f
 bx r3
.balign 4
1: .word ModeRequest + 1


.balign 4
.global TacFar_EnableVBlankIntr
.thumb_func
TacFar_EnableVBlankIntr:
 ldr r3, 1f
 bx r3
.balign 4
1: .word EnableVBlankIntr + 1


.balign 4
.global TacFar_UpdateKeyState
.thumb_func
TacFar_UpdateKeyState:
 ldr r3, 1f
 bx r3
.balign 4
1: .word UpdateKeyState + 1


.balign 4
.global TacFar_ModeUpdate
.thumb_func
TacFar_ModeUpdate:
 ldr r3, 1f
 bx r3
.balign 4
1: .word ModeUpdate + 1


.balign 4
.global TacFar_ApplyIntrCallbacks
.thumb_func
TacFar_ApplyIntrCallbacks:
 ldr r3, 1f
 bx r3
.balign 4
1: .word ApplyIntrCallbacks + 1


.balign 4
.global TacFar_VBlankIntrWait
.thumb_func
TacFar_VBlankIntrWait:
 ldr r3, 1f
 bx r3
.balign 4
1: .word VBlankIntrWait + 1


.balign 4
.global TacFar_Mode_MapFld_0
.thumb_func
TacFar_Mode_MapFld_0:
 ldr r3, 1f
 bx r3
.balign 4
1: .word Mode_MapFld_0 + 1


.balign 4
.global TacFar_MapEnmCheckAttacked
.thumb_func
TacFar_MapEnmCheckAttacked:
 ldr r3, 1f
 bx r3
.balign 4
1: .word MapEnmCheckAttacked + 1


.balign 4
.global TacFar_GetMapFloorRoom
.thumb_func
TacFar_GetMapFloorRoom:
 ldr r3, 1f
 bx r3
.balign 4
1: .word GetMapFloorRoom + 1


.balign 4
.global TacFar_TaskKill
.thumb_func
TacFar_TaskKill:
 ldr r3, 1f
 bx r3
.balign 4
1: .word TaskKill + 1


.balign 4
.global TacFar_MapEnmUpdateAnim
.thumb_func
TacFar_MapEnmUpdateAnim:
 ldr r3, 1f
 bx r3
.balign 4
1: .word MapEnmUpdateAnim + 1


.balign 4
.global TacFar_GetKeysPressed
.thumb_func
TacFar_GetKeysPressed:
 ldr r3, 1f
 bx r3
.balign 4
1: .word GetKeysPressed + 1


.balign 4
.global TacFar_UpdateMapField
.thumb_func
TacFar_UpdateMapField:
 ldr r3, 1f
 bx r3
.balign 4
1: .word UpdateMapField + 1


.balign 4
.global TacFar_ColliderSetDisabled
.thumb_func
TacFar_ColliderSetDisabled:
 ldr r3, 1f
 bx r3
.balign 4
1: .word ColliderSetDisabled + 1


.balign 4
.global TacFar_MapEnmUpdateSpawner
.thumb_func
TacFar_MapEnmUpdateSpawner:
 ldr r3, 1f
 bx r3
.balign 4
1: .word MapEnmUpdateSpawner + 1


.balign 4
.global TacFar_ColliderUpdateAll
.thumb_func
TacFar_ColliderUpdateAll:
 ldr r3, 1f
 bx r3
.balign 4
1: .word ColliderUpdateAll + 1


.balign 4
.global TacFar_DrawMapField
.thumb_func
TacFar_DrawMapField:
 ldr r3, 1f
 bx r3
.balign 4
1: .word DrawMapField + 1


.balign 4
.global TacFar_CreateMapRoom
.thumb_func
TacFar_CreateMapRoom:
 ldr r3, 1f
 bx r3
.balign 4
1: .word CreateMapRoom + 1


.balign 4
.global TacFar_SetCurrentMapRoom
.thumb_func
TacFar_SetCurrentMapRoom:
 ldr r3, 1f
 bx r3
.balign 4
1: .word SetCurrentMapRoom + 1

.balign 4
.global TacFar_DebugTextInit
.thumb_func
TacFar_DebugTextInit:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DebugTextInit + 1


.balign 4
.global TacFar_DebugTextLoadPalette
.thumb_func
TacFar_DebugTextLoadPalette:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DebugTextLoadPalette + 1


.balign 4
.global TacFar_DebugTextPrint
.thumb_func
TacFar_DebugTextPrint:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DebugTextPrint + 1


.balign 4
.global TacFar_DebugTextDraw
.thumb_func
TacFar_DebugTextDraw:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DebugTextDraw + 1


.balign 4
.global TacFar_DebugTextClear
.thumb_func
TacFar_DebugTextClear:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DebugTextClear + 1


.balign 4
.global TacFar_DebugTextFree
.thumb_func
TacFar_DebugTextFree:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DebugTextFree + 1


.balign 4
.global TacFar_Mode_MapFld_2
.thumb_func
TacFar_Mode_MapFld_2:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word Mode_MapFld_2 + 1


.balign 4
.global TacFar_MapEnmSetAnim
.thumb_func
TacFar_MapEnmSetAnim:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word MapEnmSetAnim + 1

.balign 4
.global TacFar_SeedRandom
.thumb_func
TacFar_SeedRandom:
 ldr r3, 1f
 bx r3
.balign 4
1: .word SeedRandom + 1


.balign 4
.global TacFar_MapEnmSetupArgs
.thumb_func
TacFar_MapEnmSetupArgs:
 ldr r3, 1f
 bx r3
.balign 4
1: .word MapEnmSetupArgs + 1


.balign 4
.global TacFar_TaskCreate
.thumb_func
TacFar_TaskCreate:
 ldr r3, 1f
 bx r3
.balign 4
1: .word TaskCreate + 1

.balign 4
.global TacFar_MapCellIsFreeOfType
.thumb_func
TacFar_MapCellIsFreeOfType:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word MapCellIsFreeOfType + 1


.balign 4
.global TacFar_FldPosPlaceAtCell
.thumb_func
TacFar_FldPosPlaceAtCell:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word FldPosPlaceAtCell + 1

.balign 4
.global TacFar__call_via_r3
.thumb_func
TacFar__call_via_r3:
 bx r3

.balign 4
.global TacFar__call_via_r1
.thumb_func
TacFar__call_via_r1:
 bx r1

.balign 4
.global TacFar_MapReserveArea
.thumb_func
TacFar_MapReserveArea:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word MapReserveArea + 1

.balign 4
.global TacFar_AllocObjTiles
.thumb_func
TacFar_AllocObjTiles:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word AllocObjTiles + 1

.balign 4
.global TacFar_LoadObjTiles
.thumb_func
TacFar_LoadObjTiles:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word LoadObjTiles + 1

.balign 4
.global TacFar_LoadObjPalette
.thumb_func
TacFar_LoadObjPalette:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word LoadObjPalette + 1

.balign 4
.global TacFar_AnimInit
.thumb_func
TacFar_AnimInit:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word AnimInit + 1

.balign 4
.global TacFar_AnimStart
.thumb_func
TacFar_AnimStart:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word AnimStart + 1

.balign 4
.global TacFar_AnimUpdate
.thumb_func
TacFar_AnimUpdate:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word AnimUpdate + 1

.balign 4
.global TacFar_AnimGetGfx
.thumb_func
TacFar_AnimGetGfx:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word AnimGetGfx + 1

.balign 4
.global TacFar_DrawSprite
.thumb_func
TacFar_DrawSprite:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word DrawSprite + 1

.balign 4
.global TacFar_ReleaseObjTiles
.thumb_func
TacFar_ReleaseObjTiles:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word ReleaseObjTiles + 1

.balign 4
.global TacFar_ReleaseObjPalette
.thumb_func
TacFar_ReleaseObjPalette:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word ReleaseObjPalette + 1

.balign 4
.global TacFar_MapCellAtPos
.thumb_func
TacFar_MapCellAtPos:
 ldr r3, 1f
 bx r3
.balign 4
1: .word MapCellAtPos + 1

.balign 4
.global TacFar_GetFldPosFloor
.thumb_func
TacFar_GetFldPosFloor:
 ldr r3, 1f
 bx r3
.balign 4
1: .word GetFldPosFloor + 1

.balign 4
.global TacFar_GetBgScreenBase
.thumb_func
TacFar_GetBgScreenBase:
 ldr r3, 1f
 bx r3
.balign 4
1: .word GetBgScreenBase + 1

.balign 4
.global TacFar_ColliderSetPosition
.thumb_func
TacFar_ColliderSetPosition:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word ColliderSetPosition + 1

.balign 4
.global TacFar_AnimChangeWithDef
.thumb_func
TacFar_AnimChangeWithDef:
 push {r3}
 ldr r3, 1f
 mov ip, r3
 pop {r3}
 bx ip
.balign 4
1: .word AnimChangeWithDef + 1

.balign 4
.global TacFar_GetBgCharBase
.thumb_func
TacFar_GetBgCharBase:
 ldr r3, 1f
 bx r3
.balign 4
1: .word GetBgCharBase + 1
