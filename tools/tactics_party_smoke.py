"""Generate native party/card/save integration replay for mGBA."""
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
keys=('gNativeFriendPose','gNativeCureHeal','gNativeCureChoice','gNativeCureTarget','gNativeThreats','sPartyPos','gNativePartyHealth','gNativeResult','gNativeParty','gNativeMoveLeft','gNativeActionLeft','gNativeGuard','gNativeDeck','gNativeSaveNotice','gGameState','gFieldState')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
template=Path('tests/tactics_party_smoke.lua').read_text()
(out/'party.lua').write_text(header+template.replace('@OUTPUT@',str(out)))

(out/'metadata.json').write_text(json.dumps(dict(
 explicit_memory_fixture=True,expected_checks=44,
 fixture='Health/positions/deck selection and newest-slot corruption; native deployment, movement, Cure/Guard, save/reset and party selection.',
 rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),
 driver_sha256=hashlib.sha256((out/'party.lua').read_bytes()).hexdigest(),
 result='not observed; inspect party.txt'),indent=2)+'\n')
