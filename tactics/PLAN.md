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

### Ledge-aware all-room campaign result

The immutable8f5584ea8 ROM with eb48edce3 traversal logic reached Castle
room8 but failed the unchanged300000-frame bound at300001:29 kills,2658
commands and8 chests. Traverse Town and Agrabah were completed with their
optional branches. Cloud was recruited26108/deployed38360; suspend occurred
19709; composed descent verified1476. Castle room9 took245474..297240, then
exited successfully; the final room8 only had2761 frames before timeout.
The actor was grounded idle, busy0, height20480. This contradicts a permanent
room9 hang and does not establish that room8 is inaccessible.

`ledge-aware-all-rooms-evidence` preserves ROM/replay hashes, terminal log,
screenshot and navigation snapshot. Offline same-height analysis cannot reach
the height0 door from height20480; it excludes jumps/stairs and is diagnostic
only. No all-room success is claimed, and this earlier ROM does not validate
the subsequent Climb/Drop menu. Next coverage is a current-ROM main-route
regression, while optional-branch route efficiency remains unfinished.

### Skills browsing details

Skills browsing uses plain card names instead of A PLAY when A actually opens
confirmation. It now shows selected card value, stock count out of three, and
action cost or spent-action status. Original card artwork remains visible.
No control, combat, reserved RAM or save change. All23 explicit health/card
fixture checks pass for target cycling, cancellation, confirmed healing/ranged
damage, costs and Rally Curaga. The rendered Skills screen was inspected.
Exact ROM/driver hashes are in `skills-detail-target-evidence/metadata.json`.
The running full campaign uses preceding b5b66489f, not this presentation change.

### Climb/Drop campaign coverage failure

The b5b66489f native build reached victory95748 with Sora80HP and stayed
stable120 frames, recruited Cloud13711/deployed15252, and verified composed
descent1348. The replay correctly FAILED95868 because requested suspend
room3 was bypassed by the Cloud branch: suspend coverage0. This is not a
complete passing campaign test. Exact evidence is retained in
`ledge-menu-current-main-evidence`; rerun on latest Skills build requests
visited Traverse Town room4 without expanding the120000-frame bound.

The probe generator now rejects requested suspend rooms2/3 when Cloud
branch routing bypasses them, unless all-room traversal is enabled. Verified
that the invalid room3 configuration exits2 before ELF/output access; the
valid room4 configuration generated and launched normally. Coverage checks
at victory remain mandatory. This prevents silently scheduling an impossible
save-coverage requirement; it does not weaken the campaign verifier.

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

### Unresolved walking-goal refinement

The read-only replay walking search retains its16px horizontal/8px depth
lattice for resolved goals. If remaining weighted distance exceeds8px, it
tries an8px/4px lattice under the same8192-node cap and keeps that answer only
when it approaches the goal more closely. Every move still uses native route
preview/confirmation, with no emulated-memory writes or enlarged campaign
bound. A search fixture proves the coarse grid misses a narrow gap that the
finer grid reaches, and checks unchanged open/blocked routes. Preferred,
ledge and retry policy tests pass. This is planner-policy evidence, not native
proof that the Castle branch is fixed: predicted directions still use native
movement distances, so rejection remains possible.

`refined-walk-all-room-evidence` is a fresh-SRAM all-room/chest campaign on
1ac88e86c native ROM with the refined driver, Cloud recruitment, save room4
and composed descent required, unchanged300000-frame limit. It is running;
no all-room success is claimed before terminal coverage checks.

### Guard heading and all-room save readiness

Guard now has a dedicated GUARD A CONFIRM / PARTY PROTECTION heading,
removing inapplicable target-cycling instructions. All24 native combat/menu
fixture checks pass; screen inspected. Build/header/capacity pass. Evidence
`guard-heading-evidence` preserves the exact ROM and fixture hashes.

The refined all-room replay FAILED26083 at its suspend check after entering
Traverse Town room4, before broader route coverage. Final actor grounded,
busy0; flags10081. The pre-request flags were not captured, so a transition
race is an inference, not established cause. Failure metadata/log preserved
in `refined-walk-all-room-evidence`. Replay save readiness now excludes
freeze-player, room-create, auto-walk and exit-room flags, and logs request
flags before sending native save input. The fresh
`refined-save-ready-all-room-evidence` rerun requires all rooms/chests, Cloud,
exact save/resume and composed descent under the same300000-frame limit.
It is running; neither the save-race diagnosis nor full coverage is proven.
Other-chat combat draft assets remain untouched.

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

### Compact HUD during walking

Normal walking and native route movement retain the two-row HUD instead of
flashing expanded shortcut/threat text. Moving status still shows PARTY THEN
ENEMIES; targeting, jump/ledge and modal states retain relevant controls.
All14 input-only native party tests pass including actual Goofy movement in
progress with gWin0V18, independent Sora/Goofy/Rally budgets, cancellation
and switching. The moving screenshot was inspected with original scene,
actors and cards visible. Exact evidence in `compact-walk-party-evidence`;
build/header/capacity pass. No gameplay/persistent RAM/save change.
All-room campaign remains live on85dd43827, outside this newer UI evidence.

### Refined all-room terminal result and current two-run coverage

The85dd43827 native ROM with refined walking/save-ready replay FAILED
300001 in Castle room11 (entered298785),35 kills,2702 commands,8 chests.
Both early worlds and Castle's first optional branch/backtracking completed;
room9 exited276707 after entry251133. Grounded idle busy0 at final position
107686,70090,0. Save request26126/suspend26130/exact resume26460 passed.
This improves observed coverage over the earlier room8 timeout, but does
not prove all rooms/chests or victory. Exact terminal snapshot/log/hashes
preserved in `refined-save-ready-all-room-evidence`. No limit expansion.

`compact-hud-two-run-evidence` is running on4dec13bdf native ROM: fresh SRAM,
two sequential main-route campaigns using native Select retry/seed advance,
Cloud recruitment, room4 save/exact resume and composed descent each run.
The240000 total-frame limit is the existing two-run bound; this replay does
not enforce independent120000 per-run caps. No multi-seed success claimed
until both complete and terminal coverage passes. Read-only investigation
of chest-facing stalls suggests lock-on interactions may matter, but no
controller change is made without a focused native reproduction.

### Native tactical facing verification

A fresh-SRAM input-only fixture confirms all eight R+Dpad turn-in-place
directions persist after release, with unchanged exact position, movement3,
action1 and busy0. All9 checks pass on4dec13bdf. Exact ROM/driver/log evidence
in `facing-eight-direction-evidence`. This establishes the basic facing
control works at spawn; it does not cover the recorded chest approach or
prove lock-on caused those delays. No speculative controller change made.
The current two-run compact-HUD campaign remains live and is still in its
first run; no terminal/multiple-seed success claimed.

### Recorded chest-approach verification

An explicit saved fixture recreates Traverse Town room9 approach50387,59052,0
with north-facing R+Up input then native melee. All4 checks pass: persistent
north facing, unchanged movement/action before attack, actual chest opening
with pending reward and one spent attack action. After boot it uses input
only. The fixture clears encounters and fixes a valid Key hand; it is not
campaign/lock-on evidence with live enemies. The first fixture inherited an
earlier chest count and its failed assertion is retained separately.
`recorded-chest-counter-fixed-evidence` preserves current4dec13bdf ROM/driver
hashes. This rules out basic geometry/facing failure at that cleared approach,
but does not explain intermittent replay/live-enemy delays. No controller
change is justified by this result. Two-run campaign remains live separately.

### Settled native route diagnostics

Replay now records original/actual settled positions and movement/action
budgets after each preview-confirm attempt, once native busy processing
finishes. Existing STEP records are attempts, not proof of displacement.
The read-only diagnostic adds no input or frame-budget change. Preferred,
ledge and retry policy checks pass. Fresh input-only room0-to1 test passes
1081 and emits settled records; several attempts retain exact position and
resources, confirming why attempted-command counts cannot establish movement.
Exact evidence in `settled-route-diagnostic-evidence`; this short test proves
logging/traversal only. The live two-run replay still uses its earlier driver
and repeats Agrabah room5 position76060,86995,8192. A terminal snapshot or
focused route reproduction is needed before changing physics/planning.
No arbitrary lock-on suppression is introduced.

### Agrabah route fixture isolates live-state stall

A saved-approach fixture starts seed4411213/floor1/room5 at exact observed
position76060,86995,8192 with ground8192. Initial snapshot confirms these
values; goal door45056,95744,8192 and original prop colliders are captured.
The current settled-diagnostic driver reaches room6 at4479 using native
input only after boot. Fixture clears encounters and KOs companions, so it
does not reproduce the live party/enemy state or establish the stall's cause.
Some confirmed attempts have unchanged position/budgets while other attempts
execute, but the route ultimately succeeds. Exact evidence/metadata in
`agrabah-settled-route-evidence`; source fixture retained for further native
isolation. No physics change or campaign success claimed. Live two-run job
remains separate and pending.

### Two-run failure and runtime-state capture

The4dec13bdf two-run replay FAILED240001 in first-run Agrabah room5,11
kills/2004 attempted commands. No victory or second seed was reached.
Final grounded busy0 at76060,86995,8192. Exact terrain cells, props, position
and door equal the passing cleared Agrabah fixture; runtime party/enemy/menu
state was omitted, so that fixture does not fully reproduce the failure.
Evidence retained in `compact-hud-two-run-evidence`.

