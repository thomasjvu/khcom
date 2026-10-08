# Rally and native tactical commands

Rally is available during round setup in either companion slot. Select that
slot with Left/Right, then cycle heroes with Up/Down. Deploy with A or Start.
She has her own 64-HP roster identity, pink-haired sprites and shared native
16-entry palette. Her melee strike uses the field controller; her Cure and
party Cure sleights receive a four-point healing bonus, with the usual caps.
Individual upgrades, health, budgets and facing persist in save format 12.
Formats8–11 remain readable and initialize Rally at full health on migration.

Select opens the command panel (R+Select remains an alias). Up/Down selects Move, Attack, Skills, Party,
End Turn or Suspend; A confirms and B returns. Move opens the actual reachable
map/cost preview. Attack chooses an available Key card. Skills opens the card
selection submenu, with L/R or Left/Right to browse and A to use. Party opens a deployed hero selector without renewing budgets. L+Select switches heroes directly; Start+Select suspends. Other field shortcuts remain.
A lighter single-stroke 5x7 font replaces the bold debug glyphs.

Rally's source is the approved `pink-ponytail/starter-sheet-v2.png` artwork.
`tools/import_rally.py` reproducibly compiles it into20 indexed32x64 OBJ frames,
a shared15-color-plus-transparency palette, and aligned native data. The source
is preserved. Native foot anchor is(16,56); artwork height is at most44px.
The current attack and air animations reuse approved walking poses. Dedicated
action art, a portrait/card and fuller moveset
remain unfinished. She currently starts unlocked rather than having a recruit
encounter. This is integration work toward the requested game, not a finished
Rally character or finished UI redesign.

The native input-only Rally/menu fixture verifies deployment and active control,
menu selection/submenu/back behavior, no navigation resource cost, reachable
Move preview and menu suspend/reset persistence. Host sanitizer checks include
Rally deployment and exact saved upgrades/health/budgets/facing. A recorded
format11 prop-top save also migrates through the current host reader into
format 12 with Rally initialized. Current whole-campaign and combat-specific
Rally/menu verification still need to be completed.

Skills now separates card selection from target confirmation. Left/Right or
L/R chooses the card; A opens the target screen. Up/Down cycles eligible Cure
or ranged targets. A confirms, while B returns without spending. The target
screen shows the selected hero and capped healing, or the ranged damage preview.
Ranged actions with no eligible target remain uncommitted. In card selection,
Up stocks the selected card and Down clears the stock. Three stocked cards
open a sleight confirmation with the existing map recovery/damage previews.
Attack routes stocked hands to Skills rather than unexpectedly executing a
sleight. Existing shortcut controls remain available.

The focused native target fixture uses explicit injured-party/card and enemy
placement/HP states, followed by controller-only menu operations. It passes
23 checks covering Rally's 18-point single Cure, cancel safety, empty ranged
target rejection, selected-enemy Fire damage, three-card stocking and Rally
Curaga healing/caps. The 14 input-only deployment/menu/save checks also pass on
the same ROM. The campaign verifier now snapshots all 39 meaningful bytes of
the five-hero roster and expects the starter unlock mask 23 on retry; its held
retry/reset policy test passes. Current whole-campaign evidence is tracked
separately and is not implied by these focused checks.

Attack now opens a separate confirmation rather than immediately spending its
Key card. Melee characters can set facing with the D-pad; B returns directly
to Commands without attacking or spending. A commits the native strike.
Donald/Cloud ranged Keys retain enemy target selection. A dedicated explicit
Key-hand/enemy fixture passes 10 checks for Rally facing, cancel safety, native
damage and exact card/action charging. The 23 target/sleight and 14 input-only
Rally/menu/save checks also pass on this Attack-confirmation ROM (47 total).
These fixtures do not establish whole-campaign completion.

The Commands list now includes contextual availability and short explanations
beside each command: no movement left, spent action, missing Key card, stocked
hand, hero switching, enemy turn and suspend. Rendering reads the hand without
cycling it or changing resources. The native build and all 14 input-only
Rally/menu/save checks pass; menu screenshot inspected for fit.

Select now opens Commands directly, with R+Select retained as an alias.
L+Select is the direct party-switch shortcut. Start+Select suspend and Select
after victory/defeat retain their meanings. The input-only Rally/menu/save
fixture passes16 checks, including plain-Select menu access and L+Select
cycling through Goofy and Sora after resume. The explicit Donald/Goofy combat
fixture passes8 checks for actual ranged magic/spin damage and action costs
using the new party shortcut. The retry policy regression passes. Full current
ROM campaign completion remains unverified.

