# KH CoM procedural field tactics plan

## Product direction

Build the game inside CoM's original 2.5D field engine. Sora explores generated
rooms made from each town/castle world's tiles, full sprites and props. Height,
collision, stairs, ledges, climbing, breakables, chests and doors are gameplay.
Movement and combat are constrained by tactical turns. Striking a monster
resolves combat in the field; it never starts the original battle view.

The generator must create new tactical layouts rather than reproduce exact
original maps. Preserve the visual projection, resource pipeline and physical
interactions. Do not replace field actors with icons or flatten elevations.

## Implemented foundation

- Reproducible US decomp build; independent tactics target and fork/PR.
- Native mode wrapper boots directly into the field engine.
- Seeded 12-room graph with reciprocal doors, optional branches and reward rooms.
- Seeded custom platform parameters drive CoM's procedural room builder.
- Traverse Town, Agrabah and Castle Oblivion tiles/palettes/props load normally.
- Original Sora and field enemy animation, collision, camera and climbing tasks.
- Terminal doors cannot keep advancing floors after RUN CLEAR; ignored combat/turn input and Select retry are fixture-verified.
- Tactical flying-role spawns begin on their assigned floor; old fixed-ceiling format-8 encounters are repaired on resume while retaining damage and horizontal position.
- Open-door sword hits no longer start room synthesis or lock the native controller; reproduced against the original callback and verified by native sword/travel fixtures.
- Committed full-height native jumps, bounded world-space travel from rest and safe jump-to-stair handoff; input-only height/landing/cost tests.
- Player-facing attached-stair segment preview via L+Up/Down, projected original digit, A confirmation and B cancellation; 23 emulator checks include attached saves and drop.
- Budgeted 16-pixel stair segments, native ascent/descent/drop, safe party switching and exact stair suspend/resume.
- Selectable Sora/Donald/Goofy with individual movement/action budgets and original sprites.
- Selected Fire projects expected HP loss above the same nearest valid enemy used by play; accounts for value breaks, Donald bonus and remaining HP, with explicit no-target HUD.
- Selected Cure names the intended recipient using the same range/height/missing-health calculation as the actual heal; party replay covers injured and knocked-out targets.
- Individual HP, friend knockouts, selection skipping and Cure/chest revival.
- HUD and projected original-digit damage estimates using enemy range/height/guard rules, emulator checked.
- Seeded world-specific enemy groups with ranged Red Nocturnes, Darkballs and Black Fungus.
- Large Body exit guardian with a charge/area-strike cycle, evasion, Guard interaction and saved windup state.
- Enemy damage and positions persist across all visited rooms and reboot.
- Original Donald casting and Goofy guard animation resources. Donald and Goofy
  now use their original walking cycles only during actual horizontal travel,
  face the travel direction, and return to idle when stopped; the 34-check
  party emulator fixture also verifies saves, healing, Guard and knockouts.
  Original rise/fall poses follow native jump states, with 15 input-only jump
  checks per friend. The active sprite now stays at its collision position;
  only inactive coincident sprites receive a display offset. Climb-specific
  party poses remain unfinished. Original ground shadows now follow each
  living friend independently of airborne body height; source-only shadow
  changes pass the 15-check Goofy jump and 34-check party fixtures.
- Discrete enemy decisions, health, value checks, height-limited attacks and stronger exits.
- Walkable upper ledges above void, solid native prop collision sampling and live preview revalidation.
- Eight-direction local routes with one-point projected diagonals, native diagonal execution and quarter-segment geometry sampling.
- Player walk cursor projected with original value digits; exact route costs, budget rejection and native controller execution, tested with input-only mGBA replay.
- HUD renders original font glyphs in RAM and uploads during VBlank.
- Bounded 9×9 enemy route search with native floor/collision sampling, midpoint checks, height limits and occupied destination checks.
- Three-card native sleights, combined effect/value preview, first-card exhaustion, stock cancellation
  and original stock artwork; emulator-verified save/reload behavior.
- Native twelve-card shared deck, five-card hand, original card/value artwork,
  discard/reload, Kingdom Key/Fire/Cure/Guard, character bonuses and chest cards.
- Exact native suspend snapshots in checksummed dual SRAM slots; emulator-tested
  reset and latest-slot corruption recovery.
