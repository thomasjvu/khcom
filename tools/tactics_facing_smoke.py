"""Generate input-only native turn-in-place verification."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gNativeMoveLeft','gNativeActionLeft','gNativeBusy')
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_facing_eight_smoke.lua').read_text().replace('@OUTPUT@',str(out))
assert 'emu:write' not in s
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(input_only=True,expected_checks=9,rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),result='not yet observed'),indent=2)+'\n')
