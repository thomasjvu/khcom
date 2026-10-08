"""Generate input-only projected diagonal route checks for mGBA."""
import argparse
import subprocess
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('elf'); p.add_argument('output'); a = p.parse_args()
names = {}
for line in subprocess.check_output(['arm-none-eabi-nm', a.elf], text=True).splitlines():
    words = line.split()
    if len(words) == 3:
        names[words[2]] = int(words[0], 16)
keys = ('gFieldState', 'gNativeMoveLeft', 'gNativeActionLeft', 'gNativeBusy',
        'gNativePreview', 'gNativeRouteCost', 'sRoutePos', 'sCursorX', 'sCursorY')
out = Path(a.output).resolve(); out.mkdir(parents=True, exist_ok=True)
header = ''.join(f'local {key}=0x{names[key]:08x}\n' for key in keys)
(out / 'diagonals.lua').write_text(header + Path(
    'tests/tactics_diagonal_smoke.lua').read_text().replace('@OUTPUT@', str(out)))
