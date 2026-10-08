"""Generate an explicit native upper-ledge route regression fixture."""
import argparse
import subprocess
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('elf'); p.add_argument('output'); p.add_argument('--props', action='store_true'); a = p.parse_args()
names = {}
for line in subprocess.check_output(['arm-none-eabi-nm', a.elf], text=True).splitlines():
    words = line.split()
    if len(words) == 3:
        names[words[2]] = int(words[0], 16)
keys = ('gFieldState', 'gMapRoomState', 'sMapCells', 'gCellMasks',
        'gNativeMoveLeft', 'gNativeActionLeft', 'gNativeBusy', 'gNativePreview', 'gNativeRouteCost', 'sColliderPoolObstacle', 'task_fld_sora_1', 'sEnemyTasks')
out = Path(a.output).resolve(); out.mkdir(parents=True, exist_ok=True)
header = f'local propsOnly={str(a.props).lower()}\n' + ''.join(f'local {key}=0x{names[key]:08x}\n' for key in keys)
(out / 'ledge.lua').write_text(header + Path(
    'tests/tactics_ledge_smoke.lua').read_text().replace('@OUTPUT@', str(out)))
