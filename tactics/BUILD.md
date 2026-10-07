# Building KH Tactics

## Prerequisites

Use a local US ROM with SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48`,
placed at `roms/B8CE.gba`. No ROM or extracted assets are included in the repository.
The local checkout is `/Users/area/kh-tactics`; `origin` is the fork and
`upstream` is Pheenoh/khcom. Work is on `tactics/bootstrap`.

macOS arm64 setup used here:

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt ninja
brew install arm-none-eabi-gcc
git clone https://github.com/pret/agbcc.git build/agbcc-source
(cd build/agbcc-source && git checkout da598c1d918402c42c0c0d7128ba14567f3175e9 && ./build.sh && ./install.sh ../..)
sh tools/fetch_gbagfx.sh
SSL_CERT_FILE="$(.venv/bin/python -m certifi)" .venv/bin/python tools/setup_legacy_toolchain.py
.venv/bin/python tools/extract_assets.py us
```

On Linux install the ARM GCC/binutils, C compiler, make, libpng development
headers and Python dependencies using your package manager, then use the same
agbcc, legacy-toolchain and asset-extraction commands. The upstream setup pins
binutils 2.10 and runtime source checksums. Install gbagfx before extracting assets.

## Two build targets

```sh
# Matching original game; verifies the original SHA-1.
.venv/bin/python configure.py --version us
.venv/bin/ninja

# Native field prototype; validates its header and ROM capacity.
.venv/bin/python configure.py --tactics
.venv/bin/ninja
```

Outputs: `build/us/com_us.gba` and `build/tactics-us/kh_tactics.gba`.
Run configure when switching targets; build.ninja is shared while object
files are separate. JP/EU tactics targets are explicitly rejected.

The tactics boot pointer lives in the existing startup assembly. All original
code/data remains at the same addresses; tactics code follows it at
`0x09efbfdc`. Nearby Thumb tail-call veneers bridge calls to the original
runtime outside the Thumb BL range. These veneers must use the historical
assembler, just like the C output. New globals occupy `0x0203e000` onward.
Linker assertions bound ROM growth and prevent EWRAM overlap/overflow.
The native target loads original sprite/tile resources directly; it does not
resample actors into bitmap icons. `tactics/gba.c` is the archived board renderer
and is excluded from the current target. `tools/tactics_native_hooks.py` installs
bounded eight-byte Thumb entry hooks after linking. It validates original
function size/alignment and appended target ranges. The normal build has no hooks.

Private US key variables remain at `0x02034000`, `0x02034002` and `0x02034004`.
The native controller samples hardware edges independently, then injects only
the committed action into the original field tasks. The generated room graph
and placement/reward handlers are isolated from original story/battle transitions.

## Host checks

```sh
sh tools/test_tactics.sh
python3 -m unittest discover -s tests -p 'test_tactics_*.py'
```

Rules use C89 and bounded storage. Byte-only card, actor and intent records
are explicitly packed because agbcc otherwise rounds their sizes to four
bytes. Save fields are serialized explicitly with little-endian seed/state,
a versioned header, generation counter and CRC32. The native format uses
two 1,024-byte SRAM slots; the archived board format uses 512 bytes. The
previous valid snapshot survives while the next is written. Decode keeps
one bounded candidate on the GBA stack, rather than two complete world states.

## Native emulator verification

```sh
python3 tools/tactics_native_smoke.py build/tactics-us/kh_tactics.elf build/tactics-us/native-evidence
/Applications/mGBA.app/Contents/MacOS/mGBA --script build/tactics-us/native-evidence/smoke.lua build/tactics-us/kh_tactics.gba
/Applications/mGBA.app/Contents/MacOS/mGBA --script build/tactics-us/native-evidence/scenarios.lua build/tactics-us/kh_tactics.gba
```

