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
- Selectable Sora/Donald/Goofy with individual movement/action budgets and original sprites.
- Individual HP, friend knockouts, selection skipping and Cure/chest revival.
- HUD damage estimates using enemy range/height/guard rules, emulator checked.
- Enemy damage and positions persist across all visited rooms and reboot.
- Original Donald casting and Goofy guard animation resources.
- Discrete enemy decisions, health, value checks, height-limited attacks and stronger exits.
- Three-card native sleights, combined values, first-card exhaustion, stock cancellation
  and original stock artwork; emulator-verified save/reload behavior.
- Native twelve-card shared deck, five-card hand, original card/value artwork,
  discard/reload, Kingdom Key/Fire/Cure/Guard, character bonuses and chest cards.
- Exact native suspend snapshots in checksummed dual SRAM slots; emulator-tested
  reset and latest-slot corruption recovery.
- Bounded HUD font tile allocation; mutable ROM data is rejected by the linker.
- Original chest animation with one-time healing/card reward; safe placement fallback.
- Three-world progression, defeat, run-clear and retry with a new seed.
- Appended code and RAM, bounded Thumb hooks; original ROM assets keep their addresses.
- Asset-free rules/save/graph tests and local mGBA smoke/scenario replays.

## Remaining implementation sequence

1. **Height-aware tactical navigation.** Build a graph of traversable field
   surfaces from generated cells, stairs and climb/jump edges. Preview a route
   with the original projection; charge movement by route cost. Preserve
   terrain collision during animation. Validate spawn-to-door and chest
   reachability for every generated room, then regenerate invalid rooms using
   a bounded retry policy. Current graph tests verify room connectivity, not
   complete physical navigation inside each room.
2. **Authoritative field combat.** Extend discrete enemy decisions with bounded movement paths, blocked-axis
   recovery, world-specific behavior and collision/occupancy checks. Add telegraphed targets, occupancy rules and richer enemy behaviors.
   Independent party HP and partial encounter persistence are implemented. Keep animations
   running during input wait without advancing authoritative actions.
3. **Port the tested card systems.** Extend the native deck with target selection and richer named sleight recipes.
   The basic draw/discard/reload and card effects already run in the field.
   Remove assumptions about an 8x6 board. Add projected target/range previews
   and a GBA-sized card HUD using original resources.
4. **Roguelike content.** Add room roles, enemy groups, authored tactical motifs,
   reward choices, world-specific hazards, map-card modifiers and field bosses.
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
Physical hardware, physical route guarantees and an input-only three-world
run remain unverified. The polish goal remains active. Keep the matching US build
byte-identical and never include ROMs or extracted game assets in Git or patches.
