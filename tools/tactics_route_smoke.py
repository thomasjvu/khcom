"""Generate native route input checks followed by explicit timeout fixtures."""
import argparse
import hashlib
import json
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
script=header + Path('tests/tactics_route_smoke.lua').read_text().replace('@OUTPUT@', str(out))
(out / 'routes.lua').write_text(script)
rom=Path(a.elf).resolve().with_suffix('.gba')
if not rom.is_file():p.error('matching built ROM required beside ELF')
(out/'fixture-metadata.json').write_text(json.dumps({
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'elf_sha256':hashlib.sha256(Path(a.elf).read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script.encode()).hexdigest(),
 'driver_logic_sha256':hashlib.sha256(script.replace(str(out),'@OUTPUT@').encode()).hexdigest(),
 'explicit_memory_fixtures':True,'input_only':False,'expected_checks':33,
 'scope':'Controller-only route input/arrival/rejection followed by explicit frozen/progress timeout fixtures',
 'result':'not yet observed; inspect routes.txt'
},indent=2)+'\n')