Navigation JSON now captures roster39 bytes, deck78 bytes, all three party
positions/health, six enemy HP/charge/position records, current party, budgets,
busy/preview/menu selection and field flags. Read-only, unchanged controls
and frame limits. Native room0-to1 passes1408; JSON parses and dimensions
validate in `runtime-snapshot-native-evidence`. Retry/ledge policy checks pass.
A new120000-frame current main-route run with this diagnostic is live in
`compact-runtime-main-evidence`, requiring Cloud, exact save/resume and
composed descent. This adds information needed for a faithful fixture; it
does not fix or claim campaign success. Other-chat assets remain untouched.

### Save failure contradicts transition-only diagnosis

Current compact-runtime main replay FAILED21194 at room4 suspend; request
21190 had flags81 (no freeze/create/auto-walk/exit bits), grounded busy0.
The transition-only explanation is contradicted. Snapshot captured valid
looking party/deck state but omitted save notice and pending encode state,
so ignored input versus encoding/verification rejection remains unresolved.
Failure retained in `compact-runtime-main-evidence`.

Snapshots now include save notice and a read-only pending FieldSaveState dump
whose byte length is derived from the exact ELF sSuspend symbol size. Native
room0-to1 passes979; JSON parses and1136-byte dump size verified in
`save-state-dump-evidence`. No validator/rules changes made. A fresh current
main campaign is live in `save-validator-current-main-evidence`, retaining
Cloud, room4 exact save/resume, composed descent and120000-frame bound.
Its pending-state dump will allow host validation if save failure repeats.

### Cross-ABI save-state inspector

Read-only `tools/tactics_save_state_inspect.c` accepts native SRAM or pending
state dumps and uses the unchanged production FieldSaveEncode/Decode as its
verdict. It prints deck/roster validity, party budgets and encounter-kind/
coordinate/cached-count inconsistencies. agbcc rounds encounter structs to
12 bytes (host10) and deck structs to80 (host78); explicit native mirror
layout1136 is translated into host FieldSaveState992 before validation.
No production format or validator relaxed. A loaded native Agrabah fixture
dump converts to correct floor1/room5/HP73 and passes production roundtrip;
known native SRAM also passes, uninitialized zero dump rejects as expected.
`native-save-layout-evidence` captures the actual native dump and4307-frame
room6 traversal. Current campaign save request22764/suspend22768 succeeded;
full terminal campaign result remains pending separately.

### Save-attempt timing and encounter-packing hypothesis audit

The save-validator current main run FAILED120001 in Agrabah room7,13 kills/
858 attempts, position78078,92971,16384; save/resume had passed earlier.
Its terminal sSuspend dump fails host validation with cached-count mismatches,
but NativeCaptureEncounter mutates that buffer after saves and world changes.
Thus this later dump does not contradict the earlier successful saved payload.
Ordinary native kills also clear sEnemyTasks before capture, so the proposed
dead-task packing explanation is unproven. No speculative encoder/capture
change is made. Failure/log/runtime evidence preserved.

Replay now captures `save-attempt.json` and its exact native-state bytes at
suspendStage1, immediately after save input and before reset/subsequent room
mutation, on both successful and failed requests. Retry policy passes;
`exact-save-attempt-main-evidence` is a fresh current-ROM main-route run with
Cloud, room4 save/exact resume and composed descent under120000 frames. It
is live. This timing correction makes future validator findings attributable
to the actual attempt; campaign and broader traversal remain unfinished.

### Rejected-preview recovery

Actual save attempt in `exact-save-attempt-main-evidence` has notice1 and
passes production host encode/decode after ABI translation. No save-rule
change made. The run remains live; this proves only that captured attempt.

If a confirmed move settles with unchanged exact position and unchanged
movement/action while a preview remains open, replay now cancels it using
native B and resets scanning. It avoids carrying an uncommitted cursor into
the next selection. Executed movement, charged resources and already-closed
previews do not receive this cancellation. Policy checks cover those cases;
preferred/ledge regressions pass. Native cleared Agrabah fixture logs actual
rejections812/1240/1388 and reaches room6 at2535,18 attempts, versus earlier
4479/45 attempts (timings are fixture observations, not broad guarantees).
Exact evidence in `rejected-preview-native-evidence`; no memory writes after
boot, gameplay changes or larger bounds. Full live-state recovery remains
unverified; current campaign uses preceding driver and continues separately.


### Compact ready-party turn strip

The normal field HUD keeps its18px top and16px bottom windows. Its second
row now names the eligible deployed heroes using SOR/DON/GOO/CLO/RAL,
with X marking the controlled hero, followed by THEN ENEMIES. KO members
and members with neither movement nor action remaining are omitted. During
native enemy resolution the row reads ENEMIES THEN followed by surviving
party members whose budgets refresh next. This is a selectable party phase,
not a fixed individual initiative order. Save notices temporarily replace
this row. No persistent RAM, gameplay or save format changed.

Native build/header/capacity checks pass. compact-ready-party-evidence has
14 input-only native party/movement checks passing on this exact ROM;
selected.png visually confirms the ready strip and exposed original world.

Earlier exact-save-attempt-main-evidence ended FAIL120001 in Castle room7,
16 kills/860 attempts. Its captured room4 save succeeded and production
encode/decode passed. Rejected-preview-full-main-evidence ended FAIL120001
in Agrabah room3,10 kills/907 attempts; save/exact resume passed. Cancellation
improves the isolated cleared fixture but does not establish full campaign
recovery. Both terminal logs and snapshots are retained. Neither is a full
campaign success; procedural traversal validation remains unfinished.

### Native SRAM library and acknowledged route inputs

The first rejected-direction main replay stopped at18736 after observing
unsuccessful save notice, while its state passes the production host encoder.
This was an observer verdict, not proof the native save function returned.
The stage-diagnostic repeat distinguishes SRAM verification notice3 from
state rejection notice2: save-encoded.bin exactly equals SRAM slot0, and
both SRAM and captured state pass production host decoding. Evidence is
retained in save-stage-direction-main-evidence. The direct verification
loop was observed at verification status3 with a valid written payload.
This did not establish that the function had returned; later native-update
timing evidence below corrects the earlier false-failure interpretation.

Suspend reads now use the original ReadSramFast. Writes use the original
WriteAndVerifySramFast and RAM-resident VerifySramFast, including their
wait-state setup and bounded retry policy. The inactive slot signature is
invalidated, payload verified, and signature committed last; generation
and selected slot advance only after complete verification. Added a far
call veneer for the original library; no persistent RAM or save format
change. State and SRAM rejection remain SAVE FAILED in the player UI.

native-sram-library-byte-check-evidence passes12 native checks: two saves,
alternating slots/generations, exact newest reset resume, and older exact
fallback after explicit newest CRC corruption. Both captured SRAM files
pass production host encode/decode. The initial version's three header
assertions incorrectly used read32 on eight-bit SRAM; its failure evidence
is preserved, corrected checks assemble bytes. Host deterministic30-run,
3000-world graph and1000 resource-route oracle suite passes. Native build,
header/capacity checks pass. Physical cartridge timing is not yet tested.

sram-library-main-evidence passes Cloud, room4 save/exact resume, composed
descent and both first worlds, but ends FAIL120001 in Castle room3 with
15 kills/899 attempts. It is not a complete campaign proof.

Replay route presses now wait until sRawKeys samples their held value and
the native main-loop counter advances after that update. Video frames alone
cannot release a press while native route search is still calculating.
Repeated keys first wait for an acknowledged release; waits have a360-video-
frame failure bound. This changes replay input delivery, not game rules.
Uncommitted confirmations also avoid retrying the same direction at an
identical position; displacement/room changes restore eligibility.

Explicit Agrabah room3 suspend fixture matches the live failed snapshot's
terrain, props, goal, roster/deck, positions, HP and budgets exactly. Current
native input reaches room4 at2119 frames/16 attempts with no uncommitted
confirmations, versus4740/36 before acknowledgement. The inherited4 kills
are source-save statistics, not combat performed by this fixture. Evidence
is in room3-native-ack-evidence; fixture generator is host-only and replay
does not write game memory after boot. Input/rejection/preferred/ledge/retry
policy tests pass. Fresh native-ack-full-main-evidence is still live; full
campaign, all-room/multiple-seed traversal and broader polish remain open.

### Current compact-HUD campaign and development deliverable

native-ack-full-main-evidence now passes97531 on exact native6b005110e:
Cloud9383/deployed10733, composed descent1214, suspend10747/exact resume11077,
worlds34034/72343, victory97411 Sora80 stable120 frames,14 kills699 attempts.
Fresh SRAM, no RAM writes, unchanged120000 frame bound; one main-route seed.

Packaged0.27 compact-HUD development BPS is77466 bytes and applies to the
supported US base byte-for-byte. Exact native ROM SHA256 is
7a82ead569c34249c12655470e5c64c4c17ac9667ec8d8032aae852cd0e36098.
Current14 party/movement and12 save/reset/corrupt-newest fallback checks
pass on that exact ROM. `.tactics_ram` is8140 bytes. Manifest preserves
ROM/patch/driver/log hashes and fixture scopes; notes in
COMPACT-HUD-0.27.md. All-room/chest native-ack-all-rooms-evidence is live
separately under300000 frames. Broader goal remains incomplete.

Two consecutive native-retry campaigns are also live in
native-ack-two-run-evidence, under a shared240000-frame total bound. The
first seed starts from fresh SRAM; the second must use the game's Select
retry and seed advancement, with renewed Cloud/save/descent requirements.
No independent per-run timeout is implied. Both wider jobs remain pending.

### Wider terminal results and collision-aware final approach

Both wider jobs are now terminal. All-room/chest run FAILED300001 in
Agrabah room5 after all12 Traverse Town rooms, both Agrabah optional
branches/backtracking and6 total chests;20 kills2991 attempts. Native mode
and frame progression were inspected read-only through mGBA's console;
inspection added no game memory writes or inputs. Final position
40612,92784,8192 is blocked before the forward door45056,95744,8192.

