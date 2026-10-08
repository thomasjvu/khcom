# KH Tactics 0.22 party-healing development patch

Apply `kh-tactics-0.22-party-healing-dev.bps` to the original US ROM with SHA-1
`10729bd884f8fdca7a310b6d606c52e46657aa48`. The patch round-trip reproduces
the tested 32 MiB native ROM byte-for-byte. Save format 11; native EWRAM remains
8180/8192 bytes. Native source: `4025e129b1daa73b44b5e79ce04badbaecffdfde`.

Retains procedural original-art Traverse Town, Agrabah and Castle rooms,
independent Sora/Donald/Goofy control, Cloud recruitment/deployment, original
cards, personal rewards, bosses and attached stair descent-and-walking routes.
Party setup now shows the highlighted character's own health and turn budgets,
role, power bonus and owned matching-sleight bonuses. Donald's eight-point
healing specialty now applies to his party-healing sleights as well as single
Cure. His owned Cure recipe enhancement stacks separately. Every recipient's
recovery caps at missing health; knocked-out companions can be revived.
Original digits preview the exact recovery before resolving the action.

On this exact ROM, 59 focused native checks pass: 17 Donald healing,
revival, caps, enhancement, exhaustion and suspend/reset checks; 16 Sora Curaga
and melee recipe regressions; and 26 rendered party-loadout checks across four
heroes. These use explicit health/card/upgrade/budget/recruitment or enemy
position fixtures, with native controls for deployment/stocking/execution/save.
Party-display checks inspect actual uploaded VRAM rather than staging buffers.
The host sanitizer suite and GBA build/header/ROM limits also pass.

A separate fresh-SRAM input-only campaign verifies composed stair descent and
walking at frame 1308, fights and recruits Cloud at 13332, deploys him at 15269,
suspends at 15283 and verifies reset at 15613. Three-world victory occurs at
102782 with Sora HP70; stable PASS 102902, 19 kills and 720 movement commands.
It never writes emulated memory or forces exits. The local manifest records
exact ROM, patch, driver and log hashes. Historical 0.21 stair fixtures and
0.19 all-room/nine-chest evidence retain their older exact ROM hashes.

This is unfinished. Combined standing-surface walk/climb/jump routing,
bounded physical procedural-room validation across seeds, further recruits
and content, richer loadouts, companion balance and hardware verification
remain open. Current full-run evidence covers one seed and does not visit every
optional branch or chest. Aerith recruitment is unimplemented; Tifa assets have
not been located. The full polish goal remains active.
