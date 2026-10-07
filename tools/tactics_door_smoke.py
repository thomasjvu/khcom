"""Generate a door approach fixture that crosses using native movement input."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    v=line.split()
    if len(v)==3:names[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapFloorState','gNativeMoveLeft','gNativeActionLeft','gNativeParty','gNativeTurn','gNativeRoomVisits','gTaskDescMapRnd','gTaskDescMapDoor')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out/'doors.lua').write_text(header+Path('tests/tactics_door_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
