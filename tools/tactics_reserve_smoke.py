"""Generate native reserve replacement regression with an explicit KO fixture."""
import argparse, hashlib, json, subprocess, re
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('elf');parser.add_argument('output');parser.add_argument('--aladdin',action='store_true');args=parser.parse_args()
symbols={words[2]:int(words[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',args.elf],text=True).splitlines() if len(words:=line.split())==3}
output=Path(args.output).resolve();output.mkdir(parents=True,exist_ok=True)
keys=('gNativeAssembly','gNativePartyHealth','gNativeRoster','gNativeParty')
hero_count=int(re.search(r'#define FIELD_HEROES (\d+)',Path('tactics/field_roster.h').read_text())[1])
script=f'local rosterHpOffset={9+2*hero_count}\n'+''.join(f'local {key}=0x{symbols[key]:x}\n' for key in keys)+Path('tests/tactics_reserve_smoke.lua').read_text().replace('@OUTPUT@',str(output))
if args.aladdin:
 script=script.replace('emu:write8(gNativePartyHealth+1,0)','emu:write8(gNativeRoster,55);emu:write8(gNativePartyHealth+1,0)')
 script=script.replace('gNativeRoster+2)==4','gNativeRoster+2)==5').replace('gNativePartyHealth+1)==64','gNativePartyHealth+1)==60').replace('Rally','Aladdin')
(output/'test.lua').write_text(script)
(output/'metadata.json').write_text(json.dumps(dict(explicit_memory_fixture=True,input_only=False,fixture='Donald and Goofy KO at initial assembly; native controls thereafter',expected_checks=7,rom_sha256=hashlib.sha256(Path(args.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),result='not yet observed'),indent=2)+'\n')
