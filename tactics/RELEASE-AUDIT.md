# Current release audit

Current packaged0.38 ROM SHA256 `2d438556ac07c2e9340f6c9591d6435413bc42e5174ed0321e3dc8fe1688c7af`.
It passes540 scoped native checks:95 menu/party/recipes,114 bosses/Cloud,
113 height previews,58 rewards/deployment,52 companion combat,56 party/SRAM
and52 Rally actions/states/directions. Three-run PASS379646, frontend exit0.
Initial aggregate180000-frame
attempt failed180001 and is retained. Current-ROM all-room completion is
not yet verified. Skills-strip predecessor passes all36/nine chests253238;
that result is version-scoped. Final gameplay/visual audit remains open.

Previous packaged0.37 adds compact
charged-attack warnings, SHA256
`f92bc47131331b1eb168b5b668b6e14c7ebbe8edc6c3457bb1c97c8b8b1e6a2d`. It passes304 scoped native checks (58 UI,114 boss/Cloud,44 party,12 SRAM,
14 Rally action,12 Rally state and50 card-recipe fixtures). Three-run
PASS434168 (process exited0). Fresh all-room/nine-chest run PASS253238
(process exited0) with group finishing-target policy. Earlier native defeats
137348 and their diagnostics remain preserved.

Earlier packaged0.36 ROM, SHA256
`27acd304ee14ab79fd4e1892d0cb5bb807b2adde0dd093245d2c8876c7b29950`.
It passes342 scoped native checks (40 UI,17 Donald healing,26 Goofy
moves/menus/Guard,89 boss,57 reward and113 height-preview fixtures). All-room/nine-chest campaign passes295578 on this exact ROM; revised three-run
campaign passes448492, with required native descent/reset/recruitment each.
Earlier policy failure444879 remains preserved.
Historical packaged0.37 (warning hash above); finishing-Fire
all-room replay92308 also failed137348. Decision-time diagnostic50504
repeated the failure; reason diagnostic30946 also failed137348. Earlier0.35 SHA1667d8885c5db2c5e09592554b736ad27ffe955180ef597a557de34a162c7f9f
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
| Verified complete runs | Current0.37 three-run PASS434168 and all36/nine-chest PASS253238, exact ROM hashes and terminal processes checked | Final broader polish audit remains |

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

Current0.37 exact-ROM recipe verification: 50 native checks PASS (14 mixed-stock/save/reload/cancel,11 multi-target range/height,16 Curaga/Triple Key,9 recipe suspend/reset). Explicit initial card/HP/enemy-position fixtures; stocking/save/play use native inputs. Each process exited0 and ROM/driver/log hashes verified. Complete evidence: build/tactics-us/charge-warning-recipes-complete-evidence. Initial800-frame mixed-stock run was incomplete before final880-frame cancel check and remains preserved; complete1000-frame run passes14. Scoped native total now304. Other hero/recipe combinations and visual review remain open.

Skills turn-strip polish: native source now keeps the available-party/controlled-marker/enemy-phase list visible in Skills, replacing a redundant control row. Footer includes A Next, B Back and one-action cost; no extra HUD height. Current ROM SHA256 59a8670fa43268e7cdd2e85895ec862192ba0e6f4c32827ebb5b200f643e00c7. Input-only native deployment/menu/save replay PASS29, process exit0 and hashes verified; skills screenshot inspected. Fresh all-room/chest/reset/recruit/descent campaign session73310 live. Prior0.37 304 checks and complete campaigns remain scoped to previous f92 hash; this source change is not yet fully campaign-verified.

Skills-strip exact current ROM: additional native Party14, Reload11, Target23, Attack10 and Jump10 checks PASS68. Each bounded process exited0; exact ROM/driver/log hashes and expected counts verified. Party uses native input only; other fixtures disclose health/cards/enemy placements before native input execution. Together with input-only menu29, current scoped UI/menu total97. Party and Cure target native captures inspected; no center-field obstruction. Party movement-point label MP is ambiguous and remains a concrete wording fix. Full-route73310 remains live (last observed87322), not yet a current-build campaign pass.