The generator reads diagnostic addresses from the linked ELF. `smoke.lua`
uses input to check held-button behavior, native movement, sword animation,
action budgets and enemy phase completion. `scenarios.lua` deliberately uses
fixtures to place a target/chest within range and request room transitions. It
checks enemy removal, chest opening/healing, world progression, run clear and
retry. Inspect the PASS/FAIL logs and screenshots in the selected directory.
Scenario fixtures are not an input-only complete-run test.

The older `tactics_emulator_trace.py`, save and defeat scripts target the
archived board alpha; do not run them against the native field target.

## BPS patch

```sh
python3 tools/tactics_patch.py create roms/B8CE.gba build/tactics-us/kh_tactics.gba build/release/kh-tactics-0.14-terminal.bps
python3 tools/tactics_patch.py apply roms/B8CE.gba build/release/kh-tactics-0.14-terminal.bps build/release/kh_tactics_field.gba
```

Creation verifies the supported input SHA-1 and a byte-exact application
round-trip. Application verifies source, target and patch CRC32 values.
Distribute source and the patch, never the ROM or extracted assets.

## Recorded verification, 2026-10-07

- Original matching US ROM remains byte-identical.
- Old rules/save core passes strict C89, ASan/UBSan and 30 deterministic runs.
- New room-graph generator passes 3,000 seed/floor combinations: determinism,
  reciprocal doors, connectivity and parameter bounds.
- Native mGBA input smoke passes movement stopping, budgets and field animation.
- Native fixture scenarios pass enemy hits, chest reward, transitions between
  three world asset sets, run clear and retry.
- New target BPS patch reconstructs the built ROM exactly.

Native party/deck/save modules pass strict C89 and ASan/UBSan. Save tests
flip every record byte, verify older-slot fallback and generation wraparound.
Party mGBA replay covers selection, individual budgets and HP, card effects,
reload costs, knockouts/revival, incoming damage warnings, reset restoration and corrupted-slot recovery.
Encounter replay checks exact positions and partial damage during backtracking
and after reboot in multiple visited rooms. Enemy coordinates are whole native
pixels, so signed pixel records preserve their 8.8 values exactly. Generate it with:

```sh
python3 tools/tactics_party_smoke.py build/tactics-us/kh_tactics.elf build/tactics-us/party-evidence
/Applications/mGBA.app/Contents/MacOS/mGBA --script build/tactics-us/party-evidence/party.lua build/tactics-us/kh_tactics.gba
```

Generate encounter fixtures with `tools/tactics_encounter_smoke.py` and run
its `encounters.lua` with mGBA. Fixtures explicitly request transitions and
set partial damage; they do not replace input-only route testing.

Run integration replays against a fresh copy of the ROM with its own filename
and save file, so an existing suspend does not change the starting state.

The native version still needs climb/jump route and
attack intent previews, physical reachability guarantees, distinct bosses, named sleight recipes,
input-only complete-run QA and hardware validation. The board alpha's earlier
emulator results do not establish these features in the native target.

Native sleight replay: generate with `tools/tactics_sleight_smoke.py ELF OUTPUT`,
then run its `sleights.lua` in mGBA against a fresh ROM copy. It verifies
stocking, save/reset recovery, field effect, exhaustion, reload and cancellation.

Native enemy routing uses a bounded 81-node workspace in reserved tactics RAM.
The strict C89 sanitizer suite covers obstacle detours, impassable height changes,
low stairs and enclosed actors. The 28-check party replay passes with routing
enabled (`build/tactics-us/route-party-evidence/party.txt`). Native sampling
checks endpoint and midpoint collision and rejects occupied destinations; this
is not yet a continuous swept collision test or a player route preview.

Player route replay: generate with `tools/tactics_route_smoke.py ELF OUTPUT`
and run `routes.lua` with mGBA against a fresh ROM copy. This replay uses only
button input (no state writes) and verifies cursor cancellation, exact one- and
two-segment costs, native controller arrival and rejection of routes beyond
the movement budget. The preview uses the original card-value digit sprites.
HUD glyphs now render from the original font into heap buffers, with a
VBlank upload into bounded BG0 tiles and tilemap.

