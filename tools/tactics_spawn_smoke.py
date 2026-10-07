"""Generate native tactical spawn and Fire targeting checks."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--legacy',action='store_true');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    v=line.split()
    if len(v)==3:names[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gNativeFireTarget','gNativeFireDamage','gFieldState','gNativeEnemyHp','gNativeEnemyKind','sEnemyTasks','gNativeKills','gNativeActionLeft','gNativeSaveNotice')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out/'spawn.lua').write_text(header+Path('tests/tactics_spawn_legacy_smoke.lua' if a.legacy else 'tests/tactics_spawn_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
