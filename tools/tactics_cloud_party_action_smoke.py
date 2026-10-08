"""Generate disclosed native Cloud deployment/attack animation fixture."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeRoster','gNativeParty','gNativeFriendPose','gNativeFireDamage','gNativeMoveLeft','gNativeActionLeft','gNativeEnemyHp','gFieldState','sFriends','sEnemyTasks')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_cloud_party_action_smoke.lua').read_text().replace('@OUTPUT@',str(out));(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(explicit_memory_fixture=True,fixture='Unlock Cloud; place original enemies within/outside sword range. Native deployment, card play and damage.',rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),expected_checks=9,result='not observed'),indent=2)+'\n')