Navigation build verification (2026-10-07): the input-only route replay passes
27 checks (`build/tactics-us/navigation-release-route-evidence/routes.txt`),
the party replay passes 28 checks, and world-transition scenarios pass 13.
Screenshots confirm the buffered HUD remains readable in Agrabah and Castle
Oblivion. The upload runs in the mode VBlank callback before audio mixing;
a late callback defers its upload instead of writing during visible scanout.

Encounter build: `tools/tactics_boss_smoke.py ELF OUTPUT` generates `boss.lua`.
The 16-check mGBA fixture verifies the Large Body guardian’s health, charge
warning, no windup damage, area evasion, Guard mitigation, charge persistence
through reset and defeat through a real Fire card. This is an elite guardian
using original Large Body art, not Darkside or another canonical boss.
The seeded enemy policy is tested across 3,000 seed/world combinations. Save
format 7 stores all seven native field identities and the guardian charge bit;
serializer tests cover higher kinds, charged round trips and invalid phases.

The 0.7 build also passes the 28-check party replay, 27-check input-only walk
replay and 13 world scenario fixtures. Local evidence is under
`build/tactics-us/guardian-evidence`, `encounter-party-evidence`,
`encounter-route-evidence` and `encounter-world-evidence`. These fixtures
remain narrower than an input-only complete run.

Climbing build: `tools/tactics_climb_smoke.py ELF OUTPUT` creates `climb.lua`.
It finds an actual generated stair base, places Sora there as an explicit
fixture, then drives the original controller with buttons. Its 19 checks
cover one-segment held input, ascent/descent costs, exhaustion, safe selection,
attached suspend/reset with exact vertical position, turn renewal and native
drop/landing. Save format 8 stores the stair target and facing. This does not
prove every generated room is traversable or add climb edges to the route cursor.

Turn persistence: `tools/tactics_transition_smoke.py ELF OUTPUT` generates
`transitions.lua`, with 17 checks for selected member, per-member budgets,
Guard, health, backtracking, suspend/reset, phase renewal, world advancement
and fresh-run initialization. Room changes are explicit fixtures.
`tools/tactics_door_smoke.py ELF OUTPUT` generates `doors.lua`, with 12 checks
that place Donald in front of actual generated doors and then cross them
forward and back through native diagonal movement, collision and door lookup.
It never writes room-transition flags. Neither test is an input-only full run.
The 13 world scenarios now end the turn in the reward room before opening its
chest, since entering that room no longer restores a spent action. Save format
8 remains compatible with the 0.8 climbing development build.

Input-only traversal probe: generate with
`python3 tools/tactics_traversal_probe.py ELF OUTPUT [--rooms 1..7]`, then run
`OUTPUT/traversal.lua` in mGBA against a fresh ROM copy. The bounded explorer
reads original door/cell geometry and route diagnostics, and sends buttons only.
It scans flat route previews, approaches generated stair tops/bases, climbs or
descends using native input, renews spent movement through Start, and uses the
original diagonal door approach. It never writes emulated RAM or forces exits.
A failure reports the final position and door target; it does not prove a room
is impossible. The default first-door replay passed in 965 frames and ten
movement commands, including both height changes from the actual spawn
(`build/tactics-us/traversal-door-evidence/traversal.txt`). This is one fixed
seed, not a generated-room guarantee or a complete combat run.
The earlier 0.9 `--rooms 7` probe reached room one, then exhausted its 12,000-frame
bound there (`build/tactics-us/traversal-chain2-evidence/traversal.txt`): final
projected position `(74873,62597,0)`, forward door `(12288,91648,12288)`, in
native 8.8 units. Local flat previews plus greedy stair selection are therefore
insufficient for the main chain. Global platform planning and jump edges remain
required; do not count this replay as a complete-run pass.

