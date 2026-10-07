local f=0
local out=io.open('@OUTPUT@/door-hit.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local doorWork=nil
local doorPosition=nil
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
      doorWork=w
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
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local p=emu:read32(gFieldState);local original={}
  for j=0,3 do original[j]=emu:read32(p+0x18+j*4) end
  check(approach(1,8192,-4096),'fixture finds an original open door')
  doorPosition={}
  for j=0,3 do
   doorPosition[j]=emu:read32(doorWork+4+j*4)
   emu:write32(p+0x18+j*4,original[j]);emu:write32(doorWork+4+j*4,original[j])
  end
  -- Move the native door object beside the original spawn, keeping its
  -- callback and flags. Terrain door cells stay at their actual exit.
  emu:write32(doorWork+8,original[1]-2048);emu:write8(p+0x2c,0)
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then local w=emu:read32(t+4);emu:write32(w+8,0);emu:write32(w+12,0);emu:write32(w+16,0) end
  end
 end
 if f==220 then emu:setKeys(1) end
 if f==224 then emu:setKeys(0) end
 if f==400 then
  local p=emu:read32(gFieldState);local task=emu:read32(emu:read32(p+0x94))
  check((emu:read32(p+0x70)&0x40000)==0,'sword swing near open door does not start room synthesis')
  check((emu:read32(task+0x20)&0xfffffffe)~=FldSoraWaitRoomCreate,'native player does not wait for room-card selection')
  check(emu:read16(gNativeBusy)==0,'door-adjacent sword command completes')
  check(emu:read16(gNativeActionLeft)==0 and emu:read16(gNativeMoveLeft)==3,'door-adjacent sword spends exactly one action')
  check(emu:read8(gMapFloorState+6)==0 and emu:read16(gNativeRoomVisits)==1,'sword swing does not recreate or change the generated room')
  check(emu:read32(gCurrentMode)==sNativeMode,'sword swing retains tactics mode')
  for j=0,3 do emu:write32(doorWork+4+j*4,doorPosition[j]) end
  check(approach(1,4096,-2048),'original door remains available after the swing')
 end
 if f==420 then emu:setKeys(160) end
 if f==424 then emu:setKeys(0) end
 if f==640 then
  check(emu:read8(gMapFloorState+6)==1,'walking through the attacked door still crosses to room one')
  check(emu:read16(gNativeRoomVisits)==2,'walking through attacked door creates exactly one room visit')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'door travel preserves spent action and charges one movement')
  check(emu:read32(gCurrentMode)==sNativeMode,'door travel retains tactics mode')
  emu:screenshot('@OUTPUT@/crossed.png');out:close()
 end
end)
