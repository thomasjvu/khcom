"""Check prop-top party switching using explicit saved-room/card/health fixtures."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    words=line.split()
    if len(words)==3:names[words[2]]=int(words[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gCurrentMode','gCurrentModeUpdate','sNativeMode','NativeUpdate','sColliderPoolObstacle','gFieldState','sPartyPos','gNativeDeck','gNativeActionLeft','gTaskDescMapGmk00','gNativeBusy','gNativeMoveLeft','gNativePreview','gNativeRouteCost','gNativeSaveNotice','gNativeParty','gNativePartyHealth')
script=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)+Path('tests/tactics_prop_party_smoke.lua').read_text().replace('@OUTPUT@',str(out))
(out/'break.lua').write_text(script)
(out/'fixture-metadata.json').write_text(json.dumps(dict(rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),expected_checks=14,explicit_suspend_position_and_card_fixtures=True,scope='Original pillar jump/top walking and party-switch support/resource preservation; explicit approach/cards/revived companions, native movement and switching',result='not yet observed'),indent=2)+'\n')