Diagonal navigation: `tools/tactics_diagonal_smoke.py ELF OUTPUT` generates
`diagonals.lua`. All 18 input-only checks pass: four diagonal cursor directions,
cancellation, budget preservation, one-point diagonal cost, native arrival and
unchanged combat action. The 27-check walking replay and 28-check party replay
also pass with eight-direction search and quarter-segment geometry checks.
The traversal driver now uses a wider read-only floor search, low-ledge
endpoints and native card input against nearby enemies. Its room-one stall
persists; these changes do not establish room-wide physical reachability.

Surface navigation: `tools/tactics_ledge_smoke.py ELF OUTPUT` generates a
seven-check upper-ledge fixture. It finds original generated geometry with a
finite upper floor and void below, places Sora on an adjacent floor, and checks
preview cost, native arrival, height and resources. With `--props`, the same
tool generates an independent eight-check prop fixture. It isolates one native
solid prop by temporarily suppressing other colliders, places Sora beside it,
checks blocked commit/resource preservation, then removes its collider while
the cursor remains open. The preview must update to a two-point route and the
native controller must reach the destination. These are explicit fixtures,
not input-only complete-run evidence. Original cell/prop artwork is retained.
Open previews now rebuild from the current actor and collider state; props can
be enabled or broken by native tasks between inputs. Both player and enemy
routes sample solid obstacle colliders using native radius, doubled world Y
and height overlap. Upper ledges are validated by their sampled standing
surface rather than rejected solely because their lower surface is void.
The input-only walking replay scans all eight directions for a two-segment
route, since solid props can correctly block its former cardinal candidates.

Surface build verification (2026-10-07): 123 emulator checks pass across
walking (27), diagonals (18), party (28), upper ledge (7), prop removal (8),
stairs/suspend (19) and guardian (16), under `build/tactics-us/surface-final-*`.
The strict host suite and four BPS tests pass; the 21,046-byte 0.11 BPS patch
reconstructs the built ROM byte-for-byte. Save format remains 8. The final
input-only chain probe still stalls in room one at native projected position
`(67869,61916,0)`, with three enemy kills; its final actor has no contact
collision. Global climb/jump planning and complete-run verification remain
unfinished. The prop fixture suppresses neighbouring/enemy/player contact
colliders to isolate route geometry, and holds the selected collider removed
because native prop tasks can re-enable colliders on subsequent updates.

Tactical jumps: `tools/tactics_jump_smoke.py ELF OUTPUT` generates `jumps.lua`
with 11 input-only checks for full-height ascent after early release, useful
movement from rest, bounded weighted world-space travel, native landing and
exact movement/action costs. `--stairs` generates an independent eight-check
position fixture for a jump catching an original tall stair. It verifies the
handoff releases the command gate, clears the airborne flag, retains costs,
renews the turn and allows a normal one-level movement-only climb afterward.
The native command-boundary replay passes 17 checks and the existing attached
stair/save replay passes 19 checks on this build (55 current emulator checks).

Optional native decorations now retain their original footprint and sparsity
requirements when a spot finder fails. The initialized-position fallback is
limited to mandatory base-floor objects (finder 13); large decorations can no
longer fall back to a single floor cell and crowd generated routes.

