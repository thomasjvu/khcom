"""Isolated saved stair attachment; native traversal to a connected endpoint."""
import argparse,subprocess,json,hashlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--previous-policy',action='store_true');p.add_argument('elf');p.add_argument('output');p.add_argument('--seed-save',required=True);a=p.parse_args();out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
subprocess.run(['cc','-std=c89','-Wall','-Wextra','-Werror','-I','tactics','tactics/field_save.c','tactics/field_deck.c','tactics/field_roster.c','tools/tactics_castle_climb_fixture.c','-o','build/tactics/castle_climb_fixture'],check=True)
subprocess.run(['build/tactics/castle_climb_fixture',a.seed_save,str(out/'fresh.sav')],check=True)
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in ('gFieldState','gNativeEnemyFrames','gNativeBusy','gNativeTurn','gNativeMenu','gNativeMoveLeft'))+Path('tests/tactics_climb_endpoint_smoke.lua').read_text().replace('@OUTPUT@',str(out));s=s.replace('local bottom=emu:read32(field+0x24)','local bottom=z') if a.previous_policy else s;assert 'emu:write' not in s;(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(expected_checks=3,previous_direction_policy=a.previous_policy,explicit_saved_stair_position_cleared_encounters=True,input_only_after_boot=True,rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),result='pending'),indent=2)+'\n')
