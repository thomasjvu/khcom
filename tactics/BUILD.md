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
python3 tools/tactics_patch.py create roms/B8CE.gba build/tactics-us/kh_tactics.gba build/release/kh-tactics-0.38-skills-party-ui-dev.bps --version 0.38-development
python3 tools/tactics_patch.py apply roms/B8CE.gba build/release/kh-tactics-0.38-skills-party-ui-dev.bps build/release/kh_tactics_skills_party_ui_038.gba
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


Attached stair previews: `tools/tactics_climb_preview_smoke.py ELF OUTPUT`
generates `climb.lua`. The 23-check position fixture passes in
`climb-preview-evidence/climb-preview.txt`: L+Up opens a one-point vertical
preview, opening/cancellation preserve movement and attachment, A executes
one native segment, and normal descent, exhaustion, attached suspend/reset,
drop and post-landing party selection remain valid. The projected original
value digit indicates the vertical target; original physics controls any
horizontal shift and climb-over at the top. Combined walking/stair/jump paths
and guaranteed landing previews remain unfinished.


Stair preview limits: descending markers clamp to the actual supporting floor,
including a native prop platform below an attached actor. Zero-distance descent
is rejected. The expanded fixture passes 28 checks in
`climb-preview-floor-evidence/climb-preview.txt`: exhausted movement permits
inspection but blocks A without changing height, and the last descending
marker matches the native floor. Cancellation, exact costs, attached suspend,
drop and later party switching still pass. This does not add a guaranteed
horizontal landing preview for the native climb-over animation.


Cure recipient preview: the HUD and actual heal now share `NativeCureTarget`,
retaining the existing range, height, missing-health and character-bonus rules.
The selected Cure names Sora, Donald or Goofy before A. The expanded party
replay passes all 30 checks in `cure-target-evidence/party.txt`, including
pre-play target checks for injured Donald and nearby knocked-out Donald,
matching heals, card costs, independent budgets, Guard, dual-slot corruption
recovery and friend knockout. `cure-target.png` captures the actual native HUD.


Fire intent: preview and play share `NativeFireTarget`, retaining native
world-space range and height rules and nearest-target tie order. Selected
Fire projects expected HP loss with original digit sprites, including Donald's
bonus, card-value breaks and remaining HP. The HUD reports no target when
none is in range. It is suppressed during movement previews, busy commands,
enemy turns, terminal state, exhausted actions and stocked sleights. Ten
checks pass in `fire-preview-final-evidence/spawn.txt`: original ranged spawn,
pre-play target/remaining-HP agreement, out-of-range clearing without action
cost, and a real Fire kill with exact reward/action cost. The fixture moves
the actor temporarily out of range, then restores it; it is not input-only
complete-run evidence. The strict host suite also passes.


Fire height/break verification: the HUD explicitly names a predicted card
break instead of only showing zero damage. Fourteen checks pass in
`fire-break-verified-evidence/spawn.txt`, including a nearby target above the
24-pixel height limit, a low-value card that projects zero and records an
actual native break with no HP loss, its action cost, and an independent lethal
Fire case. This is a position/card fixture: after the actual break, the final
case restores a single selected Fire hand card and places Sora near the target.
It does not establish reload ordering or input-only run coverage.

Current source regression (party/reward/HUD build): the input-only three-world
probe in `build/tactics-us/reward-party-full-run-evidence/traversal.txt` ended
in defeat at frame 57,971 after reaching Castle Oblivion, with 12 kills and
523 movement commands. ROM SHA-256:
`7732bd31a0d436d679bd33d5b763572544026e49584a021a323c7288fb12db37`.
This contradicts treating the historical 0.14 victory as verification of the
current ROM. No new release patch is certified by this run. The driver now
logs party HP at room entry and HP/threat/Guard before each enemy turn to
identify the failure; it still only reads emulated memory and sends inputs.

HUD replacement labels now clear the remaining screen row, preventing a short
Cure/Fire/reward prompt from retaining a previous label's trailing glyphs.
The current reward/save/capacity fixture passes 25 checks, including a native
tilemap assertion that the shorter reward footer leaves only blank tiles to
its right (`hud-reward-final-evidence/reward-save.txt`).

