# KH Tactics 0.18 party roguelike development patch

Apply `kh-tactics-0.18-party-roguelike-dev.bps` to the original US Chain of Memories ROM (SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48`). Applying the BPS reproduces the tested 32 MiB ROM byte-for-byte. This is an unfinished development build.

The original 2.5D engine generates Traverse Town, Agrabah and Castle rooms. Control Sora, Donald or Goofy independently. Their attacks differ; individual positions, HP, movement and actions persist. Original card art appears in combat and round setup. Fight Cloud on the Traverse Town 1–8–9–4 optional route, then choose his recruit card or a personal upgrade. Recruited Cloud can replace either companion. Every second unique room clear and world boss offers a personal power/sleight upgrade. Complete a world to revive, heal and refresh the recruited roster, including the bench.

Setup: L/R selects a slot, Up/Down changes an unlocked companion, A/Start confirms. Field: D-pad moves; L+D-pad previews walking; Select switches character. L/R selects cards; A plays; R+B cycles Fire/Cure/Cloud/Donald targets. L+A stocks cards, A with three stocked plays a sleight, L+B cancels stock. B jumps; L+R reloads; Start ends the turn. Start+Select suspends while idle or during a pending personal reward; boot resumes. Select after victory/defeat retries on a new seed.

Suspend format 10 stores roster, deployed and benched resources and upgrades. Recorded format-8 and format-9 saves migrate; fresh SRAM is the tested release route. Do not treat unsupported intermediate development save layouts as release-compatible.

The exact ROM passes a fresh input-only Cloud recruit/deploy and suspend/reset run through all three worlds (victory frame 93534, stable PASS 93654, Sora HP80). Both later world entries have full deployed HP. The manifest records hashes and verification limits. Height navigation previews, physical generation guarantees, broader seeds/optional chests, additional recruits, companion balance and hardware verification remain unfinished. Aerith is not playable. The full polish goal remains active.
