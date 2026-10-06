# Local development

Worktree: `/Users/area/kh-tactics`. Branch: `tactics/bootstrap`.
`origin` is the fork; `upstream` is Pheenoh/khcom.

## Host rules checks

```sh
sh tools/test_tactics.sh
```

This compiles freestanding C89 with strict warnings, AddressSanitizer and
UndefinedBehaviorSanitizer and runs movement, phase, combat and replay checks.
The prototype currently has adjacent enemy attacks only, no enemy movement,
cards, RNG, renderer or ROM integration.

## Matching US baseline

The local input `rom/khcom.gba` was SHA-1 validated and copied to
`roms/B8CE.gba`. Both directories' ROM contents remain ignored.

Setup performed on macOS arm64:

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt ninja
brew install arm-none-eabi-gcc
git clone https://github.com/pret/agbcc.git build/agbcc-source
(cd build/agbcc-source && ./build.sh && ./install.sh /Users/area/kh-tactics)
sh tools/fetch_gbagfx.sh
.venv/bin/python tools/setup_legacy_toolchain.py
.venv/bin/python tools/extract_assets.py us
.venv/bin/python configure.py --version us
.venv/bin/ninja
```

agbcc revision used: `da598c1d918402c42c0c0d7128ba14567f3175e9`.
The upstream legacy setup pins binutils 2.10 and runtime source checksums.
Local setup/build logs are in ignored `build/`.
Run asset extraction after gbagfx is installed.

The rules source was also compiled by agbcc and assembled to an ARM object:

```sh
arm-none-eabi-cpp -nostdinc -undef -I tactics tactics/tactics.c -o build/tactics/rules.i
tools/agbcc/bin/agbcc -mthumb-interwork -O2 -o build/tactics/rules.s build/tactics/rules.i
arm-none-eabi-as -mcpu=arm7tdmi -o build/tactics/rules.o build/tactics/rules.s
```

This is a compiler compatibility check, not a linked ROM or emulator test.
