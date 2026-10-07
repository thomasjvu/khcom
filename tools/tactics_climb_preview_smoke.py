"""Generate a native stair fixture; commands still use the original controller."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    v=line.split()
    if len(v)==3:names[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapRoomState','sMapCells','gNativePreview','gNativeRouteCost','gNativeMoveLeft','gNativeActionLeft','gNativeBusy','gNativeParty','gNativeClimbing','gNativeSaveNotice','sEnemyTasks')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out/'climb.lua').write_text(header+Path('tests/tactics_climb_preview_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
