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

# Playable tactics alpha; validates its header and ROM capacity.
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
Small actor bitmaps are generated from local extracted sprite sheets during
configuration and remain under ignored build/.

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

## Emulator replay

The replay generator emits a deterministic host policy, then translates it
into actual button presses and checks the resulting GBA state. It never writes
to emulator memory during the full-run replay.

```sh
mkdir -p build/tactics build/tactics-us/evidence
cc -std=c89 -Wall -Wextra -Werror -I tactics tests/tactics_trace.c tactics/tactics.c tactics/save.c -o build/tactics/trace
build/tactics/trace > build/tactics-us/input-trace.txt
python3 tools/tactics_emulator_trace.py build/tactics-us/input-trace.txt build/tactics-us/replay.lua build/tactics-us/evidence
# macOS path; substitute your mGBA executable on another host.
/Applications/mGBA.app/Contents/MacOS/mGBA --script build/tactics-us/replay.lua build/tactics-us/kh_tactics.gba
```

The save smoke script performs input-driven suspend/resume and reset checks,
then deliberately corrupts only the newest emulator SRAM slot to test fallback:

```sh
mkdir -p build/tactics-us/save-smoke
/Applications/mGBA.app/Contents/MacOS/mGBA --script tests/tactics_save_smoke.lua build/tactics-us/kh_tactics.gba
```

mGBA scripts use the documented API at https://mgba.io/docs/scripting.html.
The upstream ROM CI uses a container with external ROM inputs. That job is
restricted to the upstream repository; the fork runs the asset-free host
checks instead. Matching and tactics ROM validation run locally.

Inspect the result files and screenshots under the selected evidence directory.

## BPS patch

```sh
python3 tools/tactics_patch.py create roms/B8CE.gba build/tactics-us/kh_tactics.gba build/release/kh-tactics-0.1.0-alpha.bps
python3 tools/tactics_patch.py apply roms/B8CE.gba build/release/kh-tactics-0.1.0-alpha.bps build/release/kh_tactics.gba
```

Creation verifies the input SHA-1 and a byte-exact application round-trip.
Application checks the input SHA-1 and BPS source/target/patch CRC32 values.
Distribute the patch and source, rather than the ROM or extracted files.

## Validation recorded on 2026-10-06

- Matching US build produces the original SHA-1.
- Host checks pass under strict warnings, AddressSanitizer and UndefinedBehaviorSanitizer.
- Thirty deterministic seeded runs reach completion, with save round-trips at every step.
- Input-only mGBA replay completes the three-floor run and compares 41 state checkpoints.
- Emulator save tests pass immediate resume, reset/resume and corrupt-newest-slot fallback.
- Passing turns without defending reaches defeat with zero HP in mGBA.
- Patch application reconstructs the exact tactics ROM.

This is a playable alpha. Physical GBA/flashcart validation and broad player
balance testing remain outstanding. The test policy's success rate is a
regression check, not evidence that difficulty is balanced.