- Bounded HUD font tile allocation; mutable ROM data is rejected by the linker.
- Original chest animation with three seeded original-art card choices, selected
  values, one-time party healing and deck-capacity fallback. Native reward
  inspection blocks field commands and suspend until confirmation. A 21-check
  scenario and separate 25-check reward/save/reset fixture verify non-default
  selection and persisted cards, healing and opened-chest state. Fixture room
  transitions, chest approach and full-deck edge case use explicit fixtures, not input-only
  full-run evidence. Safe chest placement fallback remains in place.
- Three-world progression, defeat, run-clear and retry with a new seed.
- Room/world transitions retain selected member, all party turn budgets and Guard; actual native forward/back door crossings are emulator checked.
- Appended code and RAM, bounded Thumb hooks; original ROM assets keep their addresses.
- Input-only default-seed three-world victory on the final 0.14 ROM, stable for 120 further frames: 79,338 frames, 17 kills, 680 movement commands and 24 Goofy Guards; no teleports, forced exits or emulated RAM writes.
- Asset-free rules/save/graph tests and local mGBA smoke/scenario replays.

- Current 0.15 ROM also has an input-only three-world victory with 120-frame
  terminal stability at frame 82,661 (17 kills, 691 movement commands, Sora HP
  57). A prior replay on the identical ROM lost in Castle Oblivion; consistent
  replay outcomes still require investigation.

The exact party-loadout/healing ROM at `4025e129b` passes 59 focused native
checks and a fresh input-only three-world campaign with composed stair routing,
Cloud recruitment/deployment and suspend/reset. Victory is 102782 HP70; stable
PASS 102902, 19 kills, 720 movement commands. The byte-exact local 0.22 BPS and
manifest preserve its ROM/evidence hashes; see `tactics/PARTY-HEALING-0.22.md`.
This single-seed proof does not establish physical generation guarantees.

Current native source `79c27ab63` removes 48 bytes of redundant stair-preview
positions and draws from route waypoints directly. Reserved EWRAM is now
8132/8192 bytes (60 free). The exact ROM passes 113 stair/composed-route checks,
26 rendered party-loadout checks, 17 Donald healing/persistence checks, and
11 native retry/reset checks. Retry policy tests cover held/released input,
wrapped seed advancement, stale recruitment rejection, mode readiness and a
bounded initialization wait. An earlier two-run attempt passed one campaign
then failed because its one-frame retry input was not consumed; that failure
is preserved. The corrected driver holds Select for twelve frames and verifies
the advanced seed and reset roster before proceeding.

`compact-workspace-held-retry-evidence` passes campaign one: victory 110890
with HP80, stable state 111010, native retry verified 111022 (seed 2658846982).
Campaign two verifies Cloud recruitment/deployment, suspend/reset and composed
descent, then completes Traverse Town and Agrabah. It fails the 240000-frame
bound in Castle room 0, with repeated movement around a large pillar near the
lower platform edge. This is incomplete navigation coverage, not proof of an
impossible room. Inspect physical terrain/prop connectivity and the replay's
path choices for that seed before increasing any timeout or claiming two-run
coverage. The current memory-compact ROM is newer than packaged 0.22; older
patches and their manifests retain their original hashes.

## Remaining implementation sequence

