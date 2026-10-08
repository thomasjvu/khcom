"""Generate native card-stock/sleight/save replay for mGBA."""
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
keys=('gNativeParty','gNativeRoster','gNativeAssembly','gNativeSleightHeal','gNativeSleightDamage','gNativeEnemyHp','gNativePartyHealth','gGameState','gNativeDeck','gNativeSleights','gNativeActionLeft','gNativeSaveNotice','gNativeKills','gFieldState','sEnemyTasks')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out/'sleights.lua').write_text(header+Path('tests/tactics_sleight_smoke.lua').read_text().replace('@OUTPUT@',str(out)))

(out/'sleight-area.lua').write_text(header+Path('tests/tactics_sleight_area_smoke.lua').read_text().replace('@OUTPUT@',str(out)))

(out/'recipes.lua').write_text(header+Path('tests/tactics_recipe_smoke.lua').read_text().replace('@OUTPUT@',str(out)))

(out/'recipe-save.lua').write_text(header+Path('tests/tactics_recipe_save_smoke.lua').read_text().replace('@OUTPUT@',str(out)))

script=header+Path('tests/tactics_donald_healing_smoke.lua').read_text().replace('@OUTPUT@',str(out))
(out/'donald-healing.lua').write_text(script)
rom=Path(a.elf).resolve().with_suffix('.gba')
(out/'donald-healing-metadata.json').write_text(json.dumps({
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script.encode()).hexdigest(),
 'explicit_health_card_upgrade_enemy_position_fixtures':True,
 'expected_checks':17,
 'result':'not yet observed; inspect donald-healing.txt',
},indent=2)+'\n')

for script_name, expected in (("sleights",14),("sleight-area",12),("recipes",16),("recipe-save",9)):
 source=out/(script_name+".lua")
 (out/(script_name+"-metadata.json")).write_text(json.dumps({
  "rom_sha256":hashlib.sha256(rom.read_bytes()).hexdigest(),
  "driver_sha256":hashlib.sha256(source.read_bytes()).hexdigest(),
  "explicit_card_health_position_fixtures":True,
  "expected_checks":expected,
  "result":"not yet observed; inspect "+script_name+".txt",
 },indent=2)+"\n")

# Each caster is deployed through the native assembly menu.
for hero,name in ((1,'donald'),(2,'goofy'),(3,'cloud'),(4,'rally')):
 script=header+f'local testAreaHero={hero}\n'+Path('tests/tactics_sleight_area_smoke.lua').read_text().replace('@OUTPUT@',str(out))
 script=script.replace('/sleight-area.txt',f'/{name}-area.txt')
 (out/f'{name}-area.lua').write_text(script)
 (out/f'{name}-area-metadata.json').write_text(json.dumps(dict(rom_sha256=hashlib.sha256(rom.read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),explicit_card_enemy_position_hp_fixtures=True,native_party_assembly=True,explicit_cloud_unlock_fixture=hero==3,expected_checks=12,result='pending'),indent=2)+'\n')
