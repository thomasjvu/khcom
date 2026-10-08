"""Generate native alternating-slot saves and an explicit corruption fixture."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('elf')
p.add_argument('output')
a = p.parse_args()
names = {w[2]: int(w[0], 16) for line in subprocess.check_output(
    ['arm-none-eabi-nm', a.elf], text=True).splitlines() if len(w := line.split()) == 3}
keys = ('gNativeDeck', 'gNativeRoster', 'gNativePartyHealth', 'sPartyPos',
        'gNativeMoveLeft', 'gNativeActionLeft', 'gNativeSaveNotice', 'sSaveSlot')
out = Path(a.output).resolve()
out.mkdir(parents=True, exist_ok=True)
roster_bytes=next(int(w[1],16) for line in subprocess.check_output(['arm-none-eabi-nm','-S',a.elf],text=True).splitlines() if len(w:=line.split())==4 and w[3]=='gNativeRoster')
script = f'local rosterBytes={roster_bytes}\n'+''.join(f'local {k}=0x{names[k]:x}\n' for k in keys) + Path(
    'tests/tactics_native_sram_smoke.lua').read_text().replace('@OUTPUT@', str(out))
(out / 'test.lua').write_text(script)
(out / 'metadata.json').write_text(json.dumps({
    'rom_sha256': hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),
    'driver_sha256': hashlib.sha256(script.encode()).hexdigest(),
    'expected_checks': 12, 'explicit_sram_corruption_fixture': True,
    'scope': 'Native save inputs, alternating generations, exact reset resume and corrupt-newest fallback',
    'result': 'not yet observed'}, indent=2) + '\n')
