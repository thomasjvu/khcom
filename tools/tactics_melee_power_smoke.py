"""Generate disclosed native sword power/HUD regression."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--facing',action='store_true');p.add_argument('--rally',action='store_true');p.add_argument('--trace',action='store_true');p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeParty','gNativeDeck','gNativeRoster','gNativeEnemyHp','gNativeActionLeft','gNativeMoveLeft','gNativeMenu','gNativeBreaks','sEnemyTasks','gFieldState','sUiGlyphs','gMapRoomState','gNativeBusy')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
helper='local function hudText'+Path('tests/tactics_jump_forecast_smoke.lua').read_text().split('local function hudText',1)[1].split("callbacks:add('frame'",1)[0]
s=f'local testRally={str(a.rally).lower()}\n'+''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+helper+Path('tests/tactics_melee_power_smoke.lua').read_text().replace('@OUTPUT@',str(out))
if a.facing:
 s=s[:s.index("callbacks:add('frame'")]+Path('tests/tactics_melee_facing_smoke.lua').read_text().replace('@OUTPUT@',str(out))
if a.trace:s+=Path('tests/tactics_melee_trace_tail.lua').read_text().replace('@OUTPUT@',str(out))
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(expected_checks=(19 if a.rally else 18) if a.facing else 13 if a.rally else 12,native_rally_assembly=a.rally,eight_direction_facing=a.facing,explicit_card_power_enemy_position_hp_fixture=True,rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),result='pending'),indent=2)+'\n')
