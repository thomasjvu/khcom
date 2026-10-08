"""Verify health glyph RAM and uploaded GBA tiles against original font data."""
import argparse,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
symbols={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
 v=line.split()
 if len(v)==3:symbols[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gGameState','gNativePartyHealth','sHudTiles','gDebugFont0Tiles','gModeVBlankCallback','sHudPending','gNativeActionLeft')
header=''.join(f'local {k}=0x{symbols[k]:08x}\n' for k in keys)
(out/'hud.lua').write_text(header+Path('tests/tactics_hud_smoke.lua').read_text().replace('@OUTPUT@',str(out)))
