"""Generate disclosed original-pillar landing prediction fixture."""
import argparse, hashlib, json, subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');p.add_argument('--saved-room',required=True,type=Path);p.add_argument('--tall',action='store_true',help='disclosed tall collider side-contact fixture');a=p.parse_args()
names={x[2]:int(x[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(x:=line.split())==3}
keys=('sColliderPoolObstacle','gFieldState','sPartyPos','gNativeMenu','gNativeBusy','gNativeMoveLeft','gNativeActionLeft','gNativeJumpPrediction','gNativeJumpLanding')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_jump_pillar_smoke.lua').read_text().replace('@OUTPUT@',str(out))
if a.tall:
 s=s.replace("  x=emu:read32(pillar+4)+9216;","  emu:write32(pillar+20,emu:read32(pillar+20)+16384)\n  x=emu:read32(pillar+4)+9216;")
 s=s.replace("==emu:read32(pillar+12)-emu:read32(pillar+20),'native jump lands on original pillar top'",">emu:read32(pillar+12)-emu:read32(pillar+20),'tall side-contact cannot land on elevated prop top'")
(out/'test.lua').write_text(s)
(out/'fresh.sav').write_bytes(a.saved_room.read_bytes())
(out/'metadata.json').write_text(json.dumps(dict(explicit_saved_room_position_budget_fixture=True,expected_checks=5,tall_collider_fixture=a.tall,rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),saved_room_sha256=hashlib.sha256(a.saved_room.read_bytes()).hexdigest(),result='pending'),indent=2)+'\n')