1. **Height-aware tactical navigation.** Extend the implemented flat-surface walking
   preview and native route execution with stairs and climb/jump edges. Validate spawn-to-door and chest
   reachability for every generated room, then regenerate invalid rooms using
   a bounded retry policy. Current graph tests verify room connectivity, not
   complete physical navigation inside each room. The input-only traversal driver
   now uses a native movement/action resource-state planner for walking previews
   and execution. Eight overlay checks, nine occupancy/execution checks and
   33 route/refund checks pass; a fresh Cloud recruitment/deployment and
   suspend/reset three-world replay reaches stable PASS at 95063 (HP80,
   15 kills, 654 movement commands). Cached geometry keeps unchanged previews
   responsive, while A forces full revalidation. Attached-stair climb links now
   use the resource planner: 25 multi-segment climb/descent/save/reset checks,
   32 single-step checks, 15 vertical occupancy checks and 14 top-platform
   landing/cost checks pass. Up/Down selects
   several heights; original animation executes the route and completes native
   floor/top transitions. Floor markers include the actual landing offset.
   Attached stairs now also compose descent and walking onto a 7-by-7 landing
   grid within the existing workspace. R toggles height/floor selection; original
   controllers execute the paid route with actual-landing revalidation. The
   exact ROM passes 113 focused checks (27 composed route/toggle/save/party
   checks plus 86 stair regressions). A fresh input-only replay explicitly visits
   a generated staircase, verifies the composed route at frame 1308, recruits
   and deploys Cloud, saves at 15015 and verifies reset at 15345, then wins all
   three worlds at 106696 with HP80; stable PASS 106816, 16 kills, 745 movement
   commands. Three earlier victorious replays failed the strict composition
   coverage requirement because their navigation never visited attached stairs;
   those failures remain preserved. This proof covers one seed.
   Combined standing-surface walk/climb/jump links still need native surface
   discovery and route animation execution.
   The exact climb-route ROM also completes a fresh input-only Cloud
   recruitment/deployment and suspend/reset replay: victory 92563 HP75,
   stable PASS 92683, 15 kills and 649 movement commands. This is one seed.
   The input-only traversal driver
   now has `--all-rooms`: visit 0–1–2–3–4–9–8–1–2–3–4–5–10–11–10–5–6–7
   in each world, and require all twelve room bits before terminal PASS.
   The first Cloud/suspend replay hit its 180000-frame bound in Castle room 5
   after both earlier worlds traversed all rooms. This is incomplete coverage,
   not a room impossibility finding. The longer bound is now 300000 frames.
   `--collect-chests` requires nine native chest opens as well as all room bits.
   Card selection has read-only mock checks; controller-only opening remains
   verified in `all-rooms-facing-chests-evidence`: all room masks are 4095,
   nine chests open, Cloud is recruited/deployed, suspend/reset passes, and
   three-world victory occurs at frame 252406 with Sora HP80; stable PASS
   252526, 32 kills, 2014 movement commands. This covers one native seed and
   does not establish bounded generation or combined height-route guarantees. R+Dpad now faces the active
   character without movement, budget use or card cycling; 32 input-only native
   checks pass across all eight directions for Sora. Donald and Goofy each pass
   33 checks including controller-only assembly selection and the same facing
   state/budget checks (`donald-facing-evidence`, `goofy-facing-evidence`). Walking to face caused the earlier
   chest approach to oscillate; that incomplete replay is preserved. Approach diagnostics exposed
   a replay bug: the walking preview needed cancellation before facing/attacking
   a chest. That fix is in the new run. The first chest attempt stalled
   and is preserved in `all-rooms-chests-cloud-evidence`.
   It checks original generated geometry with actual controller movement;
   successful replay coverage still does not prove a generation guarantee.
2. **Authoritative field combat.** Party/enemy occupancy now uses continuous segment/open-box intersection with overlap-exit handling. Host checks compare 5,000 three-dimensional segments and their reverse directions against an independent floating-point slab oracle. Enemy quarter-terrain probes interpolate height. Eight native movement overlay checks pass; a Cloud-route full run passes in `swept-occupancy-cloud-full-run-evidence` (victory 83630 HP80, stable PASS 83750, 16 kills, 598 commands, verified suspend/reset). Nine native player-route checks reject party/enemy corner crossings, keep the reachable mask consistent, restore the edge after moving blockers, and execute it once; evidence: `actor-crossing-final-evidence`. Further validate enemy-driven crossings, extend solid-prop/terrain sweeps beyond quarter sampling, and add world-specific behaviors. Extend current damage and charge warnings into full area/range overlays.
   Enemy preview/turn targeting now prefers currently attackable members over nearer height-ineligible members. Independent party HP and partial encounter persistence are implemented. Keep animations
   running during input wait without advancing authoritative actions.
3. **Port the tested card systems.** Fire and Cure now support explicit enemy/party cycling with shared preview/resolution validation; Cure also projects capped recovery and revival HP. Extend targeting to other effects and expand the matching-type field recipes (enhanced Key/Fire power and Cure healing are implemented).
   The basic draw/discard/reload and card effects already run in the field.
   Remove assumptions about an 8x6 board. Add projected target/range previews
   and a GBA-sized card HUD using original resources.