Consecutive run completed first victory97357 stable to97477, then verified
native retry97489 with seed2658846982. Second Cloud109437/deploy111100,
save111115/exact resume111445, composed descent116542 and world transitions
129635/169514 passed. It FAILED240001 in Castle entrance room0, not the
earlier Agrabah progress line; position50728,97439,8192, busy3/movement0.
No second victory, all-room success or multiple-complete-seed claim is made.
Manifest and0.27 notes now record terminal scopes rather than pending work.

Final approach previously bypassed walking search when within32px across/
16px depth of a door or chest. A diagonal can still hit a corner at that
distance. Replay now uses that shortcut only when the collision-aware
walking planner agrees with the direct direction; otherwise normal native
preview scanning chooses a detour. Aligned axes no longer receive an extra
diagonal component. Direct approach input waits for native acknowledgement.
This changes the explorer, not game collision or door placement.

The recorded cleared Agrabah5 fixture reproduces the old failure12001 with
141 attempts and unchanged position. Revised input reaches room6 at327 in
two commands. The fixture with all recorded live enemies restored reaches6
at330/two commands. Its initial terrain, props, goal, party/roster/deck,
resources and enemy HP/positions exactly match the failed live snapshot.
Fixtures inherit4 source-save kills; no combat is inferred. Evidence is in
agrabah-door-baseline-evidence, agrabah-door-guard-evidence and
agrabah-door-live-party-evidence. Host fixture codec/sanitizers and actual
approach-block, acknowledgement/preferred/rejection/ledge/retry policies pass.

The fixture build also exposed an incorrect assertion that host FieldRoster
sizeof equals its39-byte body. It has trailing alignment padding. Fixtures
now assert the last logical field's extent using offsetof and copy exactly
the39 source bytes, avoiding overread. The failed setup run is explicitly
invalid and preserved separately; no native ROM or save format change.

### Save completion timing correction

collision-aware-all-rooms-evidence ended its replay at23231 with notice2,
but its captured encoded buffer has zero CRC while production CRC should be
0x6c1932d7. Both SRAM slots were still uncommitted at that observation.
Later fresh.sav contains a complete valid slot0 and passes the production
codec. The observer caught NativeWriteSuspend while it was still computing
the checksum, rather than a completed encoder rejection. The earlier
notice3/byte-identical SRAM observation likewise did not establish a final
verification failure. The original-library integration remains, but its
reported motivation was stronger than the evidence supported.

Suspend request now uses the same native-input acknowledgement barrier as
movement: hold the chord until sampled, then wait for the main-loop counter
to advance after the whole update, including encoding/writing/verifying.
Only then snapshot and judge its final notice. The360-frame acknowledgement
bound remains. No codec rules are relaxed or save failures hidden. The
interrupted replay and its in-flight/later SRAM evidence remain preserved.

Fresh native-save-ack-all-rooms-evidence (native6b005110e, replayc967d25dc)
passes save request23334/completed suspend23342/exact reset resume23672.
Captured notice1 and completed CRC verify that the verdict follows encoding;
captured SRAM passes production host encode/decode. The eight-video-frame
native completion wait explains why the earlier fixed four-frame check was
premature at this checkpoint. This full all-room/chest run remains live
under300000 frames; main-route0.27 proof and wider failures remain distinct.

That run is now terminal: native party defeat131133 in Agrabah room10,
21 kills910 attempts, all12 Traverse Town rooms and5 total chests. Completed
save/exact resume remains PASS; all-room victory remains unproven. The
door-corner and save-observation fixes do not imply combat survival or fix
the separate second-seed Castle entrance stall. No replay is still live
from this batch; terminal logs/snapshots are preserved for next fixes.

Party UI development build0.29 (2026-10-07): ordinary movement retains
the compact18-pixel top window and16-pixel footer, including the eligible
party followed by enemy phase strip. Setup alone uses56 pixels at the top,
listing all three deployed heroes HP/movement/action/power, selected-slot
marker, reserve HP, and selected owned-sleight bonuses. Original companion
cards and approved Rally sprite move down to remain visible; role text
uses the small footer. No persistent RAM added (8140/8192).

NativeHud previously published26 before later choosing18, so an emulator
frame/VBlank could observe the provisional larger window during a native
update. Select the final size before building labels. A diagnostic confirmed
RAM18 during movement after this change; attempts to read write-only WIN0V
were invalid and are retained only as diagnostic failures. Final exact-ROM
input-only regression passes14/14 checks; rendered setup/Rally/menu/save
regression passes20/20. Host suite and native ROM header/capacity pass.
ROM SHA2568449f2fca08897350200e1f35babf4c4d1eaffc807f4d9334ff913c028d4c4b0.
Development0.29 BPS applies byte-identically to this ROM. Earlier provisional
0.28 artifact is not the current build. These focused UI checks do not
establish all-room or consecutive-run victory on the changed ROM.

Complete-run verification follow-up (2026-10-07): the earlier all-room
Agrabah defeat controller kept forcing Cloud into slot1 after he was KO,
ignoring unlocked healthy reserves. Cloud is now required to deploy once;
subsequent native assembly replaces KO companions with the healthiest
unlocked nondeployed reserve. A pending replacement target survives native
duplicate swaps while cycling. Native assembly presses now use the completed
input acknowledgement barrier. Combat can Cure injured Sora outside nearby
enemy range, while offensive cards still require nearby enemies. No game
health/damage or production save rules changed.

All12 replay-policy tests pass, including new reserve selection and expanded
Cure gates. The old party policy mock expected obsolete Select-only switches;
it now checks the real L Select input516. Native KO fixture test passes7/7
on development0.29: original Donald/Goofy KO fixture, native Rally selection
and deployment, unchanged reserve HP64 and no rejuvenation of knocked-out
heroes. An initial fixture mistakenly cycled past Rally; corrected one-step
fixture is the proof, both logs retained.

Input-only reserve-recovery-all-rooms-evidence is live, session73273/PID62815
when last inspected: native0.29 exactROM, all12 rooms/world, chest collection,
Cloud recruitment/deployment, composed descent, and TTroom3 suspend/reset,
300000 video-frame bound. Initial movement and Goofy Guard are observed.
It has not reached a campaign verdict; preserve and poll this same live job
before starting another. Source comment-only cleanup after generation does
not change that driver's behavior; generated hash remains authoritative.

End Turn menu polish (2026-10-07), development0.30: show HP, remaining
movement/action, and expected next enemy damage per living hero. The HIT
legend explains the forecast; KO heroes remain explicit. Uses the existing
NativePreviewThreats result shared with map intent, includes current Guard,
clamps displayed damage99, and allocates no persistent RAM (8140/8192).

Input-only end-turn-forecast-corrected-evidence passes9/9: three rendered
hero HP/threat values (including nonzero Sora damage), explanation, no
resource spend while inspecting, cancel preserves turn, confirm advances
one native turn. Initial evidence caught shifted damage digit indices;
corrected screenshot and exact-ROM replay prove the fix. Independent
forecast-compact-party-evidence passes14/14. Host suite/build/header/capacity
pass; local development0.30 BPS applies byte-identically. ROM SHA256
b50c5353cb229d1bdafd5e0826bbc146617a051f817391504683d2b3b1815833.
No whole-campaign0.30 verdict is claimed.

Existing session73273/PID62815 all-room controller job remains live on its
immutable0.29 ROM. It has verified composed descent1217 and reached TT
room1=8648, room2=11739, room3=14652, party HP80/32/72. No restart was
performed. Keep following this same job for save/recruit/reserve/full-run
evidence; new menu ROM and campaign ROM remain distinguished.

Rally dedicated action/air artwork (2026-10-08), development0.31: generated
new action-source-v1.png with built-in imagegen using approved starterv2
identity. Added10 frames (front/rear windup, punch, recovery, rise, fall) to
existing20. Compile with one shared action scale, 32x64 cells, foot anchor
16/56, existing15 opaque colors. Original20 frame tile bytes and palette
remain byte-identical, now protected by golden hashes in asset verifier.
Unrelated other-chat drafts were not modified or imported. Action provenance
and generation prompt are saved under character-assets/rally.

Rally attack animation now plays once and holds recovery instead of looping;
idle/walk still loop. Physics, attacks/resource rules and save formats are
unchanged. RAM8140/8192. Fresh configuration regenerated header dependencies
so native object actually rebuilt for rally_data.h; the older build.ninja
had omitted the newer asset header. Asset encoding/bounds/bindings and host
suite pass. Input-only rally-single-strike-actions-evidence passes13 checks
for both attack sequences, nonloop flags, rear rise/fall, full jump height,
landing idle and costs. rally-final-actions-direction-evidence passes26
for menus, save/reset, and front/rear/mirror banks on the exact final ROM.
Screenshots of strike and rise were inspected. Early fixed-video-frame
action harness missed end-turn input; native-completion acknowledgement
corrected it. An intermediate acknowledgement harness omitted required
symbols and is not proof. Final13+26 evidence is the proof.

ROM SHA2566ea7e1243f1cdd3ab8d9a3dfa7c6d97747379b0c9f509708346a34513ea65fe9;
local0.31 BPS applies byte-identically. Dedicated hurt/climb/casting art and
Rally's own card still remain unfinished; no full polish claim.

Same all-room session73273/PID62815 remains live on0.29. Read-only CUA
console inspection confirmed room3, partyHP80/32/72, native counter172,
move1/action1; subsequent logs advance through frame21254 and partyHP80/44/72
after room3 chest recovery. Inspection did not send game input or write game
memory. Suspend14733/exact reset resume15063 remains verified. The quiet log
was not a terminal job; no restart was performed. Keep following this job
for wider coverage, distinguished from0.31 artwork regression evidence.

