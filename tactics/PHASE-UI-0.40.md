# Phase UI 0.40 development patch

The ready-party/enemy strip remains visible during detailed Attack, Jump, Party, Reload and End Turn confirmations. It occupies a spare row, preserving18+16px idle,40+16px Commands and56+16px detailed windows. Sword commands support eight directions; each committed sword strike resets the original field hit latch once. Native launcher previews model pad pull/rise/fall and display landing markers.

Patch: `build/release/kh-tactics-0.40-phase-ui-dev.bps` (98,597 bytes). Apply to US base SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48` using [build instructions](BUILD.md). Applied ROM SHA256 `a01f555dff42d1146ffa3e7ca044d3c06d4c6305450790342a67726a6eb11c33` matches the build byte for byte.

223 exact-ROM scoped native checks pass: Sora/Rally command aiming37, End Turn13, original jump-pad confirmation9, Party17, Donald healing/revival/save17, Goofy spin/Guard26, Cloud sword9, Guard Armor40, Jafar25 and Marluxia30. Disclosed card/HP/target/encounter/Cloud-unlock fixtures support focused mechanics checks; setup/menu/actions/save/reset use native input. All bounded processes exit0; ROM/driver/log hashes verified. Boss and companion screenshots inspected, including original charged poses and card artwork.

The current all-room/chest three-seed sweep is live. Its first campaign reaches stable victory301053 with all36 rooms/nine chests, native Cloud fight/recruit/deploy, suspend/reset and composed descent, then verifies native retry. This is partial aggregate coverage, not a completed three-run result. Historical failures are retained; final physical reachability, balance/content/visual audit and release readiness remain open. `build/release/phase-ui-0.40-manifest.json` records exact artifact and evidence scope.