4. **Roguelike content.** Add room roles, enemy groups, authored tactical motifs,
   richer reward pools, world-specific hazards, map-card modifiers and canonical field bosses. Traverse Town now has original multipart Guard Armor art with a warned 80-pixel slam; Agrabah now has original field/lamp art for a solo sorcerer Jafar with a charged single-target spell; Castle now has solo humanoid Marluxia using original battle idle, scythe windup and attack assets, with a depth-limited horizontal sweep and half-health enrage. Jafar's original giant Genie battle form is not ported. Guard Armor now uses original crouch/orbit animation frames for windup and impact. HP-derived hand-loss phases weaken its attack reach/damage and survive suspend, with original spark effects when card damage changes phase. Additional boss moves and richer world content need implementation.
   Isolate run-generation RNG from combat and presentation RNG. Test optional
   paths, persistent opened chests, enemy clears and backtracking.
5. **Suspend saves and complete-run QA.** Extend the implemented native dual-slot save as new systems arrive.
   The board save format is excluded from the native ROM target. Add input-only
   full-run tests for victory/defeat, doors, climbing, reward choices, reset and
   corrupted-save recovery. Produce a verified BPS patch and release notes.

## Verification boundaries

Host tests cover the old rules core and 3,000 generated room graphs. Native
mGBA smoke checks use controller input for movement, attack animation and turn
budgets. Native scenario checks use explicit fixtures to place an enemy/chest
within range and request room transitions; they verify field hit/reward
handlers and lifecycle/progression, not a complete player-driven run.

Party/card/save integration replays verify selection, independent budgets,
character bonuses, discards, reload costs, exact state restoration and
corrupted-slot fallback. Healing HP and corruption are deliberate test fixtures.
One default-seed input-only three-world victory is verified. Physical hardware,
physical route guarantees, optional reward/branch coverage and other seeds remain unverified. The polish goal remains active. Keep the matching US build
byte-identical and never include ROMs or extracted game assets in Git or patches.

## Castle pillar physical-path investigation

A diagnostic suspend reproduces seed 2658846982, Castle room 0, position
47044/96763/8192 and door 12288/99840/8192. It explicitly removes enemies
and overrides party state, so it is not input-only fresh-campaign evidence.
The first fixture incorrectly cleared room-created flags and left doors closed;
that failed run is excluded from normal door conclusions. The corrected fixture
preserves FLOOR_ROOM_FLAG_CREATED and still reaches its navigation bound.
Collision snapshots record original map masks and prop colliders read-only.

`tools/tactics_navigation_analyze.py` sweeps same-height floor footprints and
circular props at 16-, 8- and 4-pixel horizontal spacing. In the corrected
snapshot no walking path reaches the door tolerance; closest distances are
127.80, 127.80 and 123.80 pixels. Diagnostic exclusion of the large prop at
36864/167936/8192 (radius8192) makes the 4-pixel path reach the door tolerance;
excluding only the smaller prop does not. This model excludes actors, jumps,
view-dependent collider disabling and native execution, so it is evidence of
walking obstruction rather than proof that the room is impossible.

Original MapGmkGp01WaitHit in src/map/map_tasks.c supports field attacks that
disable its collider, mark its placement destroyed and play the break animation.
The current replay never deliberately breaks blocking scenery. Next: verify
native Keyblade interaction with this particular pillar, then integrate prop
interaction into navigation and repeat campaign coverage. Do not erase native
props or increase replay bounds solely to bypass the failure. Diagnostic ROM
copies were removed after disk exhaustion; saves, logs and snapshots remain.

### Static pillar identity correction

The large radius32/height32 Castle pillar is a gTaskDescMapGmk00 static
prop, not the smaller gTaskDescMapGmkGP01 breakable decoration. The earlier
suggestion that its collider could be removed by native Keyblade attack was
incorrect. `castle-static-pillar-evidence` passes seven native fixture checks:
identifies the original task at the recorded collider position, faces Sora by
controller input, attacks with an available Keyblade, and verifies that the
pillar stays solid while the card and action are spent. The approach, saved
room, card hand and absent enemies are explicit fixtures. This demonstrates
interaction behavior only, not campaign or complete physical navigation.
The preceding six-check attack attempt expected destruction and failed that
assertion; its log remains preserved. Further work must test native climbing
onto/over the prop or validate placement so solid scenery preserves a physical
route. Breaking the smaller decoration is not a fix for this obstruction.

