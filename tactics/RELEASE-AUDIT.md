# Current release audit

Current source adds compact charged-attack warnings, SHA256
`f92bc47131331b1eb168b5b668b6e14c7ebbe8edc6c3457bb1c97c8b8b1e6a2d`. It passes254 scoped native checks (58 UI,114 boss/Cloud,44 party,12 SRAM,
14 Rally action and12 Rally state fixtures). Three-run
PASS434168 (process exited0); all-room script ended in native defeat137348 in
Agrabah room10. Its terminal diagnostic is preserved for investigation.

Earlier packaged0.36 ROM, SHA256
`27acd304ee14ab79fd4e1892d0cb5bb807b2adde0dd093245d2c8876c7b29950`.
It passes342 scoped native checks (40 UI,17 Donald healing,26 Goofy
moves/menus/Guard,89 boss,57 reward and113 height-preview fixtures). All-room/nine-chest campaign passes295578 on this exact ROM; revised three-run
campaign passes448492, with required native descent/reset/recruitment each.
Earlier policy failure444879 remains preserved.
Latest packaged build is0.37 (current warning hash above); finishing-Fire
all-room replay92308 also failed137348. Decision-time diagnostic50504
repeated the failure; reason diagnostic30946 is live. Earlier0.35 SHA1667d8885c5db2c5e09592554b736ad27ffe955180ef597a557de34a162c7f9f
passes51 scoped checks, two campaigns327681 and all36/nine chests295578.
Earlier0.34 two runs PASS288347 and its all-room defeats130336/134022 remain
version-scoped. Earlier0.33 all-room PASS281892 is likewise historical.
This audit concerns the requested complete game. Historical entries in PLAN.md
describe earlier versions; current evidence wins.

| Requested behavior | Current evidence | Remaining work |
| --- | --- | --- |
| Fork and native GBA ROM | Fork thomasjvu/khcom, draft PR1, tactics native target, byte-identical BPS apply | Final release packaging and reproducible instructions |
| Original 2.5D scenes, actors, cards | Native field renderer/collision/animation; inspected native screenshots | Further visual review of each biome |
| Procedural maps from town/castle assets | Seeded room/platform generation; all36 rooms and three seeds completed on0.36 | Wider seed physical reachability, especially stairs/props |
| Controllable Sora/Donald/Goofy | Separate budgets/HP; menu and native party checks; Donald magic/Cure and Goofy spin/Guard | Current ROM Donald healing17 and Goofy spin/Guard26 checks pass; other hero/recipe combinations and visual review remain |
| Fight/recruit Cloud; party assembly only at round start | Actual fresh-run fight/recruit/deploy; recruit and alternative-power fixtures; original character cards in setup | Wider recruit/reward balance |
| Optional Aerith/Tifa suggestion | Aerith gEarF00/F01/B00/B01 idle/walk and gEarisPalette confirmed; no Tifa resources found | Inspect actual Aerith resources and feasibility; neither recruit is implemented |
| Original Rally | Five-hero roster; menu/save/reset, walking/actions/air native tests and golden asset checks | Card and six hurt/climb/cast poses implemented and native captures inspected; further visual review |
| Selectable Move/Attack/Skills/Party/End Turn | Compact single-stroke font, 18+16 idle, 40+16 Commands and 56+16 detailed windows; rendered native UI checks | Visual usability review of all submenus |
| Turn order visible | Available party members then enemy phase; freely ordered party actions; forecast before ending | Six native VRAM checks cover ready, exhausted, KO, fully exhausted and enemy phases; wider combinations remain |
| Movement tiles, heights, chests, doors | Native projected previews, costs, cancel, collision execution, composed descent; all-room nine chest campaign | Wider seeds and combined ledge/jump route discovery |
| Card combat and sleights | Original artwork, draw/discard/reload, three-card sleights, targeted Fire/Cure/Guard; host/native tests | Current-ROM review of each character/recipe combination |
| Bosses and roguelike rewards | TT Guard Armor, Agrabah Jafar, Castle Marluxia; completed campaigns; personal power/sleight rewards, Cloud summon-or-power choice | Current boss mechanics/resource/pose fixtures89 pass; visual review, balance and reward variety remain |
| Suspend saves | Current native alternating slots/reset/corrupt-newest fallback12 checks, exact saves in both complete campaigns | Multi-seed replay with reset/retry |
| Verified complete runs | 0.34 two-seed PASS288347; 0.33 all36/nine chests PASS281892 | 0.35 two runs PASS327681 and all36/nine chests PASS295578; final broader polish audit remains |

Next priority: review remaining native combat
and menus across heroes and worlds. Rally's original card and six extra states
are implemented. Explicit fixture checks and host graph checks are not fresh
full-run or universal physical reachability proof. Physical cartridge testing
is optional validation, not a user-imposed completion requirement.

## 0.35 compact Commands development pass

Current native ROM `1667d8885c5db2c5e09592554b736ad27ffe955180ef597a557de34a162c7f9f` has 51 passing scoped native checks (26 input-only
UI/save, 19 disclosed companion combat fixtures, six rendered turn-strip fixtures). Commands occupies 40 pixels
at the top plus the 16-pixel bottom strip; phase order remains visible in the
menu. This is a development build, with two-run and all-room completion on its exact
ROM. The previously pending 0.34 recovery-policy all-room replay failed by
native party defeat at134022. Full polish/release scope remains open.

Group finishing-Fire replay now considers the weakest eligible foe among multiple enemies, cycles native targets when needed, and only attacks when the actual native damage forecast confirms a kill. No native game rules or HP changed. Policy regression checks pass. Fresh current-ROM all-room run PASS253238; terminal process exit0 and ROM/driver/log hashes verified. Driver requires all36 room visits, nine chests, Cloud recruitment/deployment, exact suspend/reset and composed descent. Evidence: build/tactics-us/charge-warning-group-finisher-all-rooms-evidence. Earlier failed attempts remain retained. Final polish remains open.
