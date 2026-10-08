"""Generate Climb/Drop menu checks from an explicit saved-room approach."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--drop',action='store_true');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gNativeBusy','gFieldState','gNativeAssembly','gNativeProgressReward','gNativeMoveLeft','gNativeActionLeft','gNativeMenu','gNativeMenuChoice')
header=f'local dropTest={str(a.drop).lower()}\n'+''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)
script=header+Path('tests/tactics_ledge_menu_smoke.lua').read_text().replace('@OUTPUT@',str(out))
assert 'emu:write' not in script
(out/'test.lua').write_text(script)
(out/'metadata.json').write_text(json.dumps(dict(rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),expected_checks=8,explicit_saved_room_approach=True,input_only_after_boot=True,result='not yet observed'),indent=2)+'\n')