### Verified native pillar traversal

`castle-pillar-crossing-evidence` passes eight native checks on the existing
memory-compact ROM. From an explicit approach 36 pixels to the right of the
radius32 Castle pillar, free facing then Left+B uses the original jump controller
to land on its 32-pixel top: x37533/y83968/z0, supporting ground8192. The jump
spends exactly one movement and one action. Native Start refreshes the turn;
three subsequent Left movement commands carry Sora over the prop and off its
far side: x18963/y83968/z8192, movement0/action1. Native physics lands on the
original supporting floor; no coordinates are written after the approach.
Screenshots record the top and far-side landing. The earlier six-check jump
probe passes separately. These are saved-room/approach/card/enemy-absence
fixtures, not input-only full campaigns or proof of door arrival.

The room's lack of a same-height walking path does not imply an impossible
room: native prop-top traversal exists. Next integrate prop-top connections
into reachable navigation and preview/execution, with action-aware jump cost,
then rerun the second seed's complete campaign at the original bounded limit.
Do not replace original scenery or claim the current replay already uses this
connection. The current traversal driver only models terrain wall connections.

### Native prop-top walking previews

Walking route generation now recognizes the top of enabled original scenery
colliders when the player stands above the terrain ground. Preview positions
preserve the native underlying ground and top height instead of snapping back
to the floor. Centers must remain inside the original circular footprint;
terrain footprint checks, actor occupancy, swept walking links and the native
controller still apply. No additional reserved RAM is used (8132/8192 bytes).
Ascent and leaving a prop edge remain separate native commands.

`castle-top-preview-final-evidence` passes eleven native checks on this ROM:
original static pillar identity, jump landing/resource cost, affordable top
walking preview, native confirmation at top height with one movement and
unspent action, and subsequent far-side floor landing. The approach and saved
room are explicit fixtures, so this is not full campaign proof. Twenty-seven
composed stair/descent/party/save checks pass separately on the same exact ROM.
The build/header limits and full host sanitizer suite pass. A screenshot
confirms original sprites/scenery and card-digit route overlay on the pillar.
The second-seed replay still needs prop connections in its navigator and a new
complete campaign verification; earlier campaign evidence is on older ROMs.

### Prop-top suspend regression discovered

The extended `--jump --resume` pillar fixture saves after a confirmed top
walking route, resets, checks exact x/y/z/ground and budgets, then attempts
far-side travel. `pillar-top-resume-evidence` fails exact standing-position
restoration: Sora loses the top height after reset even though the save succeeds
and movement/action budgets persist. Original pillar reconstruction succeeds.
This is a real incomplete suspend behavior for prop-top standing. Ground and
attached-stair saves previously passing do not prove prop-top persistence.

Two attempted collider-support initialization fixes also fail the exact-position
assertion (`pillar-top-restored-support-evidence`,
`pillar-top-camera-support-evidence`). Both native edits were reverted; their
exact ROM/log hashes remain in local fixture metadata. The resume test driver
now avoids duplicate pre-reset assertions after its frame offset. It remains
a failing regression, not a green release check. Next inspect the original
room-entry/camera/player collision update ordering and saved support identity
before implementing a verified fix. Full polished suspend behavior is unproven.

Prop-top resume tracing further narrows the failure. At frame650 the saved
x33797/y83968/z0/ground8192 is loaded exactly; by frame660 z396 and native
state FALL are observed, then x27647/z2376 at670 and floor8192 by690. The
original pillar reconstructs at x36864/y167936/z8192/radius8192/height8192,
so this failure is not caused by a different regenerated prop position.
`pillar-resume-prop-position-evidence` records this read-only trace and identity.
Trace entries before field initialization read invalid pointers and are not
used as evidence; add explicit native-mode readiness before interpreting them.
A one-time collision refresh and a 32-update bounded contact-refresh experiment
both fail. Native edits were reverted, and failed exact-ROM logs retained.
Next inspect player/prop collider flags and support contacts during the first
live field updates rather than assuming a contact refresh repairs the handoff.

