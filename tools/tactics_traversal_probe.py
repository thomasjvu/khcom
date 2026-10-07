"""Generate a bounded input-only room traversal probe for mGBA.

Reads diagnostics and original door geometry; never writes emulated memory.
A failed probe is navigation evidence, not a claim that the room is impossible.
"""
import argparse
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('elf')
p.add_argument('output')
p.add_argument('--rooms', type=int, choices=range(1, 8), default=1)
p.add_argument('--worlds', type=int, choices=range(1, 4))
p.add_argument('--frames', type=int, default=36000)
a = p.parse_args()
if not 180 <= a.frames <= 120000:
    p.error('--frames must be between 180 and 120000')
names = {}
for line in subprocess.check_output(['arm-none-eabi-nm', a.elf], text=True).splitlines():
    words = line.split()
    if len(words) == 3:
        names[words[2]] = int(words[0], 16)
keys = ('gCurrentMode', 'gCurrentModeUpdate', 'gPendingMode', 'sNativeMode', 'NativeUpdate', 'gFieldState', 'gMapFloorState', 'gTaskDescMapRnd',
        'gTaskDescMapDoor', 'gNativeBusy', 'gNativeEnemyFrames', 'gNativeParty', 'gNativeGuard', 'gNativeThreats', 'sPartyAction', 'gNativePreview',
        'gNativeRouteCost', 'gNativeMoveLeft', 'gNativeActionLeft', 'gNativeClimbing',
        'sRoutePos', 'sCursorX', 'sCursorY', 'gMapRoomState', 'sMapCells', 'sMapPlatforms', 'gCellMasks', 'sEnemyTasks', 'gNativeDeck', 'gNativePartyHealth', 'gNativeKills', 'gNativeResult', 'sColliderPoolObstacle', 'gNativeFloor')
out = Path(a.output).resolve()
out.mkdir(parents=True, exist_ok=True)
header = f'local goalRoom={a.rooms}\nlocal goalFrames={a.frames}\nlocal goalWorlds={a.worlds or 0}\n' + ''.join(f'local {key}=0x{names[key]:08x}\n' for key in keys)
(out / 'traversal.lua').write_text(header + Path(
    'tests/tactics_traversal_probe.lua').read_text().replace('-- @GEOMETRY@', Path('tests/tactics_traversal_geometry.lua').read_text()).replace('-- @REGIONS@', Path('tests/tactics_traversal_regions.lua').read_text()).replace('@OUTPUT@', str(out)))