0.15 party/reward development build: the diagnostic rerun on the same ROM
SHA-256 `7732bd31a0d436d679bd33d5b763572544026e49584a021a323c7288fb12db37`
completed all three worlds and passed 120-frame terminal stability at frame
82,661, with 17 kills, 691 movement commands and Sora HP 57. Evidence:
`build/tactics-us/party-reward-health-trace-evidence/traversal.txt`. This
input-only rerun contrasts with the earlier defeat on the identical ROM;
repeatable replay outcomes are not yet established. Both results remain
preserved. The development BPS includes original party walk/jump presentation,
sleight/Cure/Fire previews, chest card choices and HUD label cleanup.
The 25-check reward/save/capacity fixture also passes. Optional branches,
input-only chest collection, other seeds and physical hardware remain
unverified. This is not a fully polished release.

Post-0.15 source presentation: Donald/Goofy share original shadow tiles and
palette; each living friend's shadow is drawn at world Y plus standing ground
height, while the body uses its actual Z. Body/shadow receive the same
inactive-coincident display offset. Resources are allocated once per room and
released on exit. `party-shadow-jump-evidence/jumps.txt` passes 15 input-only
Goofy jump checks; `party-shadow-regression-evidence/party.txt` passes 34 party
checks, and the rising screenshot was inspected. These tests cover movement
and party regressions; they do not establish full-run reproducibility. The
0.15 patch remains the previously packaged hash, without these shadow changes.

The two 0.15 full-run traces match through the first doorway (frame 896),
then differ within Traverse Town before any world transition. The diagnostic
trace's first Goofy Guard is at frame 2,180; the earlier trace's is at 2,208.
This locates divergence earlier than Castle Oblivion but does not prove an
RNG cause. Room generation explicitly seeds from room state and native enemy
placement is also seeded; the unresolved audit must consider frame-sensitive
input/controller behavior as well as presentation/enemy RNG.

Native Fire selection: `tools/tactics_fire_target_smoke.py ELF OUTPUT` generates
13 checks using two explicitly placed original enemy actors. Native R+B input
selects the farther target, rejects invalid height/range, preserves budgets
and the selected card, and A applies exactly the previewed damage only to the
chosen enemy. `fire-choice-evidence/fire-target.txt` passes all 13 checks; the
existing spawn/Fire break/height regression passes all 14 checks in
`fire-choice-regression-evidence/spawn.txt`. Target choice is transient and
resets on room initialization/resume. The packaged 0.15 patch is unchanged.

Native Cure selection uses R+B with Cure selected. Both the named HUD target
and resolution share eligibility/choice logic, including knocked-out friends.
`cure-choice-final-evidence/party.txt` passes 40 checks: the existing party
regression, native cycling through Goofy/Sora/knocked-out Donald, preserved
action budget, actual revival of the chosen member, and explicit invalid
height/range choice fixtures. Target selection resets on room entry/resume
and does not change the suspend format. The 0.15 patch remains unchanged.

Cure recovery preview and resolution now share `NativeCureRecovery`, including
Donald’s bonus and the missing-HP cap. Original value digits appear above the
chosen party position, offset above incoming damage digits.
`cure-recovery-evidence/party.txt` passes 44 checks, including capped healing
for injured Donald, zero recovery for full-health Sora, exact revival HP and
clearing the preview after spending the action. These are explicit native
party fixtures, not additional full-run evidence. The 0.15 patch is unchanged.

HUD VBlank handoff regression: completed HUD RAM is now immutable while an
upload is pending. `tools/tactics_hud_smoke.py ELF OUTPUT` generates ten checks
against the original font glyphs, HUD RAM and BG0's actual configured VRAM
character bank. It temporarily disables the callback and changes HP to verify
the pending buffer is not overwritten, then restores the callback and verifies
updated glyphs reach VRAM. `hud-handoff-evidence/hud.txt` passes all ten; the
old-ROM control in `hud-handoff-control-evidence/hud.txt` fails the pending
buffer immutability assertion. Both ROMs pass normal uploads; this proves a
handoff invariant violation, not that ordinary uploads always starved. Earlier
checks incorrectly assumed character bank zero and are superseded.
`hud-cure-regression-evidence/party.txt` also passes all 44 party/Cure checks.
Screenshots show active HP 037 and independent party HP after the fixture.
The packaged 0.15 ROM remains unchanged.