### Prop-top suspend support restored

The readiness-guarded player/prop trace identifies the initial support loss:
Sora loads at the saved top x33797/y83968/z0/ground8192, but the original
pillar initially has node flags3 (active and skipped) while the player has no
standing contact. The entrance camera culls the regenerated platform. Native
prop updates pause during the room fade, so contacts alone cannot repair it.
The pillar becomes active too late, after native falling has begun.

Suspend initialization now snaps the original camera to the restored active
hero, refreshes scenery, enables only original platform colliders supporting
the exact saved surface/footprint, sets the player collider position and
rebuilds native contacts before player updates. No save-format or reserved-RAM
change is needed (8132/8192 bytes). A nearby floor or different prop top is not
substituted for the saved position.

`pillar-resume-final-evidence` passes all 17 native checks on ROM SHA256
b389aef49f4e30e9f04c058faf9eaa85d000c3191b8b5b241ebc0bc6b8cb8759:
original pillar identity, native jump and top walking, exact x/y/z/ground and
budgets after reset, assembly deployment, far-side floor landing and two-step
movement cost with action preserved. Live support traces remain GROUND state,
standFlags 1 and active pillar node throughout. The approach/card/save fixtures
are explicit; this is not a full campaign replay. The driver deploys with A
because the empty room was cleared before saving and resumes at round setup.
Earlier apparent far-side passes while already fallen did not prove crossing.

Twenty-seven composed stair/descent/save/reset/party checks also pass on this
exact ROM (`pillar-resume-descent-regression-evidence`), alongside the native
build/header/capacity checks and complete host sanitizer suite. Failed patch
experiments are retained in metadata, including two patches accidentally
placed in the carry-turn branch instead of suspend initialization. The
second-seed campaign's navigator still lacks prop-top connections; its prior
Castle stall is not claimed fixed by this suspend repair.

The packaged 0.23 support ROM also passes a fresh-SRAM input-only three-world
campaign (`prop-support-suspend-full-run-evidence`): composed descent/walking,
Cloud recruitment 13924/deployment 15610, suspend 15624/exact resume 15954,
victory 99973 HP80, stable PASS 100093 (15 kills, 721 movement commands).
Requested suspend coverage is now mandatory and reaches stage 3. An earlier
same-ROM victory skipped requested Traverse Town room 3, exposing a missing
coverage assertion; its metadata records failed requested coverage despite a
verified victory. The stricter replay suspends in visited room 4. No RAM writes
or forced exits occur. This is one-seed coverage, not all rooms/chests or a
repair of the second-seed navigator. The 47099-byte 0.23 BPS applies byte-exactly
against the verified US base; hashes and evidence are in its local manifest.
Release notes: `tactics/PROP-SUPPORT-0.23.md`. Full goal remains unfinished.

### Rally and command-panel integration

Native source e2e4e5cf4 adds Rally from the approved pink-haired artwork as a
fifth persistent roster identity, deployable into either companion slot. Her
converted native sprites/palette,64HP and four-point Cure/sleight recovery
bonus are integrated. Format12 persists all five heroes; formats8–11 retain
legacy four-hero payload decoding and initialize Rally. A recorded format11
pillar suspend migrates successfully through the sanitizer-enabled host probe.

R+Select opens Move/Attack/Skills/Party/End Turn/Suspend commands, with D-pad
selection, A confirmation and B back. Move opens native reachable previews;
Skills browses hand cards. Single-stroke5x7 text replaces bold debug glyphs.
Fourteen input-only native deployment/menu/preview/no-cost/save/reset checks
pass on the exact packaged ROM. Host tests cover Rally deployment and saved
upgrades/health/budgets/facing. Build/header/capacity checks pass; reserved
EWRAM 8140/8192. The69869-byte local0.24 development BPS applies byte-exactly;
the local manifest records ROM/patch/driver/log hashes.

This is unfinished integration. Dedicated Rally action/air poses, directional
refinement, portrait/card/recruit encounter and richer moveset remain open,
as do fuller targeting menus and a current whole-campaign replay. Earlier
0.23 campaign coverage does not prove this newer ROM. Details and controls:
`tactics/RALLY-COMMANDS.md`. The proposed read-only prop-top replay model failed
its native decision check despite passing a mock; that edit was reverted and
failed metadata retained. No claim is made that second-seed navigation is fixed.

