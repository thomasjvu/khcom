"""Captured Castle stair position; corrected native explorer must leave room5."""
import argparse,subprocess,hashlib,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--seed-save',required=True);a=p.parse_args();out=Path(a.output).resolve()
subprocess.run(['python3','tools/tactics_traversal_probe.py',a.elf,str(out),'--worlds','3','--all-rooms','--frames','12000'],check=True)
subprocess.run(['cc','-std=c89','-Wall','-Wextra','-Werror','-I','tactics','tactics/field_save.c','tactics/field_deck.c','tactics/field_roster.c','tools/tactics_castle_climb_fixture.c','-o','build/tactics/castle_climb_fixture'],check=True)
subprocess.run(['build/tactics/castle_climb_fixture',a.seed_save,str(out/'fresh.sav')],check=True)
p=out/'traversal.lua';s=p.read_text().replace('routeStep=1','routeStep=16')
s+='''
local originalStairCompleted=false
callbacks:add('frame',function()
 if done or f<180 then return end
 local field=emu:read32(gFieldState)
 if emu:read16(gNativeFloor)==2 and emu:read8(gMapFloorState+6)==5 and emu:read32(field+0x20)==0 and emu:read32(field+0x24)==0 then originalStairCompleted=true end
 if emu:read16(gNativeFloor)==2 and emu:read8(gMapFloorState+6)==6 then finish(originalStairCompleted,'captured stair completes and native explorer exits Castle room5 to room6') end
end)
'''
assert 'emu:write' not in s;p.write_text(s)
p=out/'replay-metadata.json';m=json.loads(p.read_text());m.update(driver_sha256=hashlib.sha256(s.encode()).hexdigest(),explicit_saved_stair_position_cleared_encounters=True,scope='Third-seed Castle room5 isolated saved stair position, encounters cleared; native input stair completion and original exit to room6 required',result='pending');p.write_text(json.dumps(m,indent=2)+'\n')
