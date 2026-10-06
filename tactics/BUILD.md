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
a versioned header, generation counter and CRC32. Two 512-byte SRAM slots
retain the previous valid snapshot while the next is written.

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
python3 tools/tactics_patch.py create roms/B8CE.gba build/tactics-us/kh_tactics.gba build/release/kh-tactics-0.2-field.bps
python3 tools/tactics_patch.py apply roms/B8CE.gba build/release/kh-tactics-0.2-field.bps build/release/kh_tactics_field.gba
```

Creation verifies the supported input SHA-1 and a byte-exact application
round-trip. Application verifies source, target and patch CRC32 values.
Distribute source and the patch, never the ROM or extracted assets.

## Recorded verification, 2026-10-06

- Original matching US ROM remains byte-identical.
- Old rules/save core passes strict C89, ASan/UBSan and 30 deterministic runs.
- New room-graph generator passes 3,000 seed/floor combinations: determinism,
  reciprocal doors, connectivity and parameter bounds.
- Native mGBA input smoke passes movement stopping, budgets and field animation.
- Native fixture scenarios pass enemy hits, chest reward, transitions between
  three world asset sets, run clear and retry.
- New target BPS patch reconstructs the built ROM exactly.

The native version still needs card/deck integration, height-aware route and
intent previews, physical reachability guarantees, bosses, suspend saves,
input-only complete-run QA and hardware validation. The board alpha's earlier
emulator results do not establish these features in the native target.
