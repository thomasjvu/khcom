# KH Tactics 0.27 compact-HUD development patch

Apply `kh-tactics-0.27-compact-hud-dev.bps` to the original US ROM with SHA-1
`10729bd884f8fdca7a310b6d606c52e46657aa48`. The 77,466-byte BPS reproduces
the tested 32 MiB ROM byte-for-byte. Native/replay source: `6b005110e`.
Save format12; reserved native EWRAM8140/8192 bytes. Exact hashes and scoped
evidence are in `build/release/compact-hud-0.27-manifest.json`.

The normal field HUD uses two rows, including while walking. Its turn strip
shows available deployed heroes, marks the controlled hero with X, then
shows the enemy phase. SOR/DON/GOO/CLO/RAL identify Sora/Donald/Goofy/Cloud/
Rally. Exhausted or KO heroes leave the ready list. Enemies act together
after the freely ordered party phase. Commands, targeting and rewards open
contextual panels. Skills shows hand/draw/discard counts, card value and
stock; Reload shows recoverable cards; Guard explains protection; ledge
commands offer Climb/Drop. Rewards use a selectable list with a recipient.
Original scene, actor and card art remains visible.

Suspend uses the original wait-state-aware SRAM library and RAM-resident
verifier. Alternating slots commit their signature last. State rejection
and SRAM verification failure remain SAVE FAILED; diagnostics distinguish
the stages. No save format or persistent RAM expansion.

On this exact ROM, 14 input-only party/movement checks and 12 alternating-
slot save/reset/fallback checks pass. Newest-slot CRC corruption is an
explicit fixture. Both native SRAM dumps pass production host decoding.
The selected-party screenshot was inspected. Host sanitizer tests pass
30 deterministic runs, 3000 world graphs, 1000 route-resource oracles and
deck/save/party/enemy/roster invariants. Build/header/capacity checks pass.

A fresh-SRAM input-only campaign completes all three worlds: Cloud recruited
9383/deployed10733; suspend10747/exact resume11077; composed descent1214;
world transitions34034/72343; victory97411 with Sora80HP, stable through
PASS97531 (14 kills, 699 move attempts). The 120000-frame bound is unchanged.
Replay movement inputs now wait for actual native sampling/update completion,
so video frames cannot release input during expensive route calculations.

This proves one main-route seed. The all-room/chest run failed its300000
frame bound in Agrabah room5 after six chests and optional backtracking.
The consecutive-run test completed the first campaign and native seed retry,
but ended at240001 in the second seed's Castle entrance room0. Both second
Cloud and save/descent requirements passed; second victory did not. These
are wider failures, not two-run or all-room completion proof.
Earlier full-route failures and the original false SRAM failure
are retained. The explicit recorded Agrabah fixture reaches its exit using
native inputs; it is not fresh-campaign evidence.

This remains a development patch. Wider procedural branches/seeds, physical
cartridge checks, content/balance, further recruits, Rally action/air/card/
encounter assets and final interface polish remain unfinished. The full
ROM-hack goal stays active.