Route actor sweep: party and enemy occupancy is checked at quarter-segment
samples as well as destinations, shared by player preview and enemy routes.
Samples allow leaving an actor overlap already present at the segment origin;
destinations must still be unoccupied. This preserves coincident entrance
party movement. The first implementation blocked entrance movement; its
failed fixture is retained in `swept-actor-route-evidence`. The corrected
input-only walking fixture passes in `swept-actor-route-fixed-evidence`,
including multi-segment execution and budget rejection. Strict host tests
and ROM build pass. A dedicated enemy crossing scenario and full-run
regression remain required; quarter samples are not a continuous sweep.

Replay evidence metadata: traversal generation now writes
`replay-metadata.json` with SHA-256 of the built ROM, ELF and complete generated
Lua driver (including geometry helpers), goals and frame bound. Generation
requires the matching ROM beside the ELF and rejects direct emulated-memory
write calls in the generated script. Metadata explicitly leaves result
unobserved; completion must still be read from `traversal.txt`. This helps
separate differing drivers from differing binaries and does not prove replay
determinism. Current route-regression full-run evidence is being collected in
`swept-route-full-run-evidence` for ROM SHA-256
`404bb62955d9090a7e39145be51fa2054062e2cd8be3ef05e5fb7476683ef7b1`.

Attack sleight area preview: native attack and preview now share range/height
eligibility and power calculations. Original digits show capped damage above
each eligible enemy; the preview is suppressed while busy, in enemy phases,
reward choices, route previews and terminal states.
`sleight-area-evidence/sleights.txt` passes 14 checks, including capped preview
on an explicitly positioned enemy and actual native resolution afterward.
This fixture covers the basic melee sleight; separate Fire-area boundary and
multiple-target scenarios remain needed. The 0.15 patch remains unchanged.

Sleight boundary validation: `tools/tactics_sleight_smoke.py ELF OUTPUT` also
generates `sleight-area.lua`. The explicit two-original-enemy fixture in
`sleight-boundary-evidence/sleight-area.txt` passes 11 checks: exact Fire range
144 and height 24 boundaries, out-of-range/height rejection, melee range 64,
two-target preview/resolution equality, preserved inspection budgets and
post-resolution preview clearing. It changes card types/values and enemy
positions explicitly to isolate these cases; stock and attack use native
controller input.

The route-change input-only full-run regression completed in defeat at frame
52,280 in Agrabah, with eight kills and 485 movement commands. Evidence:
`swept-route-full-run-evidence/traversal.txt` and its hash metadata, for ROM
SHA-256 `404bb62955d9090a7e39145be51fa2054062e2cd8be3ef05e5fb7476683ef7b1`.
This run precedes the sleight preview change. It provides navigation/combat
failure evidence, not a victory or proof that the run is unwinnable. Current
source still needs a complete-run pass and reproducibility investigation.

Climb prompt polish: attached-stair previews now show `A CLIMB B CANCEL` or
`A DESCEND B CANCEL`, retaining higher-priority blocked/unaffordable warnings.
`climb-prompt-evidence/climb-preview.txt` passes all 28 existing stair checks
(save/reset, exact level movement, budget rejection, bounded descent and
landing/party control). The Up preview screenshot was visually inspected.
The fixture generator also captures the Down preview for future review.

Reward control edge cases: the reward/save tail now additionally verifies
Left/Right wraparound, full-deck browsing without granting a card, B keeping
the pending reward open, and later A input not duplicating healing/cards or
chest count. `reward-control-final-evidence/reward-save.txt` passes 31 checks
on the current ROM. The first attempt used incorrect post-reset selection
expectations; those failed assertions are preserved in `reward-control-evidence`.
Capacity/pending-state fixtures explicitly write emulated state; this is
focused native control QA, not input-only chest exploration evidence.

Enemy height targeting: threat preview and enemy turns now share a target
selector that prefers a living member within attack range and height; if
none qualifies, it retains nearest-member pursuit. Large Body uses its
existing charge range/height for this decision and retains area resolution.
The updated party fixture explicitly places Sora at the same projected
position as an enemy but 64 pixels higher, while Donald remains attackable.
`enemy-height-target-evidence/party.txt` passes all 44 checks, including
Donald’s predicted/actual damage and Sora remaining unharmed. This verifies
the focused targeting case, not full height-aware pursuit or a complete run.
The packaged 0.15 patch is unchanged.

