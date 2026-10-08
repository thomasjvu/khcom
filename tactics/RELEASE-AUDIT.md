# Current release audit

Current state-art ROM, SHA256
`5f1a98008c3f8e9e6d27f12466fe4744de8035a34743cd7f543d9563bd3f846b`.
Latest packaged development build is0.34. It passes49 native checks and
two consecutive campaigns288347; its all-room attempt ended in defeat.
A companion-recovery all-room replay is live. Earlier0.33 all-room coverage
remains scoped to its own hash64bb02ac04b1e662fc839bbd5c4c3479a626f856cd02f28e752658364c661505.
This audit concerns the requested complete game, not merely a successful build.
Historical entries in PLAN.md describe earlier versions; current evidence wins.

| Requested behavior | Current evidence | Remaining work |
| --- | --- | --- |
| Fork and native GBA ROM | Fork thomasjvu/khcom, draft PR1, tactics native target, byte-identical BPS apply | Final release packaging and reproducible instructions |
| Original 2.5D scenes, actors, cards | Native field renderer/collision/animation; inspected native screenshots | Further visual review of each biome |
| Procedural maps from town/castle assets | Seeded room/platform generation; all36 rooms traversed on current ROM | Wider seed physical reachability, especially stairs/props |
| Controllable Sora/Donald/Goofy | Separate budgets/HP; menu and native party checks; Donald magic/Cure and Goofy spin/Guard | Current-ROM review of all moves and attached poses |
| Fight/recruit Cloud; party assembly only at round start | Actual fresh-run fight/recruit/deploy; recruit and alternative-power fixtures; original character cards in setup | Wider recruit/reward balance |
| Optional Aerith/Tifa suggestion | Aerith gEarF00/F01/B00/B01 idle/walk and gEarisPalette confirmed; no Tifa resources found | Inspect actual Aerith resources and feasibility; neither recruit is implemented |
| Original Rally | Five-hero roster; menu/save/reset, walking/actions/air native tests and golden asset checks | Card and six hurt/climb/cast poses implemented and native captures inspected; further visual review |
| Selectable Move/Attack/Skills/Party/End Turn | Compact single-stroke font, 18+16 idle and 56+16 contextual windows; rendered native UI checks | Visual usability review of all submenus |
| Turn order visible | Available party members then enemy phase; freely ordered party actions; forecast before ending | Review enemy-phase strip and all depleted/KO combinations |
| Movement tiles, heights, chests, doors | Native projected previews, costs, cancel, collision execution, composed descent; all-room nine chest campaign | Wider seeds and combined ledge/jump route discovery |
| Card combat and sleights | Original artwork, draw/discard/reload, three-card sleights, targeted Fire/Cure/Guard; host/native tests | Current-ROM review of each character/recipe combination |
| Bosses and roguelike rewards | TT Guard Armor, Agrabah Jafar, Castle Marluxia; completed campaigns; personal power/sleight rewards, Cloud summon-or-power choice | Balance and reward variety; additional recruit bosses |
| Suspend saves | Current native alternating slots/reset/corrupt-newest fallback12 checks, exact saves in both complete campaigns | Multi-seed replay with reset/retry |
| Verified complete runs | Current main PASS119943; all36 rooms/nine chests PASS281892, stable victory120 frames | 0.34 two-seed PASS288347; all-room defeat130336 retained, companion-recovery job78252 live |

Next implementation priority after the active seed test: resolve a reproduced
physical navigation defect if one appears; otherwise finish Rally's missing
visual states and card, then review combat/rewards across all heroes. Explicit
fixture checks and host graph checks must not be described as fresh full-run
or universal physical reachability proof. Physical cartridge testing is useful
optional validation, not an additional user-imposed completion requirement.
