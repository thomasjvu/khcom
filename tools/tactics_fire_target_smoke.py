"""Generate explicit two-enemy Fire selection fixtures for native mGBA."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
symbols={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    fields=line.split()
    if len(fields)==3:symbols[fields[2]]=int(fields[0],16)
keys=('gFieldState','sEnemyTasks','gNativeEnemyHp','gNativeDeck','gNativeFireTarget','gNativeFireChoice','gNativeFireDamage','gNativeMoveLeft','gNativeActionLeft')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
header=''.join(f'local {k}=0x{symbols[k]:08x}\n' for k in keys)
(out/'fire-target.lua').write_text(header+Path('tests/tactics_fire_target_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
