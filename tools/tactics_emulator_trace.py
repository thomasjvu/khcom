"""Turn a host command replay into mGBA Lua button presses and state assertions.
Uses documented mGBA callbacks/Core methods: https://mgba.io/docs/scripting.html
"""
from pathlib import Path
import argparse

def generate(trace,output,evidence):
    lines=trace.read_text().splitlines();events=[];checks=[];shots=[];frame=60;cursor=(1,2)
    before=None
    def wait(n=24):
        nonlocal frame
        frame+=n
    def press(key):
        nonlocal frame
        events.append((frame,key));events.append((frame+12,0));frame+=36
    def move(x,y):
        nonlocal cursor
        cx,cy=cursor
        for _ in range(abs(x-cx)):press(16 if x>cx else 32)
        for _ in range(abs(y-cy)):press(128 if y>cy else 64)
        cursor=x,y
    shots.append((40,'title'))
    for i,line in enumerate(lines):
        parts=line.split()
        if parts[0]=='new':press(1);wait();shots.append((frame,'map'))
        elif parts[0]=='room':
            if int(parts[1]):press(128)
            press(1);wait(36);cursor=(1,2);shots.append((frame,f'room-{len(checks)}'))
        elif parts[0]=='reward':
            for _ in range(int(parts[1])):press(128)
            press(1);wait()
        elif parts[0]=='turn':
            x,y,card,sleight,tx,ty=map(int,parts[1:])
            old=(before[56],before[57])
            if (x,y)!=old:move(x,y);press(1);wait()
            if card>=0:
                press(2)
                for _ in range(card):press(256)
                if sleight:press(768)
                else:
                    # Cure and Guard snap the cursor to Sora when selected.
                    hand=[j for j in range(before[242]) if before[170+j]==1]
                    if before[116+2*hand[card]] in (2,3):cursor=(x,y)
                move(tx,ty);press(1);wait();shots.append((frame,f'preview-{len(checks)}'));press(1);wait()
            else:
                press(8);press(128);press(1);wait()
            after=bytes.fromhex(lines[i+1].split()[1])
            if after[245]==0:press(8);press(1);wait()
        elif parts[0]=='state':
            before=bytes.fromhex(parts[1]);checks.append((frame,parts[1]));wait(4)
        elif parts[0]=='clear':shots.append((frame,'castle-clear'));wait(20)
    evidence.mkdir(parents=True,exist_ok=True)
    events_lua='{'+','.join(f'[{f}]={key}' for f,key in events)+'}'
    checks_lua='{'+','.join(f'[{f}]="{data}"' for f,data in checks)+'}'
    # Keep a small representative evidence set, not one image per input.
    selected={}
    for f,name in shots:
        if name in ('title','map','castle-clear') or (name.startswith('room-') and len(selected)<5) or (name.startswith('preview-') and len(selected)<6):selected[f]=name
    shots_lua='{'+','.join(f'[{f}]="{evidence.resolve()}/{name}.png"' for f,name in selected.items())+'}'
    result=evidence.resolve()/'replay-result.txt'
    output.write_text(f'''-- Generated input-only replay; no emulator memory writes.
local events={events_lua}
local checks={checks_lua}
local shots={shots_lua}
local n=0
local passed=0
local failed=false
local function log(s)
 local f=io.open("{result}","a");f:write(s.."\\n");f:close()
end
callbacks:add("frame",function()
 n=n+1
 if events[n]~=nil then emu:setKeys(events[n]) end
 if checks[n] and not failed then
  local expected=checks[n]
  for i=0,253 do
   local want=tonumber(expected:sub(i*2+1,i*2+2),16)
   local got=emu:read8(0x0203e000+i)
   if want~=got then
    log("FAIL frame "..n.." offset "..i.." expected "..want.." actual "..got)
    emu:screenshot("{evidence.resolve()}/failure.png")
    failed=true;break
   end
  end
  if not failed then passed=passed+1 end
 end
 if shots[n] then emu:screenshot(shots[n]) end
 if n=={frame} then log((failed and "FAILED" or "PASS").." "..passed.." state checkpoints; complete castle run") end
end)
''')
    print(f'{len(events)//2} button presses, {len(checks)} checkpoints, {frame} frames')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('trace',type=Path);p.add_argument('output',type=Path);p.add_argument('evidence',type=Path);a=p.parse_args()
    generate(a.trace,a.output,a.evidence)
