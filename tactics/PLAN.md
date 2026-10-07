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

## Remaining implementation sequence

1. **Height-aware tactical navigation.** Extend the implemented flat-surface walking
   preview and native route execution with stairs and climb/jump edges. Validate spawn-to-door and chest
   reachability for every generated room, then regenerate invalid rooms using
   a bounded retry policy. Current graph tests verify room connectivity, not
   complete physical navigation inside each room. The input-only traversal driver
   now has `--all-rooms`: visit 0–1–2–3–4–9–8–1–2–3–4–5–10–11–10–5–6–7
   in each world, and require all twelve room bits before terminal PASS.
   The first Cloud/suspend replay hit its 180000-frame bound in Castle room 5
   after both earlier worlds traversed all rooms. This is incomplete coverage,
   not a room impossibility finding. The longer bound is now 300000 frames.
   `--collect-chests` requires nine native chest opens as well as all room bits.
   Card selection has read-only mock checks; controller-only opening remains
   pending in `all-rooms-facing-chests-evidence`. R+Dpad now faces the active
   character without movement, budget use or card cycling; 32 input-only native
   checks pass across all eight directions. Walking to face caused the earlier
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
