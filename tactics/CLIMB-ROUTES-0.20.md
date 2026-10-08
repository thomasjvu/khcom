# KH Tactics 0.20 climb-route development patch

Apply `kh-tactics-0.20-climb-routes-dev.bps` to the original US Chain of Memories
ROM (SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48`). The BPS round-trip
reproduces the tested 32 MiB native ROM byte-for-byte. This is an unfinished
development build using suspend format 11.

Includes procedural Traverse Town, Agrabah and Castle rooms using original
sprites, scenery, cards, doors, stairs and chests. Sora, Donald and Goofy have
separate health/turn budgets and distinct attacks. Round-start character cards
assemble the party. An optional Cloud battle offers recruitment or a personal
upgrade; recruited Cloud can replace either companion. World exits rest the
unlocked roster. Original Guard Armor, sorcerer Jafar and humanoid Marluxia
artwork supports custom tactical boss encounters. Personal power/sleight
rewards and dual-slot suspend saves persist run progress and per-hero facing.

Walking previews use movement/action resource states, native collision and
continuous actor occupancy. Unchanged previews are cached; A always revalidates
geometry. While attached to stairs, L+Up/Down opens a vertical route; further
Up/Down selects up to three segments within the movement budget. Reachable
diamonds show affordable heights, original card digits show total cost, and A
executes the original climb/descent animations. B cancels without dropping.
Floor markers include the native landing offset. Blocked/unaffordable routes
show OUT OF REACH and ROUTE X. R+D-pad faces without movement cost; Select
changes the active hero after landing. See `tactics/PLAY.md` for all controls.

The exact ROM passes 86 native stair checks: 25 multi-segment
climb/descent/save/reset checks, 32 single-step regressions, 15 party/enemy
occupancy checks and 14 top-platform landing/cost checks. Stair approach and
occupancy use explicit position fixtures; commands and movement use native
input. A separate fresh-SRAM input-only replay fights, recruits and deploys
Cloud, suspends at frame 14442, verifies reset at 14772 and completes all three
worlds. Victory is 92563 with Sora HP75; stable PASS is 92683 (15 kills,
649 movement commands). No RAM writes or forced exits occur in that full run.
The release manifest ties the ROM, patch, drivers and logs to their hashes.

Walking, climbing and jumping still have separate route controls. Combined
height routes, physical procedural generation guarantees across seeds, further
recruits/content, companion balance, richer loadouts and hardware verification
remain open. This complete-run replay covers one seed, not every optional room
or chest. Earlier 0.19 all-room/nine-chest coverage belongs to its older ROM.
Aerith is not playable; Tifa assets have not been located. The full polish goal
remains active.