Matching-stock field recipes: pure `FieldDeckRecipe` identifies three matching
types without mutation. Named HUD recipes give Key/Fire +6 power and Cure +8
healing; Guard retains existing full protection. Preview and attack share
recipe power; resolution captures the recipe before consuming stock. Strict
host tests verify mixed/Fire/Cure classification. The ROM builds and all 11
area boundary/multiple-target checks pass in `enhanced-recipe-evidence`; three
Fire sixes preview and deal 32 damage to both 40-HP fixture enemies. Dedicated
Curaga healing and enhanced melee resolution fixtures remain outstanding.
The packaged 0.15 patch remains unchanged.

Enhanced Cure/melee recipe resolution: the sleight generator also produces
`recipes.lua`. `recipe-resolution-evidence/recipes.txt` passes 12 checks:
Curaga's +8 party healing, knockout revival, HP cap, normal exhaustion and
action cost; then a real enemy turn refreshes the action before Triple Key's
32-damage preview/resolution and out-of-area exclusion. Explicit card, HP and
enemy-position fixtures isolate the recipes; stock, turn and attack commands
use native input. This fills the focused Curaga/Triple Key validation gap
noted above, without claiming full-run or canonical recipe fidelity.

Party-wide Cure sleight preview now shares capped recovery calculations with
resolution. `curaga-preview-evidence/recipes.txt` passes 16 checks, including
38 HP recovery for injured Sora and knocked-out Donald, a seven-HP Goofy cap,
and preview clearing after resolution. ROM build passes. A full-disk error
blocked the first replay copy; 170 redundant generated ROM copies from
completed evidence folders were removed after recording SHA-256/size in each
folder's `removed-rom-hashes.jsonl`. Logs/screenshots and current release
artifacts were preserved. Historical evidence paths may therefore no longer
contain a ROM copy; use the recorded hash when assessing their scope.

Enhanced recipe suspend regression: the sleight generator also emits
`recipe-save.lua`. `recipe-save-evidence/recipe-save.txt` passes nine checks:
matching Key/six-value stock survives native suspend/reset with exact piles
and action budget, reconstructs 32-damage preview, resolves once, exhausts
the first card and clears stock. Card setup and post-reset enemy placement
are explicit fixtures; stocking, save and attack use native input. Recipe
identity is derived from the persisted deck, so no save-format change is
required. This verifies enhanced recipe persistence, not full-run reset QA.

Spent-action guidance: the native HUD replaces unavailable play/sleight prompts
with `ACT SPENT START TURN`, retaining higher-priority climb, charge, preview,
reward, enemy-turn and terminal displays. The HUD smoke fixture now compares
the complete prompt against original font glyph RAM; all 11 checks pass in
`spent-action-hud-evidence/hud.txt`. ROM builds and diff checks pass.

Route timeout refunds: native execution records each segment's starting
position. On timeout it refunds the current movement point if horizontal
progress is under four pixels, alongside all unstarted later segments. This
matches the existing direct-step no-progress threshold.
`route-refund-evidence/routes.txt` passes 29 checks: 27 normal input-only
route checks plus an explicit frozen one-segment execution fixture verifying
timeout releases the gate and refunds the point. The appended fixture writes
route/controller state and is not input-only proof. Partial-progress timeout
and dynamic-prop obstruction scenarios remain unverified. ROM builds.

The extended `route-progress-refund-evidence/routes.txt` passes 33 checks.
Explicit frozen two-segment fixtures verify that four pixels of prior travel
costs the current point while refunding the later segment, and zero prior
travel refunds both points. These fixtures represent prior travel by setting
the recorded segment origin; they do not demonstrate a naturally occurring
dynamic obstruction. Dynamic-prop obstruction remains unverified.

Traverse Town room 7 now has a solo Guard Armor presentation built from the
original torso, head, hands, feet and collar animations/palette. Its field
collision task remains the Large Body controller; native tactics owns its
40 HP and warned 80-pixel slam. Shared preview/resolution checks include
the 24-pixel height limit. Other worlds retain their Large Body elites.
Idle poses animate; attack poses and multipart damage phases remain pending.

