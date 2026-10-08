# Headless native validation

The optional runner uses official mGBA core commit
`cef7dde504af189e47f6074365f2d77e8177ad06`, matching the desktop core used
for the existing captures. `tools/build_headless_mgba.py` modifies only the
upstream headless frontend: it adds a software pixel buffer, automatic SRAM
loading, and a `-F FRAMES` clean exit bound. The emulation core is unchanged.
A clean process exit alone is not a replay pass; inspect the script log.

Prerequisites on this macOS workspace: `.venv/bin/cmake`, `.venv/bin/ninja`,
and Homebrew Lua, libpng and zlib. Fetch the pinned official source into the
ignored directory before building:

```sh
git init build/tools/mgba-source
git -C build/tools/mgba-source fetch --depth=1 https://github.com/mgba-emu/mgba.git cef7dde504af189e47f6074365f2d77e8177ad06
git -C build/tools/mgba-source checkout --detach FETCH_HEAD
python3 tools/build_headless_mgba.py
```

Example invocation after generating a native test and copying its ROM:

```sh
build/tools/mgba-headless/mgba-headless -l 7 -F 1300 --script build/tactics-us/headless-rally-menu-evidence/test.lua build/tactics-us/headless-rally-menu-evidence/fresh.gba
```

## Observed current-ROM results

ROM SHA256: `6f7a3b146689c71c1442566927fbe8ad30494e60ed19efa0af1192843057503d`.
End Turn forecast: 12/12 checks. Rally assembly/menu/save/reset: 20/20.
SRAM alternating slots, newest resume and damaged-newest fallback: 12/12.
Four 240x160 captures (forecast, assembly, commands, resumed field) are
pixel-identical to their desktop counterparts in the 0.32 evidence folders.

The separate input-only main campaign reached Castle Oblivion room 7 at
frame 116471 but failed its 120000-frame completion bound at frame 120001.
It is not a complete current-ROM campaign proof. An independent current-ROM
all-room campaign was started with a 300000-frame bound; its terminal result
must be recorded separately. The older desktop all-room session remains
untouched. Native tests and campaign scripts use their actual ROM and ELF;
never mix these with an older package's completion evidence.

## Complete current-ROM campaigns

The revised driver in ed20c687c passes the native input-only main route at
frame 119943 (victory119823, Sora80 HP stable120 frames, 14 kills).
Cloud recruited18931/deployed24436; suspend8798/exact resume9128;
composed descent1258; worlds47957/82429. Evidence:
`build/tactics-us/headless-ack-combat-main-evidence`.

The independently running pre-revision driver also completes all rooms at
frame281892 (victory281772, Sora80 HP stable120 frames, 41 kills).
All three room masks are4095, all nine chests opened; Cloud recruited28856/
deployed34832; suspend14734/exact resume15064; composed descent1218;
worlds81469/160785. Evidence:
`build/tactics-us/headless-current-all-rooms-evidence`.

These are separate fresh-SRAM replays of the exact 0.32 ROM above. Completed
metadata verifies ROM and generated-driver hashes and records the terminal
log hash. The earlier120000-frame failure remains retained. Two consecutive
seeds are being tested separately; one-seed success does not prove them.
