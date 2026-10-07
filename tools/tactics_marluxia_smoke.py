"""Generate explicit Marluxia field presentation and spell fixtures."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    parts=line.split()
    if len(parts)==3:names[parts[2]]=int(parts[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapRoomState','gMapFloorState','gNativeFloor',
 'gNativeMarlReady','gNativeMarlPose','sMarlTiles','sMarlPalette',
 'gNativeEnemyHp','gNativeEnemyCharge','gNativeThreats','gNativePartyHealth',
 'sEnemyTasks','sPartyPos','gNativeSaveNotice','gNativeGuard','gNativeDeck',
 'sValueTiles','sValuePalette','sCardTiles','sCardPalettes')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out/'boss.lua').write_text(header+Path('tests/tactics_marluxia_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
