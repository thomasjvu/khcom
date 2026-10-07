"""Probe original static Castle pillar interaction using an explicit suspend fixture."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--jump',action='store_true');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    words=line.split()
    if len(words)==3:names[words[2]]=int(words[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('sColliderPoolObstacle','gFieldState','sPartyPos','gNativeDeck','gNativeActionLeft','gTaskDescMapGmk00','gNativeBusy','gNativeMoveLeft')
script=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)+Path('tests/tactics_castle_jump_smoke.lua' if a.jump else 'tests/tactics_castle_break_smoke.lua').read_text().replace('@OUTPUT@',str(out))
(out/'break.lua').write_text(script)
(out/'fixture-metadata.json').write_text(json.dumps(dict(rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),expected_checks=8 if a.jump else 7,explicit_suspend_position_and_card_fixtures=True,scope='Pillar jump and crossing from explicit approach' if a.jump else 'Static pillar identification and native attack resource/collider behavior; does not prove route completion',result='not yet observed'),indent=2)+'\n')