### Native Skills target and sleight confirmation

The Skills menu now separates browsing from confirmation, with eligible
Cure/ranged target cycling, predicted capped recovery/damage, resource-safe
cancellation and no-target ranged rejection. Up stocks cards and Down clears
stock from the selection menu; a full stock opens a distinct sleight screen.
Attack sends stocked hands to Skills instead of silently triggering a recipe.
`menu-target-sleight-evidence` passes 23 native fixture checks (explicit health,
card and enemy placement/HP setup; menu and action resolution by input).
`target-menu-rally-regression-evidence` passes 14 input-only deployment/menu/
preview/save/reset checks on the same ROM. Native build limits pass with no
new reserved RAM. Campaign replay snapshots now cover all 39 meaningful roster
bytes and starter mask 23, and the real retry-policy mock passes. Whole-campaign
verification is running separately; broad polish and content remain unfinished.

The current target-menu campaign ends as bounded FAIL120001 in Castle room5,
seed4411213 (17 kills,766 moves). Cloud recruitment13431/deployment15810 and
exact suspend15825/resume16155 pass; worlds advance at47407 and96003. This
is incomplete whole-campaign coverage on the newer ROM, not a victory or
proof of an impossible room. Bounds were retained. Local0.25 BPS is71243 bytes,
byte-exact round-trip verified, with37 focused native checks (23 targeting/
sleight fixtures plus14 input-only Rally/menu/save checks). Manifest preserves
exact ROM/patch/driver/log hashes and the failed full-run scope. Release notes:
`tactics/TARGET-MENU-0.25.md`. Full goal remains unfinished.

### Command availability build campaign audit

Native source b9e04eab1 shows command explanations and availability without
mutating the hand. A fresh-SRAM input-only three-world replay with Cloud,
required suspend and composed descent retains its 120000-frame bound. It
recruits Cloud at15050, deploys at17529, saves at17543 and verifies exact resume
at17873. Traverse Town completes at34429 and Agrabah at81033. Castle room5
is entered at107734, room6 at113757 and boss room7 at117243. The replay ends
FAIL120001 in room7 (14kills,846movement commands), before victory. This is
stronger current-build campaign coverage but still incomplete, not a release
gate pass. The immutable local ROM, generated input-only driver, log, collision
snapshot and exact hashes are in `command-status-full-run-evidence`. The prior
0.25 room5 timeout remains separate evidence. An offline same-height analysis
of that prior snapshot approaches within10.72px but does not meet its strict
goal tolerance; it excludes jumps/actors/native execution and cannot establish
that the room is blocked. Full campaign completion and wider seeds/branches/
chests remain required.

### Directional Rally and broader campaign coverage

Rally now uses a coherent front/back pair, mirrored for right-facing poses.
The approved draft's opposite-facing rows are inconsistent and remain preserved
but unused for facing playback. Twenty-six input-only directional/menu/save/
party checks pass on source ebe62cce7; cardinal/diagonal banks, mirror flags and
unchanged budgets are checked. North/east screenshots inspected. Dedicated
action/air art, card and encounter remain unfinished.

The immutable Select-menu source8a6bac4d1 all-room/chest replay ends as
FAIL300001 in Agrabah room5 (19kills,2028movement commands). It visits all
12 Traverse Town rooms, collects3chests there, recruits Cloud50061/deploys
148960 and verifies suspend44787/resume45117. World1 begins232705; two more
Agrabah chests are collected (5total) before the frame bound. The long first
Cloud-room backtrack eventually exits at148858, so repetition there was not
a permanent stall. Current three-world all-room/chest proof remains missing.
No bounds were expanded after failure and no emulated memory was written.
Exact ROM/driver/log hashes and failure scope are preserved locally in
`select-commands-all-rooms-evidence`. This predates Reload/directional Rally
and is not evidence for those newer native builds.

### Current selector ROM full campaign verification

