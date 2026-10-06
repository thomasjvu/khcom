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
- Discrete movement/action budgets, enemy phases, field sword hits and contact damage.
- Original chest animation with one-time healing; safe placement fallback.
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
2. **Authoritative field combat.** Replace temporary legacy AI time windows
   with discrete enemy decisions and bounded movement paths. Give actors HP,
   card values, height-sensitive range and telegraphed targets. Keep animations
   running during input wait without advancing authoritative actions.
3. **Port the tested card systems.** Adapt the existing draw/discard/exhaust,
   reload, card breaks, Cure/Guard/Fire and sleights to field actors/surfaces.
   Remove assumptions about an 8x6 board. Add projected target/range previews
   and a GBA-sized card HUD using original resources.
4. **Roguelike content.** Add room roles, enemy groups, authored tactical motifs,
   reward choices, world-specific hazards, map-card modifiers and field bosses.
   Isolate run-generation RNG from combat and presentation RNG. Test optional
   paths, persistent opened chests, enemy clears and backtracking.
5. **Suspend saves and complete-run QA.** Serialize run seed, generated world,
   room state, actor surfaces, HP/deck and turn state in versioned dual slots.
   The board save format is not sufficient for field state. Add input-only
   full-run tests for victory/defeat, doors, climbing, reward choices, reset and
   corrupted-save recovery. Produce a verified BPS patch and release notes.

## Verification boundaries

Host tests cover the old rules core and 3,000 generated room graphs. Native
mGBA smoke checks use controller input for movement, attack animation and turn
budgets. Native scenario checks use explicit fixtures to place an enemy/chest
within range and request room transitions; they verify field hit/reward
handlers and lifecycle/progression, not a complete player-driven run.

Physical hardware, full native card combat, physical route guarantees and an
input-only three-world run remain unverified. Keep the matching US build
byte-identical and never include ROMs or extracted game assets in Git or patches.
