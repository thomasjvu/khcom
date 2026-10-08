local f=0
local out=io.open('@OUTPUT@/transitions.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function transition(room)
 local p=emu:read32(gFieldState);local r=emu:read32(gMapRoomState)
 emu:write8(r+15,room);emu:write8(r+16,1)
 emu:write32(p+0x70,emu:read32(p+0x70)|16)
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(516) end
 if f==184 then emu:setKeys(0) end
 if f==220 then emu:setKeys(64) end
 if f==224 then emu:setKeys(0) end
 if f==300 then emu:setKeys(256) end
 if f==304 then emu:setKeys(0) end
 if f==340 then emu:setKeys(1) end
 if f==344 then emu:setKeys(0) end
 if f==400 then
  check(emu:read16(gNativeParty)==1 and emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'Donald spends movement and Fire action through real commands')
  emu:write16(gNativeGuard,2);transition(9)
 end
 if f==520 then
  check(emu:read16(gNativeParty)==1,'room entry preserves selected Donald')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'room entry preserves Donald budgets')
  check(emu:read16(gNativeGuard)==2,'room entry preserves active Guard')
  check(emu:read16(gGameState+0x32)==56,'room entry preserves active member health')
  emu:setKeys(516)
 end
 if f==524 then emu:setKeys(0) end
 if f==560 then
  check(emu:read16(gNativeParty)==2 and emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'untouched Goofy keeps his own budgets')
  emu:setKeys(16)
 end
 if f==564 then emu:setKeys(0) end
 if f==640 then emu:setKeys(1) end
 if f==644 then emu:setKeys(0) end
 if f==720 then emu:setKeys(516) end
 if f==724 then emu:setKeys(0) end
 if f==800 then transition(0) end
 if f==920 then
  check(emu:read16(gNativeParty)==0 and emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'backtracking preserves untouched Sora budgets')
  emu:setKeys(516)
 end
 if f==924 then emu:setKeys(0) end
 if f==960 then
  check(emu:read16(gNativeParty)==1 and emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'backtracking does not refill Donald')
  emu:setKeys(516)
 end
 if f==964 then emu:setKeys(0) end
 if f==1000 then
  check(emu:read16(gNativeParty)==2 and emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'backtracking does not refill Goofy')
  emu:setKeys(12)
 end
 if f==1004 then emu:setKeys(0) end
 if f==1060 then check(emu:read16(gNativeSaveNotice)==1,'carried turn state saves');emu:reset() end
 if f==1300 then
  check(emu:read16(gNativeParty)==2 and emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'reset restores carried member and budgets')
  emu:setKeys(8)
 end
 if f==1304 then emu:setKeys(0) end
 if f==1420 then
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'enemy phase restores party turn resources normally')
  check(emu:read16(gNativeGuard)==0,'enemy phase consumes Guard normally')
  transition(7)
 end
 if f==1540 then
  emu:write16(gNativeMoveLeft,1);emu:write16(gNativeActionLeft,0)
  emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253)
 end
 if f==1660 then
  check(emu:read16(gNativeFloor)==1 and emu:read16(gNativeParty)==2,'world advancement preserves selected Goofy')
  check(emu:read16(gNativeMoveLeft)==1 and emu:read16(gNativeActionLeft)==0,'world advancement preserves spent resources')
  emu:write16(gNativeResult,1);emu:setKeys(4)
 end
 if f==1664 then emu:setKeys(0) end
 if f==1800 then
  check(emu:read16(gNativeFloor)==0 and emu:read16(gNativeParty)==0,'retry starts fresh with Sora on first world')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'fresh run receives full resources')
  out:close()
 end
end)