Native source66fa4d42b completes a fresh-SRAM input-only three-world campaign
with mandatory Cloud recruitment12711/deployment15168, suspend15182/exact
resume15512 and composed descent. Worlds advance31379/68946; victory98474
has Sora80HP, and terminal floor/HP remain stable throughPASS98594 (15kills,
745movement commands). No ROM changes or memory writes were needed. The replay
now retains an affordable native preferred preview for confirmation when its
negative score cannot be beaten by another candidate, rather than cancelling
and scanning/reopening that same destination. Native A still revalidates the
route. Real inspection-block policy tests cover cost, preview presence, visits
and scan fallback; retry policy passes. The120000-frame bound is unchanged.

The prior same-ROM driver endsFAIL120001 in Castle room7 (15kills,745moves)
and remains recorded separately. Both driver hashes and their exact scopes
are retained in `party-selector-full-run-evidence` and
`party-selector-preferred-full-run-evidence`. The success is one main campaign
path, not all rooms/chests/seeds, and does not prove the unfinished Rally art,
additional recruitment/content, integrated height destination routing or
hardware verification. Full goal remains active.

The current selector ROM is packaged as0.26 command-menu development:74304-byte
BPS, byte-exact US-base round trip,98 focused exact-ROM checks and the full
main-path campaign above. Local manifest preserves individual fixture scope
and ROM/patch/driver/log hashes. Notes: `tactics/COMMAND-MENU-0.26.md`. This
deliverable preserves the full unfinished goal and does not claim all seeds,
all branches/chests or final content/art/hardware completion.

### Prop-top party switching coverage

The unchanged0.26 ROM passes14 additional native checks for original Castle
pillar jump, top walking and cycling through companions back to Sora. Return
preserves the exact supported position, underlying ground and spent movement/
action budgets. Explicit saved-room/approach, valid Key hand and revived
companion fixtures are used; jump, previewed walking and switching use native
input. `prop-party-final-evidence` ties driver/log to the exact0.26 ROM hash.
An initial wrong saved position and a too-early End Turn input are retained
as failed attempts with reasons; the final fixture waits until the native
jump has settled before End Turn. No engine repair was needed or claimed.
This is one support location, not all surfaces or hardware proof. The broader
0.26 all-room/chest replay is separate and still has no terminal result.

### All-room replay ledge failure and hero attack presentation

The0.26 all-room/chest run endsFAIL300001 in Traverse Town room4 after its
last jump at28686, with nativeBusy2 and actorState9 (LEDGE_HANG). Suspend26216/
exact resume26546 passes. Final position65273,86089,11008, underlying
ground16384;4kills/285commands. This is failed coverage. The replay returns
early whenever busy, before issuing the Up/B controls that native ledge
physics can accept during a jump; ledge-aware replay recovery needs explicit
verification. The unchanged log, exact ROM/driver hashes and read-only process
sample are retained in `command-menu-026-all-rooms-evidence`. No restart or
frame-limit expansion was used to hide the failure.

Hero Key confirmations now name the actual moves: Sora Keyblade, Donald Magic,
Goofy Shield Spin, Cloud Sword and Rally Strike. Goofy shows area intent and
the number of positive native damage previews rather than the misleading
melee-facing instruction. Explicit Donald/Goofy combat and Goofy menu fixtures
pass14 checks, including cancel safety and confirmed two-target native damage.
Shield-spin menu screenshot inspected. This newer presentation build is
separate from the packaged0.26 campaign/all-room evidence.

### Native ledge-aware replay recovery

The replay now recognizes native LEDGE_CATCH/HANG while jumpBusy2 and holds
Up through the original controller. Other busy states still wait. On real
settlement, it clears stale route planning and resumes traversal. The actual
busy-controller policy test passes catch/hang input, animation wait, one
sequence count, route invalidation and idle continuation; preferred-preview
and retry policy regressions pass.

An explicit recorded saved-room/approach fixture on native source8f5584ea8
passes5 native checks: actual jump catch, real-helper climb input at336,
grounded idle upper-surface settlement at504 and preserved jump movement/
action costs. After boot it uses input only, including pending room-clear
reward/setup handling. The original failed fixture (reward left pending)
remains recorded separately. Existing older ledge tests are unchanged.
`ledge-replay-ready-evidence` preserves the exact ROM/driver/log scope. This
proves one recorded ledge, not completion of the all-room campaign; broader
coverage is being rerun separately without changing the300000-frame bound.
