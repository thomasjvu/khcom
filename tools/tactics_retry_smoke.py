"""Generate a bounded native retry/reset fixture with exact ROM evidence."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    parts=line.split()
    if len(parts)==3:names[parts[2]]=int(parts[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gNativeSeed','gNativeAssembly','gNativeResult','gNativeFloor','gNativeRoster','gNativeKills','gNativeChests','gGameState','gNativePartyHealth','gNativeParty','gNativeMoveLeft','gNativeActionLeft','gNativeDeck','gMapFloorState')
script=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)+Path('tests/tactics_retry_smoke.lua').read_text().replace('@OUTPUT@',str(out))
rom=Path(a.elf).resolve().with_suffix('.gba')
(out/'retry.lua').write_text(script)
(out/'fixture-metadata.json').write_text(json.dumps(dict(rom_sha256=hashlib.sha256(rom.read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),explicit_terminal_progression_fixture=True,expected_checks=11,result='not yet observed; inspect retry.txt'),indent=2)+'\n')