Native initialization releases the invisible vanilla scenery tile-reservation
task. That task reserves unused scenery capacity and has no rendered output;
native rooms retain their generated terrain, props and doors. This frees OBJ
space for party/card assets and the seven boss components. Boss initialization
replaces the collision proxy's palette/tiles, allocates only idle animation
frames, and restores proxy assets on allocation failure. Palette references
are shared correctly between field task destruction and room release.

`guard-armor-reservation-evidence/boss.txt` passes 17 explicit-fixture checks:
all card/digit assets in entry/boss rooms, all seven boss components, exact
slam range/height boundaries, actual party damage, charge consumption and
suspend reconstruction. Screenshots confirm multipart original art in the
original field world. Earlier allocation failures are preserved in evidence
directories. `guard-armor-cleanup-evidence/boss.txt` passes 18 checks covering
real windup, evasion, Guard, save/reset, Fire defeat, task release, room exit
and subsequent card resource reconstruction. These tests use explicit room,
position and HP fixtures, not an input-only complete-run claim. ROM build and
host checks pass; host generation asserts solo Traverse Town encounters across
1,000 seeds. Save format remains 8; previously saved room counts remain exact.
`boss-resource-hud-evidence/hud.txt` also passes all 11 glyph/upload checks
after releasing the scenery reservation. The fresh input-only three-world replay
in `guard-armor-full-run-evidence` reached Agrabah's room-7 elite, then ended
in defeat at frame 53,352, with eight kills and 485 movement commands. Its ROM
SHA-256 is `13d640c6c27ab533f03867776449da405d5e8c2e60fbd75c41801ce17da8565b`.
It passed Traverse Town's new solo Guard Armor but does not verify complete-run
victory. Party survival and the traversal driver's boss-defense policy remain
open work; this result must not be presented as a successful full-run test.

Guard Armor slam presentation now uses its original torso crouch, collar and
orbit-hand animations. A charged decision raises the hands and lowers the
body; the next decision shows an impact pose for 24 presentation updates,
then returns to idle. Feet and shadow stay on the original standing surface.
Tile allocation covers only frames in these selected animation sequences.
The pose is derived from saved charge state on resume; the short impact timer
is presentation-only and cannot change damage or advance enemy decisions.
`guard-armor-grounded-poses-evidence/boss.txt` passes all 21 checks, including
windup/impact/idle transitions, reconstructed suspended windup, card and boss
asset allocation, and existing slam boundaries/resolution. Screenshots confirm
raised hands and grounded feet. ROM build and diff checks pass. This does not
verify additional boss moves, multipart damage phases, or full-run victory.

Guard Armor now has custom tactics damage phases derived from its persisted
HP: 27–40 HP keeps both hands (80-pixel strike, 10 damage); 14–26 HP removes
the far hand (64 pixels, 8 damage); 1–13 HP removes both hands (48 pixels,
6 damage). The original surviving sprite components continue animating.
Threat preview and enemy resolution use the same phase functions; Guard
reductions and the 24-pixel height limit remain unchanged. HUD warnings name
the current strike and reach. Phase identity needs no additional save bytes.
`guard-armor-phase-hud-evidence/boss.txt` verifies the existing presentation
and slam cases plus native Fire crossing a break threshold, exact phase ranges,
actual reduced damage, and saved body-phase HP/preview/party damage restoration.
Position/HP boundary setup is explicit; the Fire command is native input.
Host tests cover phase thresholds and range/damage values. Full-run victory,
break VFX, additional attack patterns and the other canonical bosses remain
unverified or unfinished.

The input-only traversal driver now explicitly cycles Cure to Sora when its
policy requests recovery for his HP below 60. Previously it selected Cure but
accepted the game's most-injured default recipient, often healing another
member. When Cure is unavailable and Sora HP is within 16 of projected incoming
damage, it prefers a Guard card over Fire. Both changes use native controls;
the ROM/rules are unchanged by this driver work. Read-only mock tests cover
recipient cycling, selected-card/action gates, Cure priority, defensive Guard
and healthy Fire preference, alongside the existing Goofy turn policy tests.

