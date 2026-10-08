"""Disclosed saved-room reproduction; native chest strike and door traversal."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--seed-save',required=True,type=Path);a=p.parse_args()
out=Path(a.output).resolve()
subprocess.run(['python3','tools/tactics_traversal_probe.py',a.elf,str(out),'--worlds','3','--all-rooms','--collect-chests','--frames','6000'],check=True)
subprocess.run(['cc','-std=c89','-Wall','-Wextra','-Werror','-I','tactics','tactics/field_save.c','tactics/field_deck.c','tactics/field_roster.c','tools/tactics_agrabah_room11_fixture.c','-o','build/tactics/agrabah_room11_fixture'],check=True)
subprocess.run(['build/tactics/agrabah_room11_fixture',str(a.seed_save),str(out/'fresh.sav')],check=True)
p=out/'traversal.lua';s=p.read_text().replace('routeStep=1','routeStep=14')
s+="""
callbacks:add('frame',function()
 if done or f<180 then return end
 if emu:read16(gNativeFloor)==1 and emu:read8(gMapFloorState+6)==10 then
  finish(emu:read16(gNativeChests)==6,'native chest count reaches six and room11 exits through its original door')
 end
end)
"""
assert 'emu:write' not in s
p.write_text(s)
m=json.loads((out/'replay-metadata.json').read_text());m.update(driver_sha256=hashlib.sha256(s.encode()).hexdigest(),explicit_saved_room_fixture=True,seed_save_sha256=hashlib.sha256(a.seed_save.read_bytes()).hexdigest(),scope='Recorded second-seed Agrabah room11 approach/deck/party, cleared encounter; six chests and exit to room10 required. Native input after boot.',result='pending')
(out/'replay-metadata.json').write_text(json.dumps(m,indent=2)+'\n')
