# Kingdom Hearts: Chain of Memories — Tactics Roguelike

## Goal and first playable

Build a GBA ROM hack from the matching khcom decompilation. Preserve the recognizable characters, cards, art and sound while replacing real-time battles with deliberate grid combat and replacing the story campaign with seeded, branching runs through Castle Oblivion.

The first playable is a debug-accessible encounter: Sora versus two Shadows on an 8×6 board. Move, choose a card, preview its target and damage, confirm, then resolve enemy turns. Victory and defeat return to a restart screen. This is the gate before procedural content.

## Initial design assumptions

These are editable defaults, not settled design constraints:

- Control Sora first; companions become card effects before adding a controllable party.
- Orthogonal movement, no diagonal movement. Occupied and blocked cells stop movement.
- Player phase: move up to three cells and play one card, in either order; pass ends the phase. Enemies then act in a stable order.
- Starter cards: adjacent Keyblade attack, ranged Fire, Cure, and Guard. Values change strength; card breaks and sleights follow after the core is playable.
- Draw a small hand from the deck each player phase. Played cards enter discard; reloading costs the card action. Define exhaustion and sleight penalties before introducing permanent card loss.
- Show enemy intent before committing a turn. Damage and target previews must match actual resolution.
- A short run initially spans three floors with branching battle, reward, rest, elite and boss nodes. Persist HP and deck between encounters; defeat ends the run.
- Rewards offer three cards or upgrades; map cards influence the next room's terrain or modifiers. Prefer unlocks over permanent stat inflation.
- D-pad selects cells/options; A confirms; B cancels; L/R cycles cards; Start opens pause/end-turn controls. Fit board and HUD on 240×160.

## Repository findings

Starting upstream revision: `7d2404fdd5838b4458e6582d9e1735f426d509fa`.
Fork: https://github.com/thomasjvu/khcom
Upstream: https://github.com/Pheenoh/khcom

The supplied ROM matches the US SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48`. Target US first; JP/EU support is a later port.

| Area | Existing implementation | Integration decision |
| --- | --- | --- |
| Mode dispatch | `src/mode.c`, `include/mode.h` | Add a separate tactics mode and debug launch path. |
| Battle initialization and tick | `src/btl/mode_battle.c` | Reuse only audited presentation setup; do not let legacy real-time actor updates resolve tactics combat. |
| Actor projection and resources | `src/btl/battle_runtime.c`, `include/btl/btl.h` | Adapt sprite presentation to board coordinates; keep authoritative positions in grid cells. |
| Tasks and animation | `src/taskpool.c`, `src/anim` references, sprite APIs | Continue rendering while waiting for input; run presentation after a committed simulation action. Verify actual animation interfaces before integration. |
| Cards | `include/card/`, `src/card/` | Introduce a small tactics card catalog mapped to original visual IDs. Avoid treating real-time combat definitions as tactics rules. |
| Input | `src/key.c`, `src/key_state.c` | Edge-trigger commands, cancelable previews, no actions on held input repeat. |
| Save | `src/save.c`, `src/save_data.c`, `include/save_types.h` | Audit capacity and checksums before selecting a versioned run-save layout. |
| RNG | `src/random.c` | Give run generation its own seed/state so animation cannot change room/reward outcomes. |
| Build | `configure.py` | Preserve original matching build. Add an explicit hack target later; modified ROMs cannot pass the original SHA-1 check. |

The decomp uses assembly and fixed-layout data as well as C. New modes need a linker/ROM-space audit, not simply adding a C file. EWRAM heap is currently configured as `0x34000`, IWRAM heap as `0x6800`; do not assume free space without inspecting the map and allocations.

## Ordered implementation milestones

### 0. Reproducible baseline

1. Fork and clone, retain upstream remote, isolate work on `tactics/bootstrap`.
2. Validate local ROM; install Python dependencies, ARM binutils/GCC, agbcc, pinned legacy assembler/linker/runtime and gbagfx.
3. Extract US assets, configure and build. Require byte-identical original ROM before engine integration.
4. Record host commands, tool revisions, hash and failure logs. Keep ROMs, extracted assets and binaries ignored.

Acceptance: a clean checkout plus local ROM builds the matching US image. Host simulation tests run separately without ROM assets.

### 1. Deterministic rules core (started)

Keep rules under `tactics/`, freestanding C89 with bounded storage, no allocation, no rendering and no GBA I/O. Implement board occupancy, path distance, movement/action budgets, damage, phase changes and win/loss. Then add intent planning and a compact command/event interface for rendering and replay.

Acceptance: occupied destinations and blocked routes are rejected; rejected commands change no state; movement and action budgets are independent; enemy actions resolve once; identical initial state and commands produce identical results. Compile with host warnings and sanitizers, then the GBA compiler.

### 2. GBA debug encounter

1. Audit mode registration, debug selection, VRAM/palette ownership and ROM layout.
2. Add an explicit tactics build mode with original checksum verification retained only for matching builds. Allocate new code/data deliberately and check ROM header/checksum and size.
3. Register `gModeTactics`; show an 8×6 grid, cursor, Sora and two Shadows using existing resources where practical.
4. Add movement/attack previews and enemy intents. Resolve committed events through animation, blocking further commands until presentation finishes.
5. Victory/defeat/restart; inspect in an emulator and record screenshots/input traces.

Acceptance: complete multiple encounters on the built ROM; no legacy AI movement, double damage, deadlocks or resource leaks after repeated restarts. Frame and memory budgets recorded from actual measurements.

### 3. Cards and combat identity

Add draw/discard/reload, Fire/Cure/Guard, then card-value comparisons and telegraphed card breaks. Introduce sleights only after specifying their costs and previews. Add two additional enemy archetypes with distinct movement and intents.

Acceptance: previews agree with simulation; deck conservation holds through draws/reloads; hand/deck limits enforced; every starter card has a useful role.

### 4. Roguelike run loop

Add independent seeded PRNG, connected branching floor graph, authored encounter templates with controlled randomization, persistent HP/deck, reward selection, rest and one boss. Map cards select a room modifier. Start with a complete short run before enlarging the content pool.

Acceptance: seeds replay room/reward layouts; every generated graph reaches a boss; no unwinnable reward screen or softlock; victory, defeat and new-run transitions reset the correct state.

### 5. Saving and release

Design versioned suspend saves with checksum and safe invalid-save recovery after auditing existing SRAM usage. Add seed entry, run summary and balance tuning. Build a source-ROM-validated BPS patch and publish source plus patch and instructions.

Acceptance: suspend/resume reproduces run state, corrupt/older saves recover safely, patch applies only to the supported ROM, and a full run passes emulator validation. Distribution contains no ROM or extracted game assets.

## Risks and decisions to resolve through prototypes

- Asset reuse may be easier in a new mode than inside the existing battle task graph. Prove rendering/resource cleanup in milestone 2 before committing to broad battle rewrites.
- Fixed addresses, incbins and link layout can make code growth expensive. Inspect linker script and ROM map before appending code.
- An 8×6 board plus cards may crowd the display. Validate screenshots on native resolution; use compact HUD or a separate card panel if needed.
- Phase combat, card breaks and party size affect each other. Playtest Sora-only before expanding mechanics.
- Do not reuse gameplay RNG for procedural content or save into presumed spare bytes.
- Existing upstream CI depends on an external build container and private ROM availability. The fork needs independent host-test CI; ROM builds remain local unless authorized inputs are configured.

## Immediate next task

Finish the matching US build, then add a separate tactics build target and debug mode displaying the board. The initial host rules prototype is scaffolding, not a playable ROM hack; porting and emulator validation are still required.
