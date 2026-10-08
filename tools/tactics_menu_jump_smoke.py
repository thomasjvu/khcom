"""Generate an input-only native Move/Jump command regression."""
import argparse,subprocess,hashlib,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--ledge-save',type=Path,help='explicit saved ledge approach fixture; requires --forecast-direction 96');p.add_argument('elf');p.add_argument('output');p.add_argument('--forecast-direction',type=int,choices=(16,32,64,128,80,96,144,160));p.add_argument('--direction',type=int,choices=(16,32,64,128,80,96,144,160),help='capture direct native moving jump in this D-pad direction');p.add_argument('--trace',action='store_true',help='record read-only native jump positions and budgets for predictor comparison');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeAssembly','gNativeRoster','gNativeParty','gNativeMenu','gNativeMenuChoice','gNativeMoveLeft','gNativeActionLeft','gNativePreview','gNativeSaveNotice','gNativePartyHealth','gGameState','gNativeDeck','gNativeCureTarget','gNativeCureHeal','gFieldState','sEnemyTasks','gNativeEnemyHp','gNativeFireTarget','gNativeFireDamage','gNativeBusy','gNativeDirection','gFrameCounter','gSineTable','gNativeJumpLanding','gNativeJumpPrediction','sUiGlyphs','gNativeProgressReward')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_menu_jump_smoke.lua').read_text().replace('@OUTPUT@',str(out))

if a.forecast_direction:
 s=s[:s.index('-- Input-only command')]+Path('tests/tactics_jump_forecast_smoke.lua').read_text().replace('@OUTPUT@',str(out)).replace('@DIRECTION@',str(a.forecast_direction))
if a.ledge_save:
 if a.forecast_direction!=96: p.error('--ledge-save requires --forecast-direction 96')
 (out/'fresh.sav').write_bytes(a.ledge_save.read_bytes())
 s=s.replace(' if f==180 then emu:setKeys(8) end', " if f>=180 and f<=212 and f%8==4 and (emu:read16(gNativeAssembly)~=0 or emu:read16(gNativeProgressReward)~=0) then emu:setKeys(1) end\n if f>=184 and f<=216 and f%8==0 then emu:setKeys(0) end")
 s=s.replace("else check(resolved>=2 and resolved<=4,'attachment or unresolved terrain withheld guaranteed landing') end", "else\n   check(resolved==4,'ledge approach predicts catch transition')\n   local task=emu:read32(emu:read32(field+0x94))\n   local state=emu:read32(emu:read32(task+4)+0x94)\n   check(state==9 and emu:read16(gNativeBusy)==2,'native jump reaches hanging ledge command state')\n  end")
 s=s.replace('output:close()', 'emu:setKeys(4)')
 s=s.replace("\nend)\n", "\n if f==804 then emu:setKeys(0) end\n if f==840 then\n  check(emu:read16(gNativeMenu)==9,'Select opens Climb and Drop after predicted catch')\n  emu:screenshot('@OUTPUT@/caught-menu.png');output:close()\n end\nend)\n".replace('@OUTPUT@',str(out)))
if a.direction:
 s=s[:s.index('-- Input-only command')]+"""local sampleFrame=0
callbacks:add('frame',function()
 sampleFrame=sampleFrame+1
 if sampleFrame==180 then emu:setKeys(8) end
 if sampleFrame==184 or sampleFrame==244 then emu:setKeys(0) end
 if sampleFrame==240 then emu:setKeys(DIRECTION+2) end
end)
""".replace('DIRECTION',str(a.direction))
 a.trace=True
if a.trace:
 s+=Path('tests/tactics_jump_trace_tail.lua').read_text().replace('@OUTPUT@',str(out))
assert 'emu:write' not in s
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(input_only=True,explicit_memory_fixture=False,explicit_saved_ledge_approach=bool(a.ledge_save),saved_approach_sha256=hashlib.sha256(a.ledge_save.read_bytes()).hexdigest() if a.ledge_save else None,rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),expected_checks=0 if a.direction else 7 if a.ledge_save else 5 if a.forecast_direction else 12,forecast_direction=a.forecast_direction,direct_jump_direction=a.direction,read_only_trajectory_trace=a.trace,result='not yet observed'),indent=2)+'\n')