Shorter contextual menus (2026-10-08), development0.32: menu/reward top
window now ends56 instead of64, directly after text row48; footer uses
144..160 instead of128..160. Darkened area drops96/160=60 percent to
72/160=45 percent during menus/rewards. Field18+16 remains34/160=21.25
percent. Every command/target/party/reward label still fits. Initial window
assignment chooses the same final size before label construction, avoiding
provisional larger windows during VBlank. No RAM, gameplay or save change.

Exact ROM proof: compact-context-menu-evidence12 PASS (dimensions, final
row, forecast/confirm/cancel); short-context-rally-menu-evidence20 PASS
(rendered setup, Rally controls, menus and reset); short-cloud-recruit and
short-cloud-power18 PASS each (explicit original boss fixtures, charge/
height/reach, save/reset, native Fire defeat, original summon card allocation,
fifth option and final controls rendered, recruit vs ordinary power and
no duplicate reward). Cloud fixtures were stale: updated five-hero masks
23/31 and reward offset17, formerly7/15 and offset15. Fixtures remain
explicit-memory tests, not input-only full campaign proof. Total68 native
checks, host suite and Rally tile/palette golden verifier PASS. RAM8140/8192.
ROM SHA2566f7a3b146689c71c1442566927fbe8ad30494e60ed19efa0af1192843057503d.
Local development0.32 BPS applies byte-identically; no complete0.32 run proven.

Same all-room session73273/PID62815 remains live on immutable0.29. Log
observed move settlement21554 after TTroom3 chest. One-second host process
sample shows mCoreSyncPostFrame waiting for video presentation; it does
not prove a native failure or terminal job. CUA returned from the read-only
Scripting inspection console to the verified campaign fresh.gba window and
raised that display; screenshot shows original opened chest, Sora/Donald/
Goofy, card artwork and compact phase strip. No game keys or memory writes
were introduced, and no restart was performed. Continue this handle for
terminal coverage; slow/quiet presentation is not completion or blockage.


Headless validation and replay input completion (2026-10-08): built the
optional official mGBA core pinned to cef7dde504af189e47f6074365f2d77e8177ad06.
Frontend-only changes provide pixels, automatic SRAM loading and a clean
video-frame bound. Current 0.32 native forecast12, Rally-menu20 and SRAM12
checks pass; forecast/setup/menu/resume images match desktop captures pixel
for pixel. Main-route campaign reached Castle room7 at116471 but failed
its120000 frame bound, so no complete current-ROM campaign claim. Separate
all-room job82835 remains live; observed Agrabah room10 at130251, five
chests, native suspend/reset and earlier optional Traverse rooms. Old GUI
job73273 is preserved. Full instructions: HEADLESS-VALIDATION.md.

Combat, chest opening, chest/progression reward confirmation and party-turn
inputs now use the same bounded native sampling/update acknowledgement as
movement and saves. A timed four-video-frame press was insufficient while
native route processing occupied multiple frames. All12 traversal policy
regressions pass. Independent current-ROM main-route job54345 with this
new driver is live; suspend committed8798. The running all-room script is
immutable and retains its older driver, preserving independent evidence.
README now identifies the actual 0.32 package and compact phase/menu UI.
This is progress toward the full goal, not completion; complete current
campaigns, wider seeds, remaining Rally assets and final content polish
are still required.


Current 0.32 complete-run proofs (2026-10-08): main-route input-only run
54345 exit0/PASS119943; all-room run82835 exit0/PASS281892. Both exact ROM
6f7a3b146689c71c1442566927fbe8ad30494e60ed19efa0af1192843057503d, fresh
SRAM, Cloud recruit/deploy, exact suspend/reset, composed descent and
Sora80 HP stable120 frames after victory. All-room masks4095 each, nine
chests, 41 kills. Generated driver/ROM/log hashes checked and metadata
finalized; local0.32 package manifest extended with scoped proof. Main
uses acknowledged combat/reward driver ed20c687c; all-room uses earlier
immutable script. Their earlier failures are preserved. Consecutive two-
seed native-retry job26675 is live at headless-ack-two-runs-evidence with
240000 frame bound. Full content/art/hardware polish still unfinished.


Consecutive-seed job26675 is now terminal exit0, replay FAIL240001. First
campaign PASS119943, native retry119955 seed2658846982. Second reached
Agrabah room2 at172148 then wandered to frame bound with all six enemies
already defeated, Sora74/Cloud49/Goofy72 HP, idle state and remaining
budgets1/1. Position100778,65253,0; exit12288,99840,20480. Snapshot, native
save bytes, terminal image and exact generated driver retained in
headless-ack-two-runs-evidence; metadata marks failure. Same-height read-only
analysis at16/8/4px found no progress toward the higher door (615.875px
weighted distance), but excludes stairs/jumps and cannot prove impossible
physical geometry. Inspect standing-region connectors and native ledge/jump
execution next; do not enlarge bounds or describe two-run success. Player
instructions now describe the compact ready-party/enemy strip and contextual
End Turn health/damage forecast; current release audit enumerates actual
requirements and missing proof/art/content without imposing hardware testing.


Second-seed obstruction diagnostic (2026-10-08): snapshot save-state bytes
are sSuspend from the earlier TTroom1 save, not current runtime. Production
inspection rejects them because their cached enemies became stale; no save
validator relaxed and no failing stale buffer used as exact terminal state.
Added explicit tools/tactics_agrabah_room2_fixture.c using the actual valid
second-seed SRAM, seed2658846982 floor1 room2 recorded positions/health,
refreshed budgets and deliberately cleared encounter cache. Production
encode/decode PASS. It differs in deck/hero identities and regenerates two
enemies, so it is a diagnostic fixture. Initial captured terrain cells and
props match terminal snapshot exactly. Baseline native input exits to room3
PASS16491. Region planner points at connector77824,68608,0; same-height
walking cannot approach around prop98304,118784 radius8192, hence jump/height
execution is necessary. This proves a route exists, not why campaign stalled.
Fallback jumps and region transitions now wait for native sample/update
acknowledgement. All12 replay policy regressions pass; equivalent acknowledged
jump fixture retains independent logs. No complete second-seed claim.


Rally original assembly card (2026-10-08): generated identity-preserving
emerald/gold square portrait using approved starterv2 reference; source and
provenance retained. New importer emits32x32/512byte OBJ,16-entry palette
and centered sprite. Native assembly loads it in the existing borrowed
combat-card bank, draws alongside original character cards, and frees it
through existing cycle/deploy/exit paths. No state or save format expansion.
Native20 Rally menu/deployment/suspend/reset checks PASS and setup capture
visually inspected; original30-frame golden verifier PASS. Existing complete
0.32 campaigns retain their exact hashes and do not prove this newer ROM.
Two-seed job14103 remains live on its immutable pre-card ROM.


Replay connector escape (2026-10-08): acknowledged-jump two-seed job14103
terminal FAIL240001 at second seed Agrabah room2, position100705,65259,0.
It reproduced the same post-clear loop; timing alone did not fix it.
The stuck-position policy always selected the same connector diagonal and
reset visit count, so repeated failures had no direction memory. New policy
tracks attempted jump directions per coarse position/height, initially
prefers the connector, then tries all eight directions before repeating;
room changes/native retry/reset clear attempts. New read-only policy test
checks direction coverage and separate origins/heights; all13 policy tests
pass. An independent explicit Agrabah room2 fixture is retained against the
new Rally-card ROM. This is replay navigation, not altered game geometry or
forced input/memory writes. Fresh campaign validation remains required.


0.33 replay follow-up (2026-10-08): escape-jump two-seed job87983 terminal
FAIL240001. First victory127398/stable127518; native retry127530. Second
seed2658846982 now passes the prior Agrabah room2 obstruction at212878 and
reaches room6 at224918 before shared bound. Failure remains retained. Main
replay maximum is now180000 per run (all-room remains300000), making the
fresh two-run bound360000 rather than assuming every route fits120000.
Live job65844 uses this finite limit; do not infer its result from first-run
or fixture success. All-room exploratory policy job38219 ends in genuine
native party defeat135375, not a navigation timeout. Its failure is retained.
Independent established all-room driver from9245e3140 is running on exact
0.33 with identical36-room/nine-chest/Cloud/reset/descent requirements:
job92124 at rally-card-baseline-all-rooms-evidence. ROM/ELF addresses are
current, driver hash/source revision explicit. This tests presentation-only
Rally-card integration while exploratory policy remains separately audited.

Generated additional Rally hurt/casting/climb draft is saved unchanged as
states-draft-v1.png; it incorrectly contains translucent haze and duplicate
climb poses, so it is not imported. Built-in image edit is correcting alpha
and alternate arms/knees (cell564). Runtime remains0.33 until actual asset
validation and native integration are complete; no missing pose claim made.


0.33 complete campaign proof (2026-10-08): jobs65844 and92124 terminal exit0.
Two consecutive fresh/native-retry seeds PASS292638 (first127518; retry127530
seed2658846982; second victory292518, Sora67HP stable120). Both Cloud recruit/
deploy, exact suspend/reset and composed descent verified. Prior Agrabah
room2 blockage passed212878. Independent established all-room policy PASS
281892, room masks4095 each, nine chests,41kills, Sora80 stable120. Exact0.33
ROM64bb02ac04b1e662fc839bbd5c4c3479a626f856cd02f28e752658364c661505;
metadata/log/ROM hashes checked and package manifest finalized. The defeated
exploratory-policy all-room attempt remains retained as separate evidence.