Skills now offers Reload through Down when no cards are stocked. Down with a
stock still clears it. Reload has a separate confirmation showing its one-action
cost; B returns to Skills without spending. Confirmation returns discard to
draw and replenishes the hand through the authoritative deck implementation.
A spent action blocks it. Nine explicit discarded-hand fixture checks pass,
including exact five-card draw and cancel safety; all23 target/sleight checks
pass on the same ROM. Build/header/capacity checks pass and the screenshot was
inspected. Full campaign evidence from prior ROMs remains separate.

Rally now turns between front and back views when facing or moving, including
diagonals. Because the draft's opposite-facing front rows are inconsistent,
the consistent front/back pair is mirrored for right-facing poses; the source
art is preserved. Animation banks reuse the existing native friend state,
with no added reserved RAM. The input-only directional fixture passes26 checks
covering deployment, menus, suspend/reset, party cycling, cardinal/diagonal
animation banks, horizontal mirror flags and unchanged movement/action budgets.
North/east screenshots were visually inspected. Dedicated action/air artwork
still remains unfinished; directional walking is not a substitute for it.

Move's grounded preview now offers R → Jump confirmation, with D-pad direction
selection, explicit standing/moving costs, A commit and B back to Move.
Confirmation uses the existing native jump controller and resource charging,
without extra reserved RAM. No landing preview is claimed. Ten input-only
checks pass for navigation/cancel safety, direction, actual jump travel and
settling, exact movement/action costs and spent-action rejection. All26 Rally
directional/menu/save/party checks pass on the same ROM; Jump screen inspected.
Integrated walk/climb/jump destination planning and broader campaign proof
remain unfinished.

End Turn now requires a separate menu confirmation showing each deployed
hero's live remaining movement/action resources and KO status. A starts the
authoritative enemy phase; B returns to Commands without advancing the turn.
Eight input-only native checks pass for confirmation/cancel safety, exactly
one enemy phase and refreshed party resources. The10-check Jump regression
passes on the same ROM; End Turn screen visually inspected. Direct Start
remains the existing shortcut. No new reserved RAM or save format changes.

Party now opens a dedicated deployed-hero selector rather than cycling
immediately. Up/Down highlights, A switches directly while grounded, and B
returns to Commands. Health, movement, action and KO status are shown for
each deployed hero. Direct selection uses the same activation path as the
L+Select shortcut and preserves positions, facing and independent budgets.
The input-only selector fixture covers Sora/Goofy/Rally, cancellation, actual
Goofy movement and retained resources across switches; Rally directional/menu/
save regression is checked separately on the same ROM. Full campaign and
standing-surface switch coverage remain unfinished.

All12 input-only selector checks and26 Rally regression checks pass on the
selector ROM. Native build/header/capacity checks pass; list rendering inspected.

Key confirmation names the selected hero's actual move. Goofy's Shield Spin
shows area damage intent and the count of positive per-enemy native previews,
rather than melee-facing instructions. Fourteen explicit combat/menu fixture
checks and10 Rally Attack regression checks pass on this presentation ROM.
Confirmed spin hits both previewed enemies; cancel preserves card/action and
enemy health. Screen inspected. Packaged0.26 campaign proof remains tied to
its earlier ROM hash, separate from this presentation update.

### Ledge commands and native drop repair

While hanging from a ledge, Select opens Climb/Drop. Up/Down selects, A
confirms, and B closes the menu without dropping. Confirmation uses the
original ledge physics and preserves the movement/action costs already paid
for the jump. Direct Up/B field controls remain available with the menu closed.

Dropping clears the previous jump direction, preventing continued horizontal
input from pulling the falling hero back into the ledge. Two native fixtures
pass all16 checks across Climb, Drop, browsing, cancellation, surface settlement
and unchanged resource costs. These use an explicit recorded saved-room
approach and input only after boot; they prove one ledge, not campaign-wide
coverage. Exact ROM hashes and logs are preserved in
`build/tactics-us/ledge-command-{climb,drop}-fixed-evidence/metadata.json`.
No reserved RAM or save-format change. The broader all-room replay runs on
the earlier8f5584ea8 ROM and does not validate this newer menu/drop change.

### Skills browsing details

Skills browsing uses plain card names instead of A PLAY when A actually opens
confirmation. It now shows selected card value, stock count out of three, and
action cost or spent-action status. Original card artwork remains visible.
No control, combat, reserved RAM or save change. All23 explicit health/card
fixture checks pass for target cycling, cancellation, confirmed healing/ranged
damage, costs and Rally Curaga. The rendered Skills screen was inspected.
Exact ROM/driver hashes are in `skills-detail-target-evidence/metadata.json`.
The running full campaign uses preceding b5b66489f, not this presentation change.

