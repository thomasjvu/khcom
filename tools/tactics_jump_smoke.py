"""Generate input-only native full-height and bounded-range jump checks for mGBA."""
import argparse
import subprocess
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('--preview-stairs', action='store_true'); p.add_argument('elf'); p.add_argument('output'); p.add_argument('--stairs', action='store_true'); p.add_argument('--party', type=int, choices=(1,2)); a = p.parse_args()
names = {}
for line in subprocess.check_output(['arm-none-eabi-nm', a.elf], text=True).splitlines():
    words = line.split()
    if len(words) == 3:
        names[words[2]] = int(words[0], 16)
keys = ('gFieldState', 'gNativeMoveLeft', 'gNativeActionLeft', 'gNativeBusy',
        'gNativePreview', 'gNativeRouteCost', 'sRoutePos', 'sCursorX', 'sCursorY',
        'gMapRoomState', 'sMapCells', 'sEnemyTasks', 'gNativeClimbing', 'gNativeFriendPose', 'gNativeParty', 'gNativeMenu', 'gNativeDirection', 'gNativeJumpPrediction', 'sUiGlyphs')
out = Path(a.output).resolve(); out.mkdir(parents=True, exist_ok=True)
header = ''.join(f'local {key}=0x{names[key]:08x}\n' for key in keys)
if a.party: header += f'local testParty={a.party}\n'
script=Path('tests/tactics_jump_stair_smoke.lua' if a.stairs or a.preview_stairs else 'tests/tactics_jump_smoke.lua').read_text()
if a.preview_stairs:
 glyphHelper=Path('tests/tactics_jump_forecast_smoke.lua').read_text().split('local function hudText',1)[1].split("callbacks:add('frame'",1)[0]
 script='local function hudText'+glyphHelper+script
 script=script.replace(' f=f+1',' f=f+1\n if f==120 then emu:setKeys(8) end\n if f==124 then emu:setKeys(0) end')
 script=script.replace('(y*16+14)*256-lower','(y*16+22)*256-lower')
 script=script.replace('  emu:setKeys(66)', '  emu:setKeys(4)')
 script=script.replace(' if f==184 then emu:setKeys(0) end', ''' if f==184 or f==244 or f==284 or f==324 or f==364 then emu:setKeys(0) end
 if f==240 then emu:setKeys(1) end
 if f==280 then emu:setKeys(256) end
 if f==320 then emu:setKeys(64) end
 if f==340 then
  check(emu:read16(gNativeMenu)==6 and emu:read16(gNativeDirection)==64,'native Jump preview points toward stairs')
  check(emu:read16(gNativeJumpPrediction)==3,'preview identifies native stair attachment')
  check(hudText(4,0,'ATTACH TO STAIRS'),'native HUD displays stair attachment forecast')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'stair forecast spends no resources')
  emu:screenshot('@OUTPUT@/stair-preview.png')
 end
 if f==360 then emu:setKeys(1) end''')
(out / 'jumps.lua').write_text(header+script.replace('@OUTPUT@',str(out)))
