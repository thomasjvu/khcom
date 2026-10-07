"""Generate a native stair fixture; commands still use the original controller."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');modes=p.add_mutually_exclusive_group();modes.add_argument('--multi',action='store_true');modes.add_argument('--occupancy',action='store_true');modes.add_argument('--top',action='store_true');modes.add_argument('--walk',action='store_true');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    v=line.split()
    if len(v)==3:names[v[2]]=int(v[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapRoomState','sMapCells','sRoutePos','sCursorX','sCursorY','gNativeClimbReachMask','gNativePreview','gNativeRouteCost','gNativeMoveLeft','gNativeActionLeft','gNativeBusy','gNativeParty','gNativeClimbing','gNativeSaveNotice','sEnemyTasks','sPartyPos','gNativeReachCount','gNativeReachCost','sPlayerPath','sPlayerEdge')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
header+="""local function previewPos()
 local x=emu:read16(sCursorX);local y=emu:read16(sCursorY)
 if x>=32768 then x=x-65536 end
 if y>=32768 then y=y-65536 end
 return sRoutePos+(emu:read16(gNativePreview)==3 and 7+(y+3)*7+x+3 or y+3)*16
end
"""
script=header+Path('tests/tactics_climb_walk_smoke.lua' if a.walk else 'tests/tactics_climb_top_smoke.lua' if a.top else 'tests/tactics_climb_occupancy_smoke.lua' if a.occupancy else 'tests/tactics_climb_route_smoke.lua' if a.multi else 'tests/tactics_climb_preview_smoke.lua').read_text().replace('@OUTPUT@',str(out))
(out/'climb.lua').write_text(script)
elf=Path(a.elf).resolve();rom=elf.with_suffix('.gba')
(out/'fixture-metadata.json').write_text(json.dumps({
 'elf_sha256':hashlib.sha256(elf.read_bytes()).hexdigest(),
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script.encode()).hexdigest(),
 'explicit_position_fixture':True,
 'expected_checks':27 if a.walk else 14 if a.top else 15 if a.occupancy else 25 if a.multi else 32,
 'multi_segment':a.multi,
 'explicit_actor_occupancy_fixtures':a.occupancy,
 'top_boundary':a.top,
 'combined_descent_walk':a.walk,
 'result':'not yet observed; inspect climb-preview.txt'
},indent=2)+'\n')
