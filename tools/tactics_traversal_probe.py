"""Generate a bounded input-only room traversal probe for mGBA.

Reads diagnostics and original door geometry; never writes emulated memory.
A failed probe is navigation evidence, not a claim that the room is impossible.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('elf')
p.add_argument('output')
p.add_argument('--rooms', type=int, choices=range(1, 8), default=1)
p.add_argument('--worlds', type=int, choices=range(1, 4))
p.add_argument('--runs',type=int,choices=range(1,4),default=1,help='complete consecutive runs using native Select retry to advance the seed')
p.add_argument('--frames', type=int, default=36000)
p.add_argument('--recruit-cloud',action='store_true',help='fight optional Cloud, recruit and deploy him before continuing')
p.add_argument('--suspend-room', type=int, choices=range(1,7), help='save and reset once in this Traverse Town room')
a = p.parse_args()
if not 180 <= a.frames <= 120000*a.runs:
    p.error('--frames must be between 180 and 120000 per requested run')
if a.runs>1 and a.worlds!=3:
    p.error('--runs requires --worlds 3')
names = {}
for line in subprocess.check_output(['arm-none-eabi-nm', a.elf], text=True).splitlines():
    words = line.split()
    if len(words) == 3:
        names[words[2]] = int(words[0], 16)
keys = ('gCurrentMode', 'gCurrentModeUpdate', 'gPendingMode', 'sNativeMode', 'NativeUpdate', 'gFieldState', 'gMapFloorState', 'gTaskDescMapRnd',
        'gTaskDescMapDoor', 'gNativeBusy', 'gNativeEnemyFrames', 'gNativeParty', 'gNativeGuard', 'gNativeThreats', 'sPartyAction', 'gNativePreview',
        'gNativeRouteCost', 'gNativeMoveLeft', 'gNativeActionLeft', 'gNativeClimbing', 'gNativeCureTarget',
        'sRoutePos', 'sCursorX', 'sCursorY', 'gMapRoomState', 'sMapCells', 'sMapPlatforms', 'gCellMasks', 'sEnemyTasks', 'gNativeDeck', 'gNativePartyHealth', 'gNativeKills', 'gNativeResult', 'gNativeReward', 'sColliderPoolObstacle', 'gNativeFloor', 'gNativeSaveNotice', 'gNativeProgressReward', 'sProgressHero', 'sProgressKind', 'sAssemblyChoice', 'gNativeRoster', 'gNativeAssembly', 'sPartyPos', 'gNativeEnemyHp', 'gNativeEnemyCharge')
out = Path(a.output).resolve()
out.mkdir(parents=True, exist_ok=True)
header = f'local goalRuns={a.runs}\nlocal recruitCloud={str(a.recruit_cloud).lower()}\nlocal suspendRoom={a.suspend_room or 0}\nlocal goalRoom={a.rooms}\nlocal goalFrames={a.frames}\nlocal goalWorlds={a.worlds or 0}\n' + ''.join(f'local {key}=0x{names[key]:08x}\n' for key in keys)
script = header + Path(
    'tests/tactics_traversal_probe.lua').read_text().replace('-- @GEOMETRY@', Path('tests/tactics_traversal_geometry.lua').read_text()).replace('-- @REGIONS@', Path('tests/tactics_traversal_regions.lua').read_text()).replace('@OUTPUT@', str(out))

# Tie replay evidence to the exact executable and driver, including helper code.
elf = Path(a.elf).resolve()
rom = elf.with_suffix('.gba')
if not rom.is_file():
    p.error('matching built .gba is required beside the ELF')
if 'emu:write' in script or 'emu.write' in script:
    p.error('input-only traversal cannot contain emulated memory writes')
(out / 'traversal.lua').write_text(script)
(out / 'replay-metadata.json').write_text(json.dumps({
    'elf_sha256': hashlib.sha256(elf.read_bytes()).hexdigest(),
    'rom_sha256': hashlib.sha256(rom.read_bytes()).hexdigest(),
    'driver_sha256': hashlib.sha256(script.encode()).hexdigest(),
    'driver_logic_sha256': hashlib.sha256(script.replace(str(out), '@OUTPUT@').encode()).hexdigest(),
    'goal_worlds': a.worlds or 0,
    'goal_runs':a.runs,
    'goal_room': a.rooms,
    'frame_limit': a.frames,
    'input_only': True,
    'suspend_room': a.suspend_room,
    'recruit_cloud': a.recruit_cloud,
    'result': 'not yet observed; inspect traversal.txt',
}, indent=2) + '\n')
