# Development patch 0.36

Apply `build/release/kh-tactics-0.36-command-columns-dev.bps` to the original
US ROM SHA1 `10729bd884f8fdca7a310b6d606c52e46657aa48`. The95902-byte patch
reproduces the32MiB built ROM byte-identically. ROM SHA256:
`27acd304ee14ab79fd4e1892d0cb5bb807b2adde0dd093245d2c8876c7b29950`.

Commands uses a40-pixel top panel and16-pixel bottom strip. Left/Right switches
columns on the same row; Up/Down cycles six choices. Ready living party
members precede enemies; party actions can be freely ordered. All other0.35
native game features remain, including single-play companion action poses.

229 scoped native checks pass:40 UI,17 Donald healing,26 Goofy spin/Guard/menu,
89 Guard Armor/Jafar/Marluxia and57 reward checks. Rewards include capped/owned
upgrade availability, persistent Donald power and actual magic damage, and
both Cloud summon and ordinary boss-power choices through save/reset. Explicit combat/phase/transition fixtures are
recorded separately from full input-only campaigns. Native all-room replay
PASS295578 visits all36 rooms, opens nine chests, fights/recruits/deploys Cloud,
verifies exact suspend/reset and composed descent, and ends with Sora80 HP
stable120 frames. Process exited0; ROM/driver/log hashes verified.

Three-run replay first two seeds complete129417/327681, but the third stable
victory444879 failed the required composed descent count. This failure is
retained. Revised replay seeks suitable stairs in later rooms/worlds and is
pending; no three-run success is claimed. Final polish audit remains open.
