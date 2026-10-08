"""Generate native encounter-clear upgrade and suspend fixtures."""
import argparse
import re
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('elf');p.add_argument('output');p.add_argument('--availability',action='store_true');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    parts=line.split()
    if len(parts)==3:names[parts[2]]=int(parts[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapRoomState','gMapFloorState','gNativeFloor',
 'gNativeProgressReward','gNativeSkillDamage','gNativeFireDamage','gNativeAssembly','sAssemblyTiles','sAssemblyPalette','gNativeRoster','gNativeParty','gNativeActionLeft',
 'gNativeEnemyHp','gNativeEnemyCharge','gNativeThreats','gNativePartyHealth',
 'sEnemyTasks','sPartyPos','gNativeSaveNotice','gNativeGuard','gNativeDeck',
 'sValueTiles','sValuePalette','sCardTiles','sCardPalettes','sProgressHero','sProgressKind')
hero_count=int(re.search(r'#define FIELD_HEROES (\d+)',Path('tactics/field_roster.h').read_text())[1])
header=f'local rosterClearedOffset={4+2*hero_count}\nlocal rosterPhaseOffset={6+2*hero_count}\nlocal rosterRewardOffset={7+2*hero_count}\nlocal rosterSleightOffset={4+hero_count}\n'+''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
script=header+Path('tests/tactics_reward_availability_smoke.lua' if a.availability else 'tests/tactics_progress_reward_smoke.lua').read_text().replace('@OUTPUT@',str(out))
rom=Path(a.elf).resolve().with_suffix('.gba')
if not rom.is_file():p.error('matching built ROM required beside ELF')
(out/'boss.lua').write_text(script)
(out/'fixture-metadata.json').write_text(json.dumps({
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'elf_sha256':hashlib.sha256(Path(a.elf).read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script.encode()).hexdigest(),
 'driver_logic_sha256':hashlib.sha256(script.replace(str(out),'@OUTPUT@').encode()).hexdigest(),
 'explicit_memory_fixtures':True,
 'expected_checks':7 if a.availability else 14,
 'result':'not yet observed; inspect availability.txt' if a.availability else 'not yet observed; inspect progress.txt'
},indent=2)+'\n')
