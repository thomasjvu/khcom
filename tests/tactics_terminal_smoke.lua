local f=0
local out=io.open('@OUTPUT@/terminal.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function approach(room,dx,dy)
 local p=emu:read32(gFieldState);local n=emu:read32(p+0x80)
 while n~=0 do
  local t=emu:read32(n)
  if emu:read32(t)==gTaskDescMapRnd then
   local child=emu:read32(emu:read32(t+4)+8)
   while child~=0 do
    local task=emu:read32(child)
    if emu:read32(task)==gTaskDescMapDoor then
     local w=emu:read32(task+4);local door=emu:read32(w)
     if emu:read8(door+7)==room then
      check((emu:read16(door)&11)==3,'generated native door is open and unsealed')
      for j=0,3 do emu:write32(p+0x18+j*4,emu:read32(w+4+j*4)) end
      emu:write32(p+0x18,emu:read32(p+0x18)+dx)
      emu:write32(p+0x1c,emu:read32(p+0x1c)+dy)
      return true
     end
    end
    child=emu:read32(child+8)
   end
  end
  n=emu:read32(n+8)
 end
 return false
end
local clearFrame=nil
local clearHp=nil
local clearCommands=nil
local clearVisits=nil
local function transition(room)
 local r=emu:read32(gMapRoomState);local p=emu:read32(gFieldState)
 emu:write8(r+15,room);emu:write8(r+16,1);emu:write32(p+0x70,emu:read32(p+0x70)|16)
end
local function clearExit()
 emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253)
end
callbacks:add('frame',function()
 f=f+1
 -- Explicit world-transition fixture, followed by actual final-door movement.
 if f==180 or f==420 or f==660 then transition(7) end
 if f==300 or f==540 then clearExit() end
 if f==780 then
  check(emu:read16(gNativeFloor)==2,'fixture reaches Castle Oblivion exit room')
  emu:write8(gMapFloorState+0x1c+7*16+11,0)
  check(approach(253,4096,-2048),'fixture approaches the real final doorway')
  emu:setKeys(160)
 end
 if f==784 then emu:setKeys(0) end
 if not clearFrame and f>780 and emu:read16(gNativeResult)==2 then
  clearFrame=f;clearHp=emu:read8(gNativePartyHealth);clearCommands=emu:read32(gNativeCommands);clearVisits=emu:read16(gNativeRoomVisits)
  check(emu:read16(gNativeFloor)==3,'final native doorway reaches exactly the third-world terminal state')
 end
 if clearFrame and f==clearFrame+30 then emu:setKeys(11) end
 if clearFrame and f==clearFrame+34 then emu:setKeys(0) end
 if clearFrame and f==clearFrame+120 then
  out:write('TERMINAL floor='..emu:read16(gNativeFloor)..' hp='..emu:read8(gNativePartyHealth)..'\n')
  check(emu:read16(gNativeFloor)==3,'standing at the final door cannot advance a cleared run again')
  check(emu:read16(gNativeResult)==2 and emu:read8(gNativePartyHealth)==clearHp,'run-clear state and Sora HP remain stable')
  check(emu:read32(gNativeCommands)==clearCommands,'combat and turn inputs are ignored after victory')
  check(emu:read16(gNativeRoomVisits)==clearVisits,'terminal doorway cannot recreate a room')
  emu:screenshot('@OUTPUT@/clear.png');emu:setKeys(4)
 end
 if clearFrame and f==clearFrame+124 then emu:setKeys(0) end
 if clearFrame and f==clearFrame+260 then
  check(emu:read16(gNativeFloor)==0 and emu:read16(gNativeResult)==0,'Select restarts a fresh run after stable victory')
  check(emu:read8(gNativePartyHealth)==80,'retry restores Sora HP')
  out:close()
 end
 if f==1400 and not clearFrame then check(false,'fixture failed to reach native final-door victory');out:close() end
end)
