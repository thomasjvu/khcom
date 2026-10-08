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

Exact-ROM two-run and all-room/nine-chest replays are pending. Earlier0.34
complete-run evidence does not prove this build's completion. Both0.34
all-room policies ended in native defeat. This remains a development patch;
full campaign verification and final game polish remain unfinished.