Skills browsing now names the active hero attack consistently with confirmation:
Sora Keyblade, Donald Magic, Goofy Shield Spin, Cloud Sword or Rally Strike.
The native16-check Donald/Goofy fixture passes combat, two-target spin, menu
cancellation, confirmation costs and spent-action Skills browsing. Goofy
browsing screen inspected with card value/stock/action status and original
card artwork visible. Initial added fixture selected Party due to an extra
Down; failure preserved separately, corrected input passes. Exact evidence
`hero-skills-browse-fixed-evidence` is scoped to this later presentation ROM;
the full campaign still runs on e2e4a18d8. Native build/header/capacity pass.

### Skills-detail full campaign and Guard effect confirmation

The immutable e2e4a18d8 ROM passes one fresh-SRAM input-only three-world
main-route run: victory111583 Sora80HP, stable PASS111703,13 kills,873
commands. Cloud recruited13099/deployed14900; suspend14915 room4 and exact
resume15245; composed descent verified1332. Required coverage was checked
at victory. Exact ROM/driver/log scope is in
`skills-current-save4-main-evidence/replay-metadata.json`. This is one seed
4411213, not all-room/chest/multiple-seed/hardware coverage; later hero labels
and Guard effect text are outside this campaign ROM.

Guard confirmation now explains its party-wide protection and duration until
the next enemy phase ends, plus one card/action cost: ordinary Guard reduces
hits to one damage; Goofy blocks all incoming damage. All24 explicit native
Donald/Goofy combat/menu checks pass including both guard states, cancellation
and card/action costs. Both Guard screens visually inspected. Exact current
fixture hashes are in `guard-menu-effect-evidence/fixture-metadata.json`.
This changes presentation only, with no reserved RAM/save/combat change.

### Skills pile counts

Skills now shows two-digit hand, draw and discard counts alongside card value,
stock and action status. Counts derive directly from the authoritative deck
and exclude stocked/burned cards, so players can judge reload availability.
No persistent RAM/save/control change. All24 native Donald/Goofy/Guard fixture
checks pass; Skills screenshot visually shows HAND05 DRAW06 DISCARD01 after
one card play. Build/header/capacity pass; exact evidence is in
`skills-pile-count-evidence`. The ongoing all-room replay uses preceding
85dd43827 ROM, not this newer HUD. Save request26126/suspend26130/exact
resume26460 passed there; all three Traverse Town chests collected by90125.
This is intermediate coverage, not terminal all-room success. Rally combat
drafts inspected: visible fringe/noise requires cleanup before native import;
other-chat source assets remain untouched.

### Reload availability and no-op verification

Reload confirmation shows the authoritative number of discarded cards to
recover. With zero discard it says NOTHING TO RELOAD instead of advertising
a charged action. Native behavior remains unchanged: only a successful
reload spends an action. All11 explicit native fixture checks pass including
confirmation/cancel, actual reload drawing five, spent-action rejection, and
empty reload preserving available action and both piles. Both12-card and
empty confirmation screens visually inspected. Exact evidence in
`reload-count-evidence/metadata.json`; native build/header/capacity pass.
No reserved RAM/save change. Broader all-room replay remains on85dd43827
ROM; current intermediate evidence has5 chests and Agrabah room3, no terminal
full-coverage result yet.

### Compact field HUD and visible phase sequence

Normal field play now shows only two rows: active hero HP/movement/action
and PARTY THEN ENEMIES (Select opens Commands). During the enemy phase
it switches to ENEMIES THEN PARTY. The game uses freely ordered party
actions followed by its enemy phase, not individual initiative; the display
reflects those rules. Detailed card/pile/threat/control text stays in menus
or relevant targeting/ledge/reward states. Top dim window18px, bottom dim
window16px for card values; full original card artwork remains visible.
Both normal and enemy-phase screenshots visually inspected.

Encounter-clear upgrades now use a selectable list with the receiving hero,
owned/max markers and eligible Cloud recruitment alongside powerups. Up/Down
chooses; L/R changes recipient; A confirms through existing rules. Reward
text uses contiguous menu rows inside a64px window. All14 native encounter
clear/upgrade/suspend/exact resume and real Donald damage checks pass. The
old fixture used four-hero cleared/phase offsets; failed evidence retained
in `reward-list-evidence`. Updated five-hero fixture and exact current ROM
evidence in `compact-hud-reward-evidence` pass. No reserved RAM/save/game
rules change. Build/header/capacity pass. Wider all-room run is still on
85dd43827, separate from this HUD.
