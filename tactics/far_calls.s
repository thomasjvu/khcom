/* All callees are Thumb functions with at most three register arguments. */
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
