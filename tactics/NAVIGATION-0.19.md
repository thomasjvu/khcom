# KH Tactics 0.19 navigation development patch

Apply `kh-tactics-0.19-navigation-dev.bps` to the original US Chain of Memories ROM (SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48`). The BPS applies byte-for-byte to the tested 32 MiB native ROM. This is an unfinished development build, using suspend format 10.

Includes original-engine procedural Traverse Town, Agrabah and Castle rooms; individually controllable Sora, Donald and Goofy; distinct character attacks; round-start party cards; fight/recruit/deploy Cloud; personal power/sleight rewards; world rest for deployed and benched recruits; Guard Armor, sorcerer Jafar and humanoid Marluxia encounters; original sprites, cards, doors, ledges and chests.

New navigation features include continuous actor occupancy checks along movement segments, height-interpolated enemy terrain probes, affordable attached-stair direction markers, and R + D-pad to face without movement or turn cost. Cancel a walking preview with B before facing. L + D-pad previews walking; A confirms; Select changes active hero. Setup uses L/R for the slot and Up/Down for unlocked companions, then A/Start confirms. Other controls are in `tactics/PLAY.md`.

The exact ROM completes a fresh controller-only replay through all twelve rooms in each world, with backtracking, all nine chest opens, Cloud recruitment/deployment, and verified suspend/resume. Victory frame 252406 ends with Sora HP80; stable terminal PASS is 252526 (32 kills, 2014 movement commands). Donald and Goofy each pass 33 controller-only assembly/facing state and resource checks. The manifest records ROM/patch/evidence hashes.

Remaining work includes combined walk/climb/jump routes and previews, physical generation guarantees across seeds, broader content and recruits, companion balance, and hardware verification. One complete seed does not prove all procedural layouts. Aerith is not playable; Tifa assets have not been located. The full polish goal remains active.