Rally original six state poses integrated: hurt after actual nonzero HP loss,
cast for non-Key cards/sleights, held climb for native stairs/ledge states.
Source3 solid-green sheet compiled against unchanged palette, original30
frames retain golden hashes, all six4bpp encodings/boots verified. Native49
checks PASS:12 disclosed native-damage/generated-stair fixtures,14 input-only
melee/air,23 menu/deploy/card-OBJ/save-reset. Captures inspected; hostPASS.
RAM still8140/8192; new ROM5f1a98008c3f8e9e6d27f12466fe4744de8035a34743cd7f543d9563bd3f846b.
Initial action regression failed its input wait because ELF has two local
sRawKeys symbols; corrected generator resolves tactics reserved-EWRAM local.
A second attempt mislabeled Fire as rear melee after draw advanced the hand;
explicit Key selection now verifies all rear strike stages. Failed logs
remain. Traversal generator also resolves the tactics input variable now.
Complete0.33 proof does not prove this newer state-art ROM; run fresh campaigns.


Fresh state-art ROM campaigns are live: job27303 at rally-states-two-runs-
evidence (two native-retry seeds,360000 bound, current exploratory policy)
and job36732 at rally-states-baseline-all-rooms-evidence (all36 rooms/nine
chests,300000 bound, established9245e3140 policy with current ELF addresses).
Both require actual Cloud recruit/deploy, exact suspend/reset, composed
descent and stable terminal victory. Generated headers now resolve tactics
sRawKeys rather than its vanilla duplicate. Observed positions and command
logs progress; do not infer completion from any earlier ROM proof.


State-art build packaged0.34 (2026-10-08): exact ROM5f1a98008c3f8e9e6d27f12466fe4744de8035a34743cd7f543d9563bd3f846b,
BPS96168 bytes, production apply byte-identical.49 native checks, host and
asset golden/state encoding PASS; RAM8140/8192. Two-seed job27303 terminal
PASS288347: first stable123214, retry123226, second victory288227 Sora67
stable120; recruit/deploy/reset/descent per run required. Baseline all-room
job36732 terminal native defeat130336, retained. Neither terminal result
is inferred from earlier ROMs. Metadata hashes verified; manifest scoped.

Replay companion recovery now heals injured/KO nearby friends when Sora is
healthy rather than reserving every Cure for the leader. Eligible companion
checks match actual96px raw-plane range and24px height; out-of-range targets
are not cycled toward. New read-only cases cover injured Donald, KO Goofy,
height and range exclusions; all13 policy regressions PASS. Native gameplay
and ROM remain unchanged by this replay policy. Full36-room/nine-chest job
78252 at rally-states-party-recovery-all-rooms-evidence is live under300000
frames; its terminal result remains required. Aerith original resources are
confirmed as gEarF00/F01/B00/B01 with gEarisPalette; these provide idle/walk
only. No Tifa sprite resources found in this checkout. Neither added recruit
is claimed implemented. Current release remains development, goal active.

## Compact commands pass (0.35 development)

Native Commands now uses a 40-pixel top panel, three rows in two columns,
with the ready party followed by the enemy phase. Together with the 16-pixel
bottom strip this leaves 104/160 pixels unobscured. Ordinary movement retains
the 18-pixel header. Detailed menus remain contextual.

ROM SHA256 `1667d8885c5db2c5e09592554b736ad27ffe955180ef597a557de34a162c7f9f`: input-only menu/assembly/preview/suspend/reset 26/26;
explicit native Donald/Goofy moves fixtures 10/10 and Cloud deployment/sword
fixture 9/9. Action poses play once; idle/walk loop normally. The first Cloud
fixture observed too early for idle; the final fixture waits for native recovery.
Evidence is in `build/tactics-us/compact-commands-*-final`. Header/capacity and
Rally asset encoding checks pass. Earlier complete campaigns prove their
recorded 0.34 ROM only; this build has no new complete campaign proof.

The 0.34 companion-recovery all-room attempt also ended in native party defeat
at frame134022 in Agrabah. This remains a failed campaign, not a completion.

## Exact 0.35 campaign verification

Replay turn policy now locates Goofy by deployed roster identity, current HP
and remaining action, rather than assuming slot two. Read-only tests cover
Goofy swapped into slot one and no Goofy deployed. This changes the test driver
only. Fresh-ROM sessions73601 (two runs,360003 emulator frames) and92992
(all rooms/nine chests,300003) use exact 0.35 ROM, native input acknowledgement,
Cloud fight/recruit/deploy, composed descent and exact suspend/reset. Their
results remain pending in compact-commands-two-runs-evidence and
compact-commands-all-rooms-evidence. Host suite completed successfully.

0.35 replay checkpoint: first fresh campaign victory129297 HP76, stable120
frames, RUN COMPLETE129417; native retry verified129429 seed2658846982.
Both sessions73601/92992 were polled and remain live. Second run and all-room
result are still pending; this checkpoint does not prove both goals complete.

0.35 turn-strip native verification: six disclosed budget/HP/phase fixtures
pass against actual VRAM glyph pixels. Covers controlled ready hero, narrow
idle windows, exhausted controlled hero, KO Donald, no remaining party budgets
and enemy-first phase with living next party. Screenshot inspected. Total
scoped native checks51; no new ROM changes or campaign restart.

## Exact 0.35 complete campaign results

Two-run session73601 exited0 and script PASS327681: first victory129297
HP76 stable120 to129417; native retry129429 seed2658846982; second victory
327561 HP63 stable120 to327681. Per-run Cloud recruit/deploy, exact suspend/
reset and composed descent requirements checked. Independent all-room session
92992 exited0, PASS295578: victory295458 HP80 stable120, room masks4095 each,
nine chests,38 kills,1894 moves; same Cloud/reset/descent requirements. Native
final capture inspected. ROM1667d8885c5db2c5e09592554b736ad27ffe955180ef597a557de34a162c7f9f
and generated driver/log hashes verified; metadata/manifests finalized. These
prove their finite native campaigns and do not close the full polish audit.

## Command column navigation

Native Commands Left/Right switches columns on the same row; existing Up/Down
cycling remains. Header now says D PAD. Native ROM SHA256 `27acd304ee14ab79fd4e1892d0cb5bb807b2adde0dd093245d2c8876c7b29950`.
Input-only column checks8/8, existing input-only assembly/menu/preview/save/
reset26/26 and rendered turn-strip fixtures6/6 pass on this exact ROM.
Full three-run session81749 (540003 emulator frames) and all-room/nine-chest
session58715 (300003) are fresh native input-only campaigns, pending. No
earlier complete-ROM evidence is promoted to this newer hash.

## Current native companion combat review

On ROM27acd304ee14ab79fd4e1892d0cb5bb807b2adde0dd093245d2c8876c7b29950,
Donald healing17/17 and Goofy moves/menus/Guard26/26 pass with disclosed
fixtures. Donald uses specialty+8 and owned Cure+4, caps recovery, revives
Goofy, spends one action with exact card exhaustion, and resumes identical
party/deck/upgrade state. The first Donald fixture failed three expectations
because stale byte9 now addresses Sora; corrected byte10 targets Donald.
Failed evidence retained, native code unchanged. Goofy held-Guard test waits
until48-frame action timer reaches zero, verifies pose1 persists and animation
loop flag remains clear. Native screenshots captured. Total83 scoped current
native checks, including40 previous UI checks. Full campaigns still pending.

Latest command-column ROM all-room campaign session58715 exited0,
PASS295578. All three room masks4095, nine chests, Sora80 HP stable120,
Cloud fight/recruit/deploy, exact suspend/reset and composed descent. ROM/
driver/log hashes verified. Session81749 remains live after two completed
seeds, now running the third; three-run completion is not yet proven.

## Current native boss review

Exact command-column ROM boss fixtures pass89 checks: Guard Armor37,
Jafar24, Marluxia28. Deployment now happens by native A input in every room.
Old fixtures stopped in assembly and failed; intermediate failures retained.
Guard Armor waits bounded180 video callbacks for the original break spark to
expire while authoritative enemy decisions remain frozen. Jafar/Marluxia
confirm native boss power rewards before testing resource release on exit.
Marluxia floor setup no longer pre-clears Agrabah's boss before entering it,
which previously triggered an unconfirmed reward and blocked advancement.
Native game code unchanged. Exact hashes/logs recorded. Total scoped checks172.

Three-run session81749 exited0 but script FAIL444879: third seed1018315455
reached victory444759 HP80 stable120, actual Cloud recruit/deploy and exact
suspend/reset stage3, but composed descent count0. First two complete129417/
327681. No three-run completion is claimed; missing route execution needs
replay investigation. All-room PASS295578 on the same ROM remains valid.

## Deliberate descent coverage across seeds

The three-run failure came from seeking a composed stair only in world0 room0.
Replay now seeks a qualifying stair in any current room/world until coverage
completes; native landing preview, cost/edge verification, walking execution,
arrival/budget assertions and terminal count requirement remain intact. New
read-only policy test covers later-world targeting, absent stairs, completed
coverage and unrequested detours; all15 policy tests pass. Fresh three-run
session21328 uses unchanged current ROM and540003 emulator-frame bound,
pending. Earlier failure preserved.

Packaged0.36 development BPS95902 bytes, applied32MiB byte-identical. Exact
hash/172 native checks/all-room evidence in COMMAND-COLUMNS-0.36.md and
ignored manifest. This does not claim final release or three-run completion.

## Current native roguelike reward review

