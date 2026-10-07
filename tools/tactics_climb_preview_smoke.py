"""Generate a native stair fixture; commands still use the original controller."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    v=line.split()
    if len(v)==3:names[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapRoomState','sMapCells','sClimbPreviewPos','sClimbReachPos','gNativeClimbReachMask','gNativePreview','gNativeRouteCost','gNativeMoveLeft','gNativeActionLeft','gNativeBusy','gNativeParty','gNativeClimbing','gNativeSaveNotice','sEnemyTasks')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
script=header+Path('tests/tactics_climb_preview_smoke.lua').read_text().replace('@OUTPUT@',str(out))
(out/'climb.lua').write_text(script)
elf=Path(a.elf).resolve();rom=elf.with_suffix('.gba')
(out/'fixture-metadata.json').write_text(json.dumps({
 'elf_sha256':hashlib.sha256(elf.read_bytes()).hexdigest(),
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script.encode()).hexdigest(),
 'explicit_position_fixture':True,
 'expected_checks':32,
 'result':'not yet observed; inspect climb-preview.txt'
},indent=2)+'\n')
