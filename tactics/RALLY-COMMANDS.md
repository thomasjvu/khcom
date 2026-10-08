# Rally and native tactical commands

Rally is available during round setup in either companion slot. Select that
slot with Left/Right, then cycle heroes with Up/Down. Deploy with A or Start.
She has her own 64-HP roster identity, pink-haired sprites and shared native
16-entry palette. Her melee strike uses the field controller; her Cure and
party Cure sleights receive a four-point healing bonus, with the usual caps.
Individual upgrades, health, budgets and facing persist in save format 12.
Formats8–11 remain readable and initialize Rally at full health on migration.

R+Select opens the command panel. Up/Down selects Move, Attack, Skills, Party,
End Turn or Suspend; A confirms and B returns. Move opens the actual reachable
map/cost preview. Attack chooses an available Key card. Skills opens the card
selection submenu, with L/R or Left/Right to browse and A to use. Party switches
the active member without renewing budgets. Existing field shortcuts remain.
A lighter single-stroke 5x7 font replaces the bold debug glyphs.

Rally's source is the approved `pink-ponytail/starter-sheet-v2.png` artwork.
`tools/import_rally.py` reproducibly compiles it into20 indexed32x64 OBJ frames,
a shared15-color-plus-transparency palette, and aligned native data. The source
is preserved. Native foot anchor is(16,56); artwork height is at most44px.
The current attack and air animations reuse approved walking poses. Dedicated
action art, improved directional animation, a portrait/card and fuller moveset
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