57 exact-ROM native fixture checks pass: availability7, ordinary Donald
power reward14, Cloud recruit18 and alternate boss power18. Five-hero roster
availability offsets corrected (phase16/reward17/Sora sleights9); game code
unchanged. Capped/owned selections preserve the reward; available selection
adds exactly one unlock. Donald power persists through reset and increases
actual/predicted magic by1. Both Cloud choices preserve their distinct roster
state through reset and release original art resources on exit. Initial short
1700-frame attempts were incomplete; final1900/2400 runs include every check.
Hashes and metadata finalized, total229 current scoped native checks. Revised
three-run session21328 polled live in its second seed; result remains pending.

## Current native height preview review

113 checks pass on exact0.36 ROM: base32, multi-segment25, occupancy15,
top-boundary14, composed descent-to-walking27. Fixtures initially position the
actor on real generated stairs (occupancy also moves blockers); subsequent
preview/confirm/execution uses original native physics. Covers cancellation,
vertical/combined costs, actor occupancy, route boundaries, budgets, attached
state persistence, exact suspend/reset and party selection after arrival.
All five processes exited0; count/ROM/script/log hashes verified. Total342
scoped current native checks. These fixtures complement, rather than replace,
the full all-room campaign. Three-run session21328 remains live on third seed.

## Three current native campaigns completed

Revised any-room descent replay PASS448492 on exact0.36 ROM: first stable
victory129417 HP76; retry129429 seed2658846982; second stable victory326093
HP76; retry326105 seed1018315455; third stable victory448492 HP80. Per-run
Cloud recruitment/deployment, suspend/reset stage3 and native composed descent
all required and verified. Third descent385146 in Agrabah validates the policy
fix. ROM/driver/log hashes finalized; earlier FAIL444879 retained. Emulator
session21328 completed its540003-frame frontend bound and exited0.

## Compact charged-attack warning

Native compact idle HUD previously cleared the older windup text while
rebuilding its ready-party strip. It now adds one warning row naming charged
Guardian/Guard Armor phases/Jafar/Cloud/Marluxia attacks. Header26 and footer16
leave118/160 battlefield pixels clear. Turn phase strip remains; header returns
to18 when charge clears. Charge is sampled before publishing window geometry
to prevent intermediate-frame window mismatch.

New ROM `f92bc47131331b1eb168b5b668b6e14c7ebbe8edc6c3457bb1c97c8b8b1e6a2d`: native58 checks pass (six disclosed warning/room-transition
fixtures,26 input-only menu/save,8 input-only columns,6 turn-strip fixtures and
12 input-only End Turn). The first warning-window mismatch was fixed and its
evidence retained. Earlier342 checks/campaign proofs stay scoped to0.36.
Fresh three-run session84597 and all-room/nine-chest63209 are live on exact
new ROM. Full polish audit remains open.

## Charge warning combat verification

Exact current warning ROM f92bc47131331b1eb168b5b668b6e14c7ebbe8edc6c3457bb1c97c8b8b1e6a2d
passes114 boss/Cloud checks: armor40, Jafar25, Marluxia30, Cloud recruit19,
including actual VRAM text for all three armor phases, spell, sword and
normal/enraged scythe. Original mechanics/save/resource checks remain. Combined
with58 UI checks,172 scoped checks pass on this exact ROM.

All-room script FAIL137348, native Sora defeat in Agrabah room10. Terminal
position61618,131108,36864; Cloud13/Donald5 HP; one enemy12 HP at projected
65536,141312,36864. Late policy alternated Cure/Guard and enemy turns without
finishing the enemy. Preserve this failure; investigate lethal-card selection
and navigation rather than treating it as a successful campaign or altering
health. Three-run session84597 is still live.

## Current warning-ROM complete campaigns and finishing decision

Three-run session84597 exited0 and PASS434168 on exact warning ROM: stable
victories129518/311898/434168 HP76/76/80; native retries129530 and311910.
Per-run actual Cloud recruit/deploy, exact suspend/reset and composed descent
verified; ROM/driver/log hashes finalized. Packaged0.37 development patch
applies byte-identically, manifest and CHARGE-WARNING-0.37.md record scope.

Replay now selects the lowest-value legal Fire that finishes the last enemy,
matching raw-plane range<128px, height<=24px, native floor card-break threshold
and Sora power. Actual native target/damage must confirm before playing.
It only overrides healing/Guard when the remaining enemy can be removed.
Read-only tests cover lethal prediction, native confirmation, range, height,
breaks and multiple enemies; all15 policy files pass. Fresh all-room session
92308 is pending; prior native defeat137348 remains retained. No game health,
damage, save or movement rules changed by this driver policy.

## Current party/SRAM/Rally review and remaining all-room failure

82 current native checks pass: party44 (native deployment added and bounded
preview-update wait), SRAM12 including damaged-newest fallback, Rally input-
only action14 and complete state fixture12. The first party attempt had one
preview assertion before a native update; later revival already passed. It is
retained. Rally750-frame attempt covered only casting; complete1100-frame run
includes actual damage/climb. Counts/hashes finalized; current scoped total254.

Finishing-Fire all-room92308 ended in identical native defeat137348 and exited0.
Decision-time diagnostic50504 also failed137348. Forecast logs show no eligible
finishing choice near the last turns. Recorded terminal state in a read-only
memory mock predicts hand slot1, enemy2, HP12; this is model diagnosis only,
not native proof. New diagnostic logs explicit selection-rejection reason and
controlled party, session30946 live. Preserve failures; do not claim resolution.

Decision-reason replay30946 script FAIL137348. At135585 (HP50) and136589
(HP26), finishingFireSlot explicitly rejects multiple living enemies while
Sora is controlled. The lone-enemy terminal snapshot does not describe those
earlier decisions. Next diagnosis must inspect live target choice and incoming
damage before terminal cleanup, not assume a guaranteed last-enemy kill.

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

Release instructions refreshed for0.38: removed obsolete0.19/0.20/0.32 front-page claims, documented compact UI and exact current three-run evidence, aggregate540000-frame command and final-log/hash/exit requirements. BPS commands use current0.38 development version/name. Four Python patch tests PASS; native source unchanged. All-room81924 live at186038.

Controls guide corrected stale chronological claims: current save12/migration8–11, implemented assembly/recruitment/Rally states and verified three-run coverage. Native Agrabah/Jafar and Castle/Marluxia captures inspected; compact top HUD leaves scene center visible. All-room81924 live at218382, no terminal result yet.

Current0.38 all-room81924 PASS253238 and terminal exit0 confirmed. Exact ROM/driver/log hashes verified; all36/nine chests, actual Cloud/redeploy, exact suspend/reset and composed descent required by input-only driver. Current540 scoped checks and three-runPASS379646 remain verified. Next concrete gameplay gap: Jump menu explicitly says NO LANDING PREVIEW; moving jump spends one movement/action and uses original physics. Requested reachable overlay still lacks jump destination/connection prediction. Implement faithful prediction from native collision/velocity/controller limits, verify actual landings/obstructions/heights/cancel/costs; do not replace with guessed guaranteed tiles. Final goal remains active.

Jump predictor groundwork: optional read-only --trace in native Jump menu test records actual positions/ground/busy/direction/budgets once per native frame. Original source confirms five-frame startup, speed halving then doubling,66 gravity and512 airborne cap, controller32px weighted horizontal bound. Native input checks PASS10 with317 unique-frame samples/55 moving-jump samples; trajectory starts45491,75776,36864 and ends53602,75776,36864 (31.68px actual travel). Exact hashes/exit0 verified in jump-native-trajectory-evidence. This is an oracle for future predictor comparison, not an implemented landing preview.

Jump motion model added in field_jump.c/h, C89 and allocation-free, seeded after native ground commit. It follows original five-step startup,1331 ascent,66 gravity,17 acceleration,512 cap and tactical weighted32px travel stop. Sanitized host comparison matches all46 recorded motion samples through landing for the rightward native fixture. Initial17-unit overshoot exposed missing held-direction release at bound; corrected. Comparison fixture explicitly seeds recorded origin45056/75776 and initial speed435; no broader directions/terrain proof. Model is not yet linked into native UI: collision, props, ledge/stair attachment and initial ground direction/turn adapter remain required. Current0.38 native ROM unchanged.

Jump motion comparator no longer seeds a hardcoded origin/speed: read-only trace now includes actual actor speed/angle and original s16 sine/cosine table values. Previous sample supplies origin; first native busy sample supplies committed speed. Fresh native input checks PASS10 and sanitized model matches46 samples through rightward landing. Exact hashes/exit0 verified in jump-native-speed-trajectory-evidence. Other directions/initial-facing transitions and collision/height cases remain required; model still not native UI-integrated.

Eight-direction native jump traces now captured input-only via --direction (native Start deployment and B+D-pad jump), all frontend exits0 and exact hashes verified. Generic comparator PASS46 each for16/right,128/down,80/up-right,144/down-right,160/down-left (230 samples). It FAILS32/left atframe64,64/up at57,96/up-left at59 where native geometry blocks/slides movement. Exact failed comparisons retained. This confirms free-air model alone cannot safely promise landing markers; next adapter must use native terrain/collider footprint and sliding resolution. Current packaged ROM unchanged; no UI prediction claim.

Jump collision response implementation: FieldJumpMotionWall restores pre-step X/Y and applies native230/256 damping except falling over4095 above ground (ledge probe case). Sanitized assertions reproduce actual leftward frames64–66 at x40094: z27261/26524/25853, speed460/428/399, plus source-defined falling height cases. Existing46-sample free-air trace comparison still passes. Collision detection and stair/ledge branching remain native-adapter work; wall unit test verifies response from explicit contact inputs, not independent collision prediction. No new ROM/UI release.

