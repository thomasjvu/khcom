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
- Individual HP, friend knockouts, selection skipping and Cure/chest revival.
- HUD and projected original-digit damage estimates using enemy range/height/guard rules, emulator checked.
- Seeded world-specific enemy groups with ranged Red Nocturnes, Darkballs and Black Fungus.
- Large Body exit guardian with a charge/area-strike cycle, evasion, Guard interaction and saved windup state.
- Enemy damage and positions persist across all visited rooms and reboot.
- Original Donald casting and Goofy guard animation resources.
- Discrete enemy decisions, health, value checks, height-limited attacks and stronger exits.
- Walkable upper ledges above void, solid native prop collision sampling and live preview revalidation.
- Eight-direction local routes with one-point projected diagonals, native diagonal execution and quarter-segment geometry sampling.
- Player walk cursor projected with original value digits; exact route costs, budget rejection and native controller execution, tested with input-only mGBA replay.
- HUD renders original font glyphs in RAM and uploads during VBlank.
- Bounded 9×9 enemy route search with native floor/collision sampling, midpoint checks, height limits and occupied destination checks.
- Three-card native sleights, combined values, first-card exhaustion, stock cancellation
  and original stock artwork; emulator-verified save/reload behavior.
- Native twelve-card shared deck, five-card hand, original card/value artwork,
  discard/reload, Kingdom Key/Fire/Cure/Guard, character bonuses and chest cards.
- Exact native suspend snapshots in checksummed dual SRAM slots; emulator-tested
  reset and latest-slot corruption recovery.
- Bounded HUD font tile allocation; mutable ROM data is rejected by the linker.
- Original chest animation with one-time healing/card reward; safe placement fallback.
- Three-world progression, defeat, run-clear and retry with a new seed.
- Room/world transitions retain selected member, all party turn budgets and Guard; actual native forward/back door crossings are emulator checked.
- Appended code and RAM, bounded Thumb hooks; original ROM assets keep their addresses.
- Input-only default-seed three-world victory on the final 0.14 ROM, stable for 120 further frames: 79,338 frames, 17 kills, 680 movement commands and 24 Goofy Guards; no teleports, forced exits or emulated RAM writes.
- Asset-free rules/save/graph tests and local mGBA smoke/scenario replays.

## Remaining implementation sequence

1. **Height-aware tactical navigation.** Extend the implemented flat-surface walking
   preview and native route execution with stairs and climb/jump edges. Validate spawn-to-door and chest
   reachability for every generated room, then regenerate invalid rooms using
   a bounded retry policy. Current graph tests verify room connectivity, not
   complete physical navigation inside each room.
2. **Authoritative field combat.** Extend the bounded enemy route search with swept actor collision checks and additional world-specific behaviors. Extend current damage and charge warnings into full area/range overlays.
   Independent party HP and partial encounter persistence are implemented. Keep animations
   running during input wait without advancing authoritative actions.
3. **Port the tested card systems.** Extend the native deck with target selection and richer named sleight recipes.
   The basic draw/discard/reload and card effects already run in the field.
   Remove assumptions about an 8x6 board. Add projected target/range previews
   and a GBA-sized card HUD using original resources.
4. **Roguelike content.** Add room roles, enemy groups, authored tactical motifs,
   reward choices, world-specific hazards, map-card modifiers and canonical field bosses. The current Large Body guardian is an elite encounter with a charged attack cycle.
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
