"""Generate native card-stock/sleight/save replay for mGBA."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    v=line.split()
    if len(v)==3:names[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gNativeSleightDamage','gNativeEnemyHp','gNativePartyHealth','gGameState','gNativeDeck','gNativeSleights','gNativeActionLeft','gNativeSaveNotice','gNativeKills','gFieldState','sEnemyTasks')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out/'sleights.lua').write_text(header+Path('tests/tactics_sleight_smoke.lua').read_text().replace('@OUTPUT@',str(out)))

(out/'sleight-area.lua').write_text(header+Path('tests/tactics_sleight_area_smoke.lua').read_text().replace('@OUTPUT@',str(out)))

(out/'recipes.lua').write_text(header+Path('tests/tactics_recipe_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