Jump terrain adapter groundwork: callback-based FieldJumpTerrainCheck matches original FldSoraCheckBlocked two ±1536 front/back probes, raises each local ground before blockage test, chooses lower support only when both clear. Sanitized host tests cover footprint/support and blocked-state isolation. Callback design keeps predictor local without swapping live actor globals. Actual native GetFldPosGround/IsFldPosBlocked binding, generated-room blocked trajectory verification and collider/attachment handling still required; no landing UI claim and packagedROM unchanged.

Live native jump terrain diagnostic integrated: field_jump linked, local predictor uses original ground/block probes and recorded ground-start speed/angle semantics. Nearby veneers added for GetAngleDiff and agbcc indirect r9 calls. Native Jump input checks PASS12, including exact pre-commit predicted X/Y/Z versus actual landing; exit0/hashes verified, ROM 153b3c05cb12cbe2cd8314780227cf3d40cfb85e2017f1a28246629f4dae6b00. Diagnostic globals fit linker RAM bounds. Blocked terrain returns unresolved; collider/platform/stair/ledge cases and wider directions still unverified. No visible landing marker or new package; previous0.38 540/campaign proofs remain scoped to packaged2d438 hash.

Live terrain predictor eight-direction native input verification PASS32, hashes/exit0 confirmed in jump-live-forecast-{direction}-evidence. Resolved16/128/80/144/160 match actual landing exactly;32/64/96 deliberately unresolved due terrain block (not complete collision prediction). Preview preserves budgets. Derived identical ROM copies removed after verification for disk space; scripts/saves/logs/metadata retained with canonical current build. Next implement blocked rollback plus original stair/ledge branch detection and colliders; do not consider unresolved checks a complete Jump requirement.

Native jump terrain collision prediction now applies wall rollback/damping while excluding actual stair/ledge attachment candidates using original climb-direction/over-under probes. New veneer GetFldPosClimbDir. All eight native menu forecasts resolve and match actual landing X/Y/Z exactly, PASS32 with budgets preserved, hashes/exit0 verified; eight jump-wall-forecast evidence dirs. Derived ROM duplicates removed after verification, canonical current build retained. Prop/collider support and attached cases not covered; diagnostics remain invisible pending that validation. Previous0.38 package remains unchanged/version-scoped.

Native diagnostic predictor adds read-only scenery pool6 contact calculations: reverse active order, fixed-point circles using original Sqrt8/GetAngle, platform-top support and original230/256 push/damping. Four-argument angle veneer preservesr3. Canonical live collider fields are never modified. Clear Jump regression PASS12, exact hashes/exit0 verified; current ROM eac76d5155a700766f3900e04b7b705cdc4f11daaead095a3b8dbaa90ecde9ea. Targeted prop-top/push fixtures still required before visible marker, and multiple overlapping platforms require checking original precedence. Current full campaigns remain previous0.38 scoped.

Original pillar contact predictor PASS5 in jump-pillar-contact-evidence: saved Castle-room fixture from prop-party-final-evidence, explicit approach position and3move/1action budgets; subsequent menu/direction/confirm native input. Resolved forecast37533,83968,0 equals actual native pillar-top landing exactly. ROM/driver/log/seed-save hashes verified, frontend exit0. Portable generator requires disclosed --saved-room input; side contacts/overlapping props/attachment cases still required. This scoped prop support proof does not complete landing UI.

Tall prop side-contact fixture increases original pillar collision height16384 (disclosed), uses native menu jump after initial approach/budget placement. Initial prediction45914 vsactual46081 failed and retained. Native landing animation continues collider pushes after first ground contact; predictor now simulates eight original landing updates with zero movement speed. New tall-side-settle replay PASS5, exact predicted/actual46081,83968,8192, hashes/exit0 verified. New ROM not packaged; rerun clear/pillar cases, overlapping/platform/attachment cases still required before UI.

Collider precedence corrected to track last platformZ in reverse pool traversal, including colliding platform contacts, then native ground/platform minimum only if standFlag is set. Pillar-top and tall side-contact regressions PASS10 on new exact ROM, hashes/exit0 verified. Attempted second-original-platform overlap fixture fails setup because no matching second static platform in saved room; retained FAIL, no overlap proof. Need inventory/explicit synthetic overlapping-collider fixture before claiming precedence validated. New configurable --overlap fixture preserved. No visible landing UI/package or broad campaign claim.

Source audit corrects collider traversal: original ColliderCheckPoolPairs processes obstacle poolA forward, player poolB reverse; predictor now obstacles forward. Synthetic overlap fixture promotes an existing obstacle to platform, relocates associated work/collider and sets differing height (explicit test writes). New run FAIL2: diagnostic resolved-status not1 at sampling frame, expected original pillar top wrong for second platform; predicted/actual37775,83968,4096 match but cached coordinate equality is insufficient proof. Preserve failure and investigate prediction publication timing/fixture support expectation before claiming overlap pass. Native source changed; prior prop regression proofs version-scoped.

Overlap diagnostic identifies prediction0 transient during active recomputation (action1/busy0/menu6), not failed physics. Publish status only when completed; clear on inactive state or direction change. Fixture now expects explicitly set second-platform support, not original pillar top. Base5/tall5/synthetic overlap6 PASS16, exact native landing37533,83968,0 /46081,83968,8192 /37775,83968,4096; hashes/exit0 verified. Earlier failures retained. Current predictor still needs attachment/height/culling/dynamic cases and full native regression before visible landing marker/package.

### Jump marker and compact UI verification

Resolved native jumps now draw a reach diamond at the predicted field landing position, with a short status line inside the existing Jump menu. Unresolved routes withhold the marker, including prediction timeouts. No extra HUD rows or persistent panels were added. Idle HUD remains 34/160 pixels, with the ready-party strip followed by enemy phase.

Current native ROM SHA256: `2eb777c691dbcbff373d303eb8f2a9908c78d389ea6b7deb2a689d32f313ba23`. Eight input-only direction fixtures passed 40 checks, including actual VRAM status text and exact final landing coordinates. Three disclosed saved-room collider fixtures passed 16 checks (original pillar, modified tall collider, synthetic overlapping platform). Screenshot inspected for the rightward landing preview. Evidence: `build/tactics-us/jump-marker-*-evidence`. These 56 scoped checks do not replace the separate 0.38 campaign proof or establish every attachment/culling case.

### Attachment-aware jump status

Native jump forecasts now distinguish stair attachment (status3, ATTACH TO STAIRS) and ledge catch (status4, CATCH LEDGE) from unresolved landing; these transitions withhold the guaranteed landing diamond. Stair preview native fixture passed12, including actual VRAM text, unchanged budgets, subsequent native attachment and next-turn one-level climb. Evidence jump-stair-preview-verified-evidence; ROM SHA256 `35953e2f0493cfd1e6e1c14bef9cda3d10c1e7974210c569ca6f0ca955349572`. Fixture explicitly relocates the approach and enemies; all command actions use native inputs. Three earlier setup attempts are retained: immediate/offset approaches failed before assembly acknowledgment; corrected assembly driver passed11 before adding VRAM assertion. Ledge label branch still needs native runtime verification. Pure jump terrain/wall checks now run in the normal host suite, which passed.

### Native ledge-catch forecast verified

Saved ledge approach fixture passed7 on the current attachment-preview ROM: direction/menu, forecast classification, actual VRAM label, unchanged resources, predicted catch, native hanging state, and Select opening the Climb/Drop menu. Busy2 intentionally persists while hanging to own the unfinished native jump; the command menu remains accessible. Evidence jump-ledge-preview-verified-evidence; source save hash, ROM/driver/check hashes recorded. Initial setup attempts and incorrect busy0 expectation retained separately. Generator --ledge-save now reproduces the explicit saved approach with conditional setup acknowledgment and native input only after boot. This verifies one native ledge approach, not arbitrary geometry.

### Donald area recipe boundaries

Current attachment-preview ROM passes12 native Donald area recipe checks: native assembly selection, Fire exact144px/24px boundaries and excluded range/height cases, unchanged melee64px boundary, two-target36-point specialty preview and exact execution, stock/action clearing. Initial fixture had one expected-value substitution error (32 versus36 on first target); retained independently. Corrected evidence attachment-ui-donald-area-corrected-evidence hashes/count/exit0 verified; explicit card/enemy HP/positions, native stocking and execution. Three-run attachment-ui-three-runs-evidence is still live; first native victory130609 and retry verified130741, so full aggregate completion is not yet proved.

### Four-caster native area matrix

Current ROM passes48 native area recipe checks: Sora, Donald, Goofy and Rally each12, native assembly selection, exact range/height inclusion/exclusion, two-target prediction and execution, resource/stock clearing. Explicit cards/enemy HP/positions; deployment/stocking/play use native input. Rally initial cycle direction selected Goofy instead and is retained as a failed setup fixture; corrected Down selection passes. Generator now uses one parameterized Lua fixture rather than brittle expected-value string rewrites. All four ROM/driver/check hashes and process exits verified. Hash-identical completed fixture ROM copies were removed to recover disk space; cleanup manifest retained, canonical ROM and live replay ROMs preserved.

### Cloud completes area caster matrix

Cloud area fixture passes12, completing60 scoped area checks across all5 heroes. Explicit Cloud unlock is applied after roster initialization; actual party deployment uses native assembly cycling, then native stock/play. An earlier unlock before initialization was overwritten and failed identity validation; preserved. Cloud fixture verifies mechanics, not recruitment acquisition (separate live fresh campaign covers recruitment). Current three-run script reports PASS379750, but frontend89724 still live to its540003-frame bound; terminal exit/hash audit required before promoting full proof. All-room3run frontend70148 remains live.

### Version0.39 development package