Party menu clarity: movement budget now explicitly reads MOVE instead of ambiguous MP, alongside HP and ACT. Same panel geometry and independent native budgets. Exact current ROM 2d438556ac07c2e9340f6c9591d6435413bc42e5174ed0321e3dc8fe1688c7af; input-only Party replay PASS16 with actual native VRAM labels for Sora, Rally and Goofy, native move/switch persistence, exit0 and hashes verified. Native party screenshot inspected for clipping. Previous Skills-strip all-room73310 remains live; its ROM predates this label edit and evidence remains version-scoped.

0.38 Skills/Party UI development package: 96242-byte BPS apply verified byte-identical; exact current ROM 2d438556ac07c2e9340f6c9591d6435413bc42e5174ed0321e3dc8fe1688c7af. Current native checks PASS95 (Party16, menus29, recipes50), terminal processes/hashes verified. Earlier Skills-strip all-room73310 finalized PASS253238/exit0; this precedes MOVE label and stays version-scoped. Current three-run31013 remains live. Development package is not final polish proof.

0.38 native boss/Cloud verification PASS114: Guard Armor40, Jafar25, Marluxia30, Cloud recruit19. Exact ROM/driver/log hashes and expected checks verified; all four bounded processes exit0. Disclosed initial encounter/health/position/charge fixtures, then native attack/Guard/phase/save/reset/reward input. Includes rendered charged-warning text, range/height, multipart phases, original actor/card allocation restoration. Current scoped total209; three-run31013 still pending.

0.38 height previews PASS113: base32, multi25, actor occupancy15, top14, descent/walk27. Native generated-stair placement fixtures disclose position/blockers; preview/cancel/costs and original controller/save/reset execution checked. Exact ROM/driver/log hashes and exit0 verified. Scoped current total322. Initial three-run31013 script FAIL180001 because aggregate --frames was180000 rather than prior540000; not native defeat. Its frontend still live at last poll and failure retained. Corrected aggregate-limit replay59790 live, unchanged native ROM/requirements. CLI help now explicitly explains aggregate frames and validation wording.

Current0.38 reward checks PASS40 (availability7, personal Donald power14, Cloud alternate-power19), exact hashes/exit0 verified. Cloud deployment18 fixture fails multiple assertions; retained without PASS claim, requires source/fixture diagnosis. Current proven scoped total362. Disk cleanup removes only identical current-ROM copies from completed PASS fixtures; metadata/scripts/saves/logs/screenshots and canonical build/release ROMs retained. Campaign59790 live.

Cloud deployment fixture repaired for five heroes: preserve Rally unlock31, native cycle through Rally to Donald, restore Goofy through native assembly controls before recruiting/deploying Cloud; persisted Donald HP/action/facing offsets20/30/35. Complete restored-Goofy replay PASS18, exit0/hashes verified, current scoped total380. Original and intermediate failures retained. Native game rules unchanged; current three-run59790 remains live.

Current0.38 companion checks PASS52: Donald healing/revival/bonus/save17, Goofy spin/Guard menus26, Cloud native sword/action/recovery9. Exact ROM/driver/log hashes verified, all processes exit0; explicit card/health/enemy fixtures disclosed. Current scoped total432. Three-run59790 remains live. Audit opening now distinguishes current build from historical campaign evidence.

0.38 current-ROM three-run script PASS379646: victories130570/259594/379526 HP75/74/73, stable120 frames each; actual per-run recruitment/deployment, exact reset and composed descent required. ROM/driver/log hashes verified; frontend59790 still live, no exit claim. Fresh exact-ROM all36/nine-chest replay81924 live. Additional native party-state44 and alternating SRAM12 checks PASS56, bounded exit0/hashes verified; corrupt-newest fallback explicitly disclosed. Current scoped total488.

Current0.38 Rally checks PASS52:14 input-only action/air,12 disclosed native hurt/cast/climb,26 input-only directional facing. Hashes/counts/exit0 verified; encoded30-frame/six-state fixed-feet/palette asset checks pass using system python3 (venv lacks Pillow). Scoped current total540. Three-run59790 frontend exit0 confirmed; metadata finalized PASS379646. Exact current-ROM all36/nine-chest81924 remains live.
