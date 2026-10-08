"""Generate explicit original launcher landing regression."""
import argparse,subprocess,hashlib,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--seed-save',required=True,type=Path);p.add_argument('--pad',type=int,choices=(0,1,2),required=True);a=p.parse_args()
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
subprocess.run(['cc','-std=c89','-Wall','-Wextra','-Werror','-I','tactics','tactics/field_save.c','tactics/field_deck.c','tactics/field_roster.c','tools/tactics_agrabah_room10_fixture.c','-o','build/tactics/agrabah_room10_fixture'],check=True)
subprocess.run(['build/tactics/agrabah_room10_fixture',str(a.seed_save),str(out/'fresh.sav'),str(a.pad)],check=True)
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gFieldState','gMapRoomState','gNativeMenu','gNativeJumpPrediction','gNativeJumpLanding','gNativeMoveLeft','gNativeActionLeft','gNativeBusy','sUiGlyphs','gNativeAssembly','gNativeProgressReward')
helper='local function hudText'+Path('tests/tactics_jump_forecast_smoke.lua').read_text().split('local function hudText',1)[1].split("callbacks:add('frame'",1)[0]
s=f'local targetHeight={(0,24576,45056)[a.pad]}\n'+''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+helper+Path('tests/tactics_pad_jump_smoke.lua').read_text().replace('@OUTPUT@',str(out))
assert 'emu:write' not in s
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(expected_checks=9,pad_index=a.pad,explicit_saved_pad_position_party_deck_fixture=True,input_only_after_boot=True,seed_save_sha256=hashlib.sha256(a.seed_save.read_bytes()).hexdigest(),rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),result='pending'),indent=2)+'\n')