Three-run attachment-preview campaign PASS379750 verified after frontend89724 exit0; three victories,2 native retries,3 Cloud deployments,3 suspend requests and3 verified composed descents, exact ROM/driver/log hashes. Version0.39 BPS98,240B applies byte-identically to current ROM; manifest records79 exact-ROM scoped native checks. Native all-room3seed frontend70148 remains live and is not promoted as passed. README/build/current audit updated with version scope. Final release is still unproven.

### Multi-seed all-room stationary-turn diagnostic

Live all-room3seed frontend70148 completed first run then stopped moving in second-run Agrabah room11 at64930,50810,0 after frame458536, repeatedly ending turns with party73/64/56 and threat0. Process still live to900003; no restart or pass claimed. Replay driver now records read-only runtime/terrain/prop snapshots after8 consecutive turns at unchanged world/room/position, including budgets/menu/phase log. Short native room0-to1 smoke PASS2453, frontend exit0; it proves driver loading/traversal, not execution of the stationary snapshot branch. Existing live driver remains unchanged and will capture terminal failure snapshot at bound.

### All-room3seed terminal failure

Frontend70148 exited0; script FAIL900001 in second-run Agrabah room11, seed2658846982. Final54316,53034,0, door94208,43520,0; native busy0, move3, action1, menu/preview0, no enemies, HP73/64/56. This contradicts a stuck native command gate; explorer navigation/chest decision loop needs isolation. Exact ROM/driver/log/snapshot hashes verified; result markedFAIL and manifest/current docs updated. First all-room run completed, but three-seed sweep did not. Grounded screenshot/save-state/runtime evidence retained in attachment-ui-all-rooms-three-runs-evidence.

### Agrabah room11 chest collision approach fixed in explorer

Aligned fixture reproduces native-room11 left/right movement oscillation, not a blocked turn controller. The explorer required less than8px lateral separation before striking a chest; original chest/player collision prevented that side approach. Expanded attack approach to16px, retaining vertical/height/facing and action checks. Native chest count5→6 at342, reward confirmed, and original door exit to room10 PASS2042; frontend exit0 and exact ROM/driver/log hashes verified. Explicit saved seed2658846982/floor1/room11, captured78-byte deck/party/position, cleared encounters, five-chest initial count; all strike/reward/movement/door actions native input. Reusable fixture sources tools/tactics_agrabah_room11_fixture.c and tools/tactics_room11_chest_smoke.py. Earlier unmatched route-step and diagnostic/bounded broader-goal failures retained. This is scoped chest/exit proof, not full campaign completion.

### Chest-facing command hint

Sword confirmation now says FACE ENEMY OR CHEST inside the existing24px row, making native chest strikes visible to players without another persistent panel. Native build/link/header/capacity checks pass. PLAY facing persistence text corrected to current format12 and version-specific8–11 migration. This one-line native hint produces a new un-packaged build; live corrected all-room3seed frontend67315 continues using its frozen packaged0.39 ROM, so its eventual evidence remains version-scoped. No whole-game proof is inferred from this text edit.

### Periodic turn snapshots and corrected sweep progress

Replay diagnostics now snapshot every16 actually requested enemy phases, covering movement oscillation that exact-position counters miss. Known room11 narrow-approach fixture explicitly restored old8px policy in its driver: periodic snapshot executed and parsed, seed2658846982/floor1/room11, busy0/move0/action1; bounded reproduction endsFAIL6001 as expected, not a campaign pass. Evidence room11-periodic-turn-diagnostic-evidence with updated driver/log/snapshot hashes and exit0. Live corrected all-room sweep67315 temporarily spent many turns in first-seed room11 but subsequently progressed: all9chests, native victory298862, stable run complete298982, retry298994, then second-seed Agrabah entered389745. No terminal three-seed pass claimed yet.

### Repeat native sword damage and on-hit power

Native sword confirmation now shows on-hit power or explicit card-break warning in the existing32px row. Shared NativeMeleeBaseDamage supplies both UI and execution. A three-strike fixture exposed a real native combat bug: ROOM_FLAG_ATTACK_HIT stayed latched after the first physical sword hit, blocking subsequent hits. Clear this vanilla field-to-battle gate once when committing each sword attack, preserving within-strike hit semantics. Exact new-ROM Sora12 and Rally13 checks PASS25: native setup/menu/facing, actual VRAM labels, consecutive7/11 damage, zero-value/power8 behavior, low-value break with one action spent, exactly one break. Read-only native hitbox traces and ROM/driver/log hashes verified; both frontends exit0. Earlier failed menu/facing fixtures and unfixed trace retained. Explicit cards/upgrades/enemy HP/positions; deployment/commands/attacks native input. Exact hit-target geometry forecast still open.

Frozen0.39 corrected chest explorer sweep67315 exited0 with FAIL900001, second-seed Agrabah room10 at50465,123652,45056, goal45056,64000,0; native busy0/move2/action0. First all-room9chest run completed298982; second-seed room11 chest regression was cleared before this later height stall. Failure ROM/driver/log/snapshot hashes verified. New sword-hit fix is not covered by these older complete-run results.

### Original jump-pad connections resolve room10 height fixture

Captured terrain-only graph had no ascent route from region3/z45056 to door region4/z0. Native reproduction confirms3 original pads: x32768/rawY73728/z24576/height24576, x49152/rawY81920/z45056/height20480, x57344/rawY86016/z61440/height16384. Added read-only launcher ascent edges to the explorer region graph and require native jumpGmkHeight readiness before pressing B. Original collider/controller owns launches; no forced movement or geometry rewrites. Explicit captured seed/room/position/party/deck fixture, cleared encounters/opened previous chest; native two-pad climb reaches intermediate24576 and upper0 surfaces, then original exit to room5 PASS3351, frontend exit0, exact hashes. Evidence agrabah-room10-pad-exit-verified-evidence. Earlier fixture without pad edges fails12001; broader pad-edge fixture exits but later fails its room-specific route state after changing worlds, retained. Existing jump landing predictor does not model the special Gmk ascent physics yet and needs a guarded/native-tested pad classification plus eventual landing model.

### Repeat-sword native campaigns verified

Current native sword-hit-gate/on-hit-power ROM passes3 fresh campaigns371709; frontend46355 exited0. Exact ROM/driver/log hashes match and logs contain3 stable victories,2 retries,3 Cloud deployments,3 native suspends and3 verified composed descents. This covers the corrected per-strike native hit gate (unlike packaged0.39) and preserves version scope. Original jump-pad landing model is still open; all-room3seed routing with new pad edges needs a fresh sweep.

### Native launcher landing preview verified

FieldJumpMotion now mirrors the original Gmk launcher pull, rise and fall transition. Stationary launcher previews use the actual collider center, pad direction and target height; moving starts on launchers conservatively withhold a guaranteed landing. Three original Agrabah room10 saved pad fixtures pass24 native checks, including actual VRAM status, preserved preview budgets, exact final XYZ and one-action cost. Predicted/actual coordinates:39266,70441,0;42654,78633,24576;50846,82729,45056. Input-only after explicit saved position/party/deck fixture; setup acknowledges native pending assembly/reward. Initial setup failures retained. Ordinary Move/Jump regression passes12 on this same ROM, with exact landing and spent-action rejection. Frontends exit0; ROM/driver/log hashes verified. Host rules/save/worldgen/terrain/wall checks pass. Broad all-room three-seed sweep remains live on the predecessor sword-hit-gate ROM; it does not validate this new launcher model.

### Eight-direction sword aiming and third-seed sweep failure

Native sword confirmation now uses the same eight-direction mapping as field aiming and jump prediction, reading held input so a newly pressed second diagonal component retains the first. Sora17/Rally18 native input checks pass35; menu and budgets remain unchanged for all8 angles. Native consecutive sword damage/card-break regression passes12 on this ROM. Frontends exit0 and exact hashes verified. Initial fixture incorrectly read the actor u8 angle as u16; failed evidence retained, corrected byte reader passes. Six hash-identical completed ROM copies removed with cleanup manifest to recover disk space; canonical ROMs retained.

Original launcher-route all-room sweep80163 exited0: first two full campaigns complete300977/639960, native retries seeds2658846982/1018315455. Third-seed Castle floor2 room5 fails900001. Final original actor state6/CLIMB, ground36864; periodic snapshot shows busy0/M0/A1, last enemy HP16 on ground36864, Sora z16426 attached to stairs and companions knocked out. The original driver reverses directions around an intermediate goal height; this is not three-run all-room proof. Exact predecessor ROM/driver/log hashes verified and metadata records failure. Launcher landing and eight-direction changes need their own full-run verification.

### Castle stair endpoint regression resolved in native explorer

Corrected the earlier enum misdiagnosis: state6 is CLIMB; FALL is4. Source include/map/fld_types.h is authoritative. Isolated saved third-seed Castle room5 attachment with encounters cleared reproduces old driver policy reversing between stair bands near the intermediate goal/door height20480: one setup PASS/two endpoint FAIL. Correct policy compares goal height with the supporting bottom36864, commits toward a connected stair endpoint before routing onward. Native endpoint fixture PASS3 at original ground0; actual corrected explorer exits room5 to room6 PASS6632. Native inputs after disclosed saved position/deck/party/cleared-encounter fixture, no emulated-memory writes. Frontends exit0/exact hashes verified. Initial wrong-enum fixture and failed partial-ROM disk attempt retained. No native gameplay code changed for this driver correction. Full-room bound raised to400000 per run because recorded successful campaigns consumed300977/338983 frames; aggregate1.2M remains bounded. New current-ROM all-room/chest/Cloud/suspend/composed-descent3seed sweep7656 launched, scope pending.
