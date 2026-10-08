"""Generate disclosed native Cloud deployment/attack animation fixture."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--aladdin',action='store_true');p.add_argument('--case',choices=('hit','boundary','edge','height','break','cap','save'),default='hit');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeRoster','gNativeParty','gNativeFriendPose','gNativeFireDamage','gNativeMoveLeft','gNativeActionLeft','gNativeEnemyHp','gFieldState','sFriends','sEnemyTasks','gNativeDeck','gNativeBreaks','gNativeSaveNotice','gNativePartyHealth')
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
 if a.case=='boundary':
  s=s.replace('i==0 and 8192','i==0 and 12032').replace('emu:write16(gNativeEnemyHp+i*2,30)','emu:write32(w+16,emu:read32(p+0x20)+6144);emu:write16(gNativeEnemyHp+i*2,30)')
 if a.case=='edge':
  s=s.replace('i==0 and 8192','i==0 and 12288')
 if a.case=='height':
  s=s.replace('emu:write16(gNativeEnemyHp+i*2,30)', 'emu:write32(w+16,emu:read32(p+0x20)+6400);emu:write16(gNativeEnemyHp+i*2,30)')
 if a.case=='break':
  s=s.replace('emu:write16(gNativeMoveLeft,2)','emu:write8(gNativeDeck+24,1);emu:write16(gNativeMoveLeft,2)')
 if a.case in ('edge','height','break'):
  s=s.replace('gNativeFireDamage)==14','gNativeFireDamage)==0').replace('gNativeEnemyHp)==16','gNativeEnemyHp)==30').replace('gNativeMoveLeft)==3','gNativeMoveLeft)==2')
  s=s.replace('previews sword damage at ranged target','previews no damage for '+a.case).replace('resolves one sword hit','preserves target HP for '+a.case).replace('and restores one movement after hitting','without restoring movement for '+a.case)
 if a.case=='cap':
  s=s.replace('emu:write16(gNativeMoveLeft,2)','emu:write16(gNativeMoveLeft,3)').replace('and restores one movement after hitting','and caps movement at three')
 if a.case=='save':
  s=s.replace('  out:close()','')
  s=s.replace('end)\n',Path('tests/tactics_aladdin_party_save_tail.lua').read_text())
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(explicit_memory_fixture=True,fixture='Unlock Aladdin or Cloud; place original enemies within/outside sword range. Aladdin starts with two movement. Native deployment, card play and damage.',rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),expected_checks=14 if a.case=='save' else 9,case=a.case,result='not observed'),indent=2)+'\n')
