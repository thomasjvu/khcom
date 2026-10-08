"""Generate an mGBA replay that verifies the native field command boundary."""
import argparse
import subprocess
from pathlib import Path

def generate(elf, output):
    names={}
    for line in subprocess.check_output(['arm-none-eabi-nm',str(elf)],text=True).splitlines():
        parts=line.split()
        if len(parts)==3:names[parts[2]]=int(parts[0],16)
    output=Path(output).resolve();output.mkdir(parents=True,exist_ok=True)
    source='local frame=0\nlocal initialX, initialY, stoppedX, stoppedY\n'
    for name in ('gFieldState','gGameState','gNativeCommands','gNativeMoveLeft','gNativeActionLeft',
                 'gNativeBusy','gNativeEnemyFrames','gNativeKills','gNativeRoomVisits'):
        source+=f'local {name}=0x{names[name]:08x}\n'
    source+=f'local out=io.open("{output}/result.txt","w")\n'
    source+='''local function check(ok,message)
 out:write((ok and "PASS " or "FAIL ")..message.."\\n");out:flush()
end
callbacks:add('frame',function()
 frame=frame+1
 local p=emu:read32(gFieldState)
 if frame==180 then
  initialX=emu:read32(p+0x18);initialY=emu:read32(p+0x1c)
  check(p>=0x02000000 and p<0x02034000,"native field allocated")
  check(emu:read16(gNativeMoveLeft)==3,"initial movement budget")
  check(emu:read16(gGameState+0x32)==80,"initial HP")
  emu:setKeys(64)
 end
 if frame==240 then
  stoppedX=emu:read32(p+0x18);stoppedY=emu:read32(p+0x1c)
  check(emu:read32(gNativeCommands)==1,"held input commits one command")
  check(emu:read16(gNativeMoveLeft)==2,"one movement point consumed")
  check(emu:read16(gNativeBusy)==0,"movement completes")
  check(stoppedX~=initialX or stoppedY~=initialY,"Sora moves through native field")
 end
 if frame==300 then
  check(emu:read32(p+0x18)==stoppedX and emu:read32(p+0x1c)==stoppedY,"held direction stays stopped")
  emu:setKeys(0)
 end
 if frame==320 then emu:setKeys(1) end
 if frame==324 then emu:setKeys(0) end
 if frame==500 then
  check(emu:read16(gNativeActionLeft)==0,"field attack consumes action")
  check(emu:read16(gNativeBusy)==0,"attack animation completes")
  check(emu:read16(gNativeRoomVisits)==1,"attack stays in field mode")
  emu:setKeys(8)
 end
 if frame==512 then emu:setKeys(0) end
 if frame==650 then
  check(emu:read16(gNativeEnemyFrames)==0,"enemy phase completes")
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,"turn budgets restored")
  check(emu:read16(gNativeRoomVisits)==1,"enemy contact stays in field mode")
'''
    source+=f'  emu:screenshot("{output}/field.png")\n  emu:setKeys(2)\n end\n'
    source+=''' if frame==662 then emu:setKeys(0) end
 if frame==680 then
  check((emu:read32(p+0x70)&0x800000)~=0,"jump uses native height physics")
 end
 if frame==950 then
  check(emu:read16(gNativeBusy)==0,"jump lands and releases input gate")
  check(emu:read16(gNativeActionLeft)==0,"jump consumes action")
  out:close()
 end
end)
'''
    (output/'smoke.lua').write_text(source)
    scenario_names = ('gFieldState','gMapRoomState','gMapFloorState','gGameState',
                      'gNativeKills','gNativeChests','gNativeFloor','gNativeResult','gNativeDeck','gTaskDescMapGmk01','gNativeReward','gNativeRewardChoice','sRewardSeed','gNativeSaveNotice','gNativeActionLeft','gNativeMoveLeft','sHudScreen')
    header = ''.join(f'local {name}=0x{names[name]:08x}\n' for name in scenario_names)
    template = Path('tests/tactics_native_scenarios.lua').read_text()
    (output/'scenarios.lua').write_text(header + template.replace('@OUTPUT@', str(output)))
    reward = template.split(' if f==650 then')[0].replace('  transition(7)\n', '')
    reward += Path('tests/tactics_reward_save_tail.lua').read_text()
    (output/'reward-save.lua').write_text(header + reward.replace('@OUTPUT@', str(output)).replace('/scenarios.txt', '/reward-save.txt'))
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('elf');parser.add_argument('output')
    args=parser.parse_args();generate(args.elf,args.output)