Two full input-only traces on ROM SHA-256
`f618c99d66af5ba1bd227dd0a5912c0d9c6a994dbc594b9237bb6a168fc1bbe4`
remain failures: `explicit-sora-cure-full-run-evidence` (Cure targeting only)
ended in Castle Oblivion room 7 at frame 73,903, with 17 kills/509 commands;
`defensive-sora-full-run-evidence` (Cure plus defensive Guard) ended in Castle
Oblivion room 0 at frame 59,050, with 11 kills/518 commands. Earlier arrival in
the second trace is not proof of better combat performance. Both metadata files
record their terminal log result. The latest source still lacks a verified
complete-run victory; party deployment/recovery and card-use policy need work.

Guard Armor's replacement draw callback now draws its original child task pool
after setting the ground shadow position/priority, restoring original shadow
and spark rendering. Native enemy maintenance updates child visual tasks while
the original parent AI remains frozen. This also lets ordinary field hit sparks
advance and release their resources instead of retaining frozen child effects.
When nonlethal card damage crosses an armor phase, an original `MapSpark` effect
is created once for that attack. Resume derives the phase from HP without
replaying the break effect. Child task capacity bounds effect creation.

`guard-armor-effect-turn-evidence/boss.txt` passes 37 checks. Added cases verify
a native Fire threshold crossing starts a visible original spark, its child
task expires, it is not emitted again during idle rendering, and visual updates
do not advance the authoritative enemy-turn counter. The existing 33 asset,
phase, range/height, damage and suspend checks also pass. Screenshot inspection
confirms the original yellow spark against the multipart boss. ROM build,
host checks and diff checks pass. This is focused visual/phase evidence, not
full-run victory or broader seed/hardware certification.

Agrabah room 7 now contains a solo sorcerer Jafar with 48 HP, using the
original field idle and lamp-pose sprites/palette. This is a custom tactics
encounter, not a port of the original giant Genie battle. It retains a native
field collision task and shadow/effect children. A decision within 96 projected
pixels and 32 pixels of height charges a single-target 10-damage spell; the
next decision targets the closest currently reachable living member. Preview
and resolution share selection and Guard reductions. Saved kind/HP/windup
reconstruct the art and threat after resume. Original event art has static
idle/lamp poses; richer casting animation and projectile VFX remain pending.

Jafar's exit regression exposed missing Guard card art, then missing value
digit palettes in the next Agrabah room. Optional prop placement now stops
requesting nonmandatory spots once two prop palettes are counted; mandatory
base props remain exempt. The US installer verifies the original private
counter address (`sMapGmkPaletteCount` at `0x02034f79`) before patching. Regular
cohorts reuse two enemy palettes, with seeds varying their pair and all world
roles retained across seeds. Fresh Traverse Town/Agrabah boss rooms are solo;
Castle retains its existing encounter. These budgets preserve tested card and
digit colors within sixteen OBJ banks. Legacy cached cohorts retain stored
roles; broader legacy-save/seed palette coverage remains unverified.

`jafar-cohort-budget-evidence/boss.txt` passes 24 explicit-fixture checks:
original Jafar/card/digit resource allocation, actual native windup, exact
spell range/height bounds, single-target preview/damage, Guard, suspend/reset,
native Fire defeat and room resource reconstruction. Forced floor advancement,
positions, and final HP/card selection are fixture setup, not full-run proof.
Earlier palette failures are preserved. On the same ROM,
`jafar-budget-armor-regression-evidence/boss.txt` passes 37 checks and
`jafar-budget-reward-regression-evidence/reward-save.txt` passes 31. Host checks
verify solo room counts and preserved world-role diversity over seeded groups.
Native build and diff checks pass. Full-run evidence is recorded separately.

Two fresh-SRAM input-only replays complete all three worlds on this source:
`jafar-full-run-evidence` reaches victory at frame 97,225 and passes 120-frame
terminal stability at 97,345; `jafar-full-run-repeat-evidence` reaches victory
at 97,328 and passes stability at 97,448. Both record 17 kills, 799 movement
commands and final Sora HP 60. No emulated memory writes, teleports or forced
exits are used by the full-run driver. Both use ROM SHA-256
`ae815595a2c7c3de21736baecd64716266aa69fb045799e5292d1f7fcf749d32`.
Metadata now also records a driver logic hash with only output paths normalized;
ROM and logic hashes match across these repeats. Separate output directories
preserve both logs/screenshots/SRAM. Frame timing differs, so these are repeat
successes rather than bit-identical deterministic traces. Coverage is the
default seed/main route only; broader seeds, optional branches, full-run reset,
the remaining boss content, navigation guarantees and hardware remain open.

