"""Generate disclosed native Cloud deployment/attack animation fixture."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--aladdin',action='store_true');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeRoster','gNativeParty','gNativeFriendPose','gNativeFireDamage','gNativeMoveLeft','gNativeActionLeft','gNativeEnemyHp','gFieldState','sFriends','sEnemyTasks')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_cloud_party_action_smoke.lua').read_text().replace('@OUTPUT@',str(out))
if a.aladdin:
 import re
 s=re.sub(r'f==([0-9]+)',lambda m:'f=='+str(int(m[1])+80 if int(m[1])>=280 else int(m[1])),s)
 s=s.replace('f==200 or f==240','f==200')
 s=s.replace('gNativeRoster,31','gNativeRoster,63').replace('gNativeRoster+2)==3','gNativeRoster+2)==5')
 s=s.replace('local p=emu:read32(gFieldState)','emu:write16(gNativeMoveLeft,2)\n  local p=emu:read32(gFieldState)')
 s=s.replace('gNativeFireDamage)==17','gNativeFireDamage)==14').replace('gNativeEnemyHp)==13','gNativeEnemyHp)==16')
 s=s.replace('Cloud','Aladdin').replace('without movement','and restores one movement after hitting')
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(explicit_memory_fixture=True,fixture='Unlock Aladdin or Cloud; place original enemies within/outside sword range. Aladdin starts with two movement. Native deployment, card play and damage.',rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),expected_checks=9,result='not observed'),indent=2)+'\n')
