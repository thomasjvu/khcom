# Development patch 0.35

Apply `build/release/kh-tactics-0.35-compact-commands-dev.bps` to the original
US ROM SHA1 `10729bd884f8fdca7a310b6d606c52e46657aa48`. The 95877-byte patch
reproduces the 32MiB built ROM byte-identically. ROM SHA256:
`1667d8885c5db2c5e09592554b736ad27ffe955180ef597a557de34a162c7f9f`.

Commands has three rows in two columns and a party-phase strip underneath.
It uses 40 pixels at the top and16 at the bottom, leaving104 of160 battlefield
pixels unobscured. Idle movement keeps the18-pixel header. Detailed menus
are contextual. Party actions can be taken in any order; enemies act together
when End Turn is confirmed. Spent/KO members leave the ready strip.

Donald, Goofy and Cloud action animations play once; idle and walking loop.
26 input-only native assembly/menu/preview/save/reset checks and19 disclosed
combat-fixture checks pass. Six additional disclosed budget/KO/phase fixtures
verify native rendered turn-strip pixels, for51 scoped native checks. Host rules/save/deck/party/roster suites,3000 world
graphs,1000 route-resource graph oracles and Rally asset encoding pass.
Graph coverage is not universal native physical reachability evidence.

Exact-ROM replay results: two consecutive runs PASS327681, first stable
victory129417 with Sora76 HP, native retry129429 seed2658846982, second
stable victory327681 with Sora63 HP. Both require actual Cloud recruitment/
deployment, exact suspend/reset and composed descent. The independent
all-room/nine-chest run passes295578: room masks4095 each, nine chests,
Sora80 HP stable120 frames,38 kills,1894 movement commands. Both emulator
processes exited0, and ROM/driver/log hashes are recorded in their evidence
metadata. These are finite route/seed proofs, not universal reachability.

This remains a development patch; final combat, content, balance and visual
review remain open in RELEASE-AUDIT.md. Earlier failed campaigns are retained.