The 0.16 world-bosses development BPS is now packaged as
`build/release/kh-tactics-0.38-skills-party-ui-dev.bps` (29,647 bytes). Application to
the supported original US ROM reproduces the tested native ROM byte-for-byte,
SHA-256 `ae815595a2c7c3de21736baecd64716266aa69fb045799e5292d1f7fcf749d32`.
`world-bosses-0.16-manifest.json` records patch/source hashes, the native source
commit, both fresh-SRAM victory/stability traces and their normalized driver
logic hashes, plus 92 focused fixture checks and scope limits. Release notes
are in `build/release/WORLD-BOSSES-0.16.md`. BPS unit tests pass all four cases.
The package uses save format 8 and remains a development build; it does not
certify the open content, navigation, other-seed, legacy-save or hardware work.
Old release artifacts and historical evidence are preserved separately.


Castle Marluxia full-run verification (source c08e1c13b):
`marluxia-full-run-evidence/traversal.txt` completes all three worlds through
controller input from fresh SRAM. Victory is followed by 120 stable terminal
frames; final PASS at frame 95,731 records 15 kills and 797 movement commands.
The ROM hash and driver hashes are recorded in `replay-metadata.json`.
This verifies the default-seed main route with the new solo Castle boss;
optional branches, other seeds and physical hardware remain open.

`tools/tactics_traversal_probe.py --suspend-room 2` now supports a controller-only
save/reset checkpoint in Traverse Town. The driver releases save keys, resets,
waits for native boot and compares party HP, budgets, Guard, world/room and deck
before continuing. This mode is prepared for integration testing; generation
and the existing combat/party policy tests pass, but its full replay is pending.

The first controller-only suspend/reset three-world replay passes at frame
95,837 with 14 kills and 788 movement commands. It saved at 18,168 in Traverse
Town room 2 and verified resume at 18,498 before continuing to victory. Evidence:
`marluxia-suspend-run-evidence/traversal.txt`. This driver checked HP, budgets,
Guard and deck; the subsequently strengthened position/enemy checks await a
new replay. The replay predates the distinct Donald/Goofy attack implementation.

Distinct character base attacks now pass eight explicit native emulator checks
in `character-moves-evidence/moves.txt`: Donald targets ranged magic and Goofy
spins his shield across nearby enemies; previews match actual damage and attacks
spend the selected member's action. Generator:
`tools/tactics_character_moves_smoke.py`. Recruitment and party assembly requirements
are tracked in `tactics/PARTY-ROGUELIKE.md`; those systems remain unfinished.

Character attack build full-run/reset verification: source c19ee99f6,
`character-moves-suspend-full-run-evidence/traversal.txt`, PASS frame 94,761,
12 kills, 780 moves, Sora HP 58. Suspend at 18,098, verified resume at 18,428.
This driver verifies all party positions, active member, HP/budgets/Guard, deck
including selection, enemy HP and windups; then completes all three worlds and
120 terminal stability frames. It predates format-9 roster serialization.

Format-9 roster state: host save/rules tests and `roster-save-evidence/moves.txt`
pass (11 native fixture checks). `tactics_field_legacy_save_test.c` also reads
recorded format-8 native SRAM and verifies default roster migration.

Round setup build (361b911c1) completes all three worlds with a mid-run
suspend/reset: `assembly-suspend-full-run-evidence/traversal.txt`, PASS at
96,493, 15 kills, 797 movement commands, Sora HP 80. Save/resume at 18,154/18,484.
This predates encounter-clear reward gameplay and the font fix.