The input-only traversal probe now supports `--frames 180..120000` (default
36000), `--rooms 1..7`, and `--worlds 1..3`. It records native jump attempts,
uses underlying map ground when standing on a prop, and can target the exit
encounter before attempting world advancement. A fresh fixed-seed run crossed
all seven main-path doors from the actual spawn to room seven in 33,810 frames
with six enemy kills (`build/tactics-us/jump-ground-chain-evidence/traversal.txt`).
This proves that main-path traversal for one seed, not all generated rooms,
chests, the exit encounter or a complete three-world run. World-advancement
probes remain separate from the successful room-seven traversal. After the
decoration fix, `footprint-world-evidence/traversal.txt` passes one complete
world in 23,479 frames, with five kills and 178 movement commands. It crosses
all seven doors, clears the exit encounter and takes the original world exit
using buttons only. Current post-fix jump (11), jump/stair (8) and native
command-boundary (17) checks pass, alongside the strict host and four BPS tests.
The 13 world/chest/retry/defeat scenario fixtures also pass after the fix
(49 current emulator checks total). The 21,260-byte 0.12 patch reconstructs
the ROM byte-for-byte. The three-world input-only probe reaches Agrabah room
four, then exhausts its 120,000-frame bound: Sora `(124262,83519,8192)` and
door `(20480,83456,8192)` are on separate same-height platforms. The local
driver does not plan an intermediate height change between those platforms.
Evidence: `footprint-three-world-evidence/traversal.txt`. This is a navigation
failure, not a demonstrated impossible room; complete-run verification remains
pending.


Door synthesis regression: the original `MapDoorWaitHit` callback accepts sword
hits even on already open ordinary doors. That starts a room-card UI which the
native tactics update does not run, leaving `FIELD_FLAG_ROOM_CREATE` set and
Sora in `FldSoraWaitRoomCreate`. The tactics-only hook now returns idle from
that callback. Door graphics, open flags and native walking transitions remain
original; the seeded graph owns room creation.

`tools/tactics_door_hit_smoke.py ELF OUTPUT` generates a 14-check fixture. It
moves an original open door object beside the actual spawn, moves enemies out
of sword range, and uses A to run the native sword animation and hitbox. It
checks synthesis stays closed, controller recovery, exact action cost and room
identity, restores the door object, then crosses its actual map exit using
movement input. This is an explicit position fixture. In
`door-object-old-evidence`, restoring only the original callback's first eight
ROM bytes reproduces five failures, including synthesis opening and subsequent
travel failing. The same fixture passes all 14 checks with the hook in
`door-object-hit-evidence`.

The read-only traversal driver now labels connected terrain regions separately
at each height and builds directed transitions from original wall tops/bases.
A shortest region path can climb and descend between disconnected platforms
at equal height. Local native walking previews still validate actual commands;
this global planner is emulator QA infrastructure, not yet a player route
preview. It uses no emulated RAM writes. `lua
tests/tactics_traversal_regions_test.lua` checks disconnected equal-height
regions, cycles, one-way drops and selection among equally short connectors.
The driver reports script exceptions and verifies the active native mode and
update function so an original-menu transition cannot count as a run pass.


Tactical spawn regression: vanilla airborne spawns enter at fixed `z=-160`
for entrance AI which tactics intentionally does not run. Fresh Red Nocturnes
and Darkballs could remain unreachable by Fire and local enemy movement.
Native spawns now begin at their assigned ground. Cached format-8 actors of
these two kinds at exactly the old ceiling entrance are repaired on restore;
other saved positions, horizontal coordinates, standing surface and damage
remain unchanged. No save-format change is required.

`tools/tactics_spawn_smoke.py ELF OUTPUT` generates six checks: the seeded
entry encounter contains both actors on their floors, includes a Red Nocturne,
and a real selected Fire card defeats it, awards one kill and spends an action.
The fixture places Sora next to the actor and moves the other enemy away. All
six pass in `ground-spawn-evidence`; the old ceiling build fails three in
`ceiling-spawn-old-evidence`. `--legacy` constructs an old ceiling encounter,
writes a real suspend with Start+Select, resets the emulator and verifies the
repair retains the original horizontal coordinates and partial damage. It
writes `legacy-spawn.txt`.


Final 0.13 verification: 43 emulator checks pass on the final ROM: native
commands (17), door hit/travel (14), fresh spawn/Fire (6), legacy suspend repair
(6). Evidence directories are `encounter-final-native-evidence`,
`encounter-final-door-evidence`, `encounter-final-spawn-evidence`, and
`legacy-spawn-repair-evidence`. The 21,328-byte BPS patch reconstructs the
final ROM byte-for-byte. The strict host suite, four BPS tests and region-graph
Lua checks pass. Twelve forward/back native door checks and ten encounter
persistence checks also passed before the narrowly scoped legacy repair.

