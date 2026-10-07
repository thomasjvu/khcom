"""Generate native route input checks followed by explicit timeout fixtures."""
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
out = Path(a.output).resolve(); out.mkdir(parents=True, exist_ok=True)
keys = ('gNativePreview', 'gNativeRouteCost', 'gNativeMoveLeft', 'gNativeActionLeft',
        'gNativeBusy', 'gNativeCommands', 'gNativePartyHealth', 'gGameState', 'gFieldState', 'sRoutePos', 'sCursorX', 'sCursorY','sPathLength','sPathIndex','sFrames','sPathStartX','sPathStartY','sPlayerPath')
header = ''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
(out / 'routes.lua').write_text(header + Path('tests/tactics_route_smoke.lua').read_text().replace('@OUTPUT@', str(out)))