Encounter progression and HUD: fourteen explicit native checks in
`progress-power-verified-evidence/progress.txt` pass. Card attacks clear the
second-new-encounter fixture, a reward opens, Donald alone receives power +1,
and pending/confirmed state survives reset without duplicate grants. Upgraded
magic previews and resolves 16 damage using the value-6 Fire card. Rendered HUD
now keeps foreground F ink and excludes D shadow pixels, so numeric values stay
distinct. The first power fixture expected the wrong card value and failed;
`progress-power-evidence` is retained, then corrected expectations passed.

Reachable movement overlay: native `reach-overlay-evidence/reach.txt` passes
eight checks, including allocation, reachable mask/cursor cost agreement,
three/one/zero-point budgets and cancellation. `reach.png` shows terrain-space
diamond outlines alongside original party/room art. Host bounded flood-fill
tests compare every reachable cost against destination routing, including
diagonals, height barriers and enclosed actors. Stair/jump destinations are
not part of this overlay yet.

Progression full-run evidence is a failed replay, retained in
`progression-suspend-full-run-evidence`: the driver stopped on Guard Armor's
reward and hit the 120,001-frame bound (6 kills, 285 moves). Its reward branch
repeated held A without a release phase, so a rejected early press did not
produce another edge. The driver now inserts a release frame between reward
choices; a corrected full-run replay is required.

Corrected progression/overlay input-only full run (98d2835de) passes at
102,377: 14 kills, 856 movement commands, Sora HP 80, stable for 120 victory
frames. Evidence: `reach-progression-suspend-full-run-evidence/traversal.txt`.
It includes a verified mid-run suspend/reset and predates Cloud's optional
encounter. The earlier held-confirm reward replay failure is retained.

Cloud challenge/recruitment: fifteen explicit native checks pass in each of
`cloud-recruit-evidence/cloud.txt` and `cloud-power-choice-evidence/cloud.txt`.
The fixtures verify original battle art, solo HP, charged sword sweep,
range/height boundaries, charged suspend, native Fire defeat, original summon
card resources, recruit or power choice, reboot persistence and exit cleanup.
Generator: `tools/tactics_cloud_recruit_smoke.py`, with `--power` for alternative
loot. Setup and enemy HP/positions are fixtures, not input-only optional-route
proof. Original art is visible in `cloud.png` and `recruit.png`.
Cloud's roster unlock is connected; playable deployment is still unfinished.

Cloud party mapping and format-10 bench state: sixteen explicit native checks
pass in `cloud-deploy-budget-evidence/deploy.txt`, including controller-driven
setup after an explicit unlock fixture, sword preview/resolution, per-hero
health/caps/power, preserved injured/spent benched Donald, and native reboot.
Recorded format-8 and format-9 SRAM fixtures pass host migration decoding.
These checks do not prove input-only optional-route recruitment or complete
Cloud-party runs. The native ROM and all host rule/save tests build and pass.
Summon artwork is reserved for setup/recruitment; combat Guard uses original
Guard Armor enemy-card artwork. Per-character HP/movement/action records fit
the existing 1,024-byte suspend slots (983-byte payload, plus header/checksum).

## Current input-only campaign replay

Use a new output directory and fresh SRAM for each independent replay. The
frame limit is aggregate across all runs; a three-run replay uses 540000.
The headless build script expects mGBA source at `build/tools/mgba-source`,
checked out at `cef7dde504af189e47f6074365f2d77e8177ad06`. It validates that
revision before applying frontend-only changes. See
[headless validation](HEADLESS-VALIDATION.md) for runtime dependencies; then
build with `python3 tools/build_headless_mgba.py`.

```sh
python3 tools/tactics_traversal_probe.py build/tactics-us/kh_tactics.elf build/tactics-us/local-three-runs --worlds 3 --runs 3 --frames 540000 --recruit-cloud --composed-descent --suspend-room 1
cp build/tactics-us/kh_tactics.gba build/tactics-us/local-three-runs/fresh.gba
build/tools/mgba-headless/mgba-headless -l 7 -F 540003 --script build/tactics-us/local-three-runs/traversal.lua build/tactics-us/local-three-runs/fresh.gba
```

Require the final `PASS completed 3 three-world runs` in `traversal.txt`,
check ROM/driver hashes in `replay-metadata.json`, and confirm process exit0.
An exit0 alone does not prove the replay passed. The retained 0.38 evidence is
`build/tactics-us/party-label-three-runs-aggregate-evidence`: PASS379646.