The ceiling-spawn fix changes actual combat: the input-only single-member
replay reaches Agrabah room seven, then loses at frame 44,941 with nine kills
(`tactical-spawn-three-world-evidence`). A new driver policy uses native threat
previews, selects Goofy, plays an available Guard, ends the turn, waits for the
real enemy phase and returns control to Sora. It skips unavailable party
members and preserves native attached-stair input. `lua
tests/tactics_traversal_party_test.lua` checks that input policy against a
read-only memory mock. This is not a native gameplay rule change, damage
adjustment or a complete-run pass.


**Input-only three-world victory (final 0.13 ROM):**
`party-guard-three-world-evidence/traversal.txt` passes at 79,150 frames with
17 kills, 680 movement commands and 24 real Goofy Guards. The replay crosses
all three native worlds, clears their exit encounters and reaches native
`gNativeFloor=3` / `gNativeResult=2`. It never writes emulated RAM, teleports,
changes enemy HP or forces a transition. Its ROM SHA-256 matches the final
release exactly:
`b62b7dd78a498760caee1fab0f8fc4b4c79540af3fa2ff575d5e352cdabf45d8`.
This proves one default-seed victory path. It does not cover optional branches,
input-only chest opening, other seeds, suspend during the full run or physical
hardware. Those scopes remain separate from the passing explicit fixtures.
The full polish goal remains active.


Terminal doorway regression (0.14): the original actor can detect the final
open doorway again while idle after RUN CLEAR. Native exits are now cleared
when the run is terminal, preventing repeated floor increments or transitions.
`tools/tactics_terminal_smoke.py ELF OUTPUT` generates ten checks. This fixture
requests the first world transitions, then crosses the actual final door using
movement input. It verifies the exact terminal floor, stable HP, ignored combat
and turn inputs, unchanged room visits and Select retry with restored HP.
The old build reaches floor 19 within 120 frames of victory and fails the floor
stability check (`terminal-door-old-evidence/terminal.txt`); the final build
remains at floor 3 and passes all ten (`terminal-door-evidence/terminal.txt`).
The earlier 0.13 input-only victory proves reaching clear but fails the added
post-clear stability check, so its hash and replay above remain historical
0.13 evidence rather than proof of a polished terminal state.


**Final 0.14 verification:** 53 emulator checks pass on the final ROM: native
commands (17), door hit/travel (14), fresh spawn/Fire (6), legacy suspend repair
(6), terminal stability/retry (10). The current evidence is in
`release13-native-evidence`, `release13-door-evidence`,
`release13-spawn-evidence`, `release13-legacy-evidence`, and
`terminal-door-evidence`; the release13 folder prefix is historical, and every
fixture ROM hash matches the final 0.14 ROM. The strict host suite, four BPS
tests and both region/party Lua checks pass. The 21,342-byte 0.14 BPS patch
reconstructs the final ROM byte-for-byte.

The final-ROM input-only replay
`final-terminal-three-world-evidence/traversal.txt` reaches victory at frame
79,218 and passes terminal stability at frame 79,338: floor 3 and Sora HP 51
remain unchanged for 120 frames after clear. It records 17 kills, 680 movement
commands and 24 Goofy Guards. The driver performs no emulated RAM writes,
teleports, enemy-HP changes or forced transitions. Its native mode and update
function are checked throughout. SHA-256 of the release and replay ROMs:
`6150d69bbdd9772ea86a9258fcc53f7dbfeddf1a88c5d761fd23de6336ea9e59`.
This verifies one default-seed victory and stable terminal state. Optional
branches, input-only chest openings, other seeds, full-run suspend and physical
hardware remain unverified; player height previews, richer rewards/bosses and
UI/animation polish remain implementation work. The full goal stays active.
