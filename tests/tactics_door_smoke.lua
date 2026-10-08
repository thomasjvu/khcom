local f=0
local out=io.open('@OUTPUT@/doors.txt','w')
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
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(516) end
 if f==184 then emu:setKeys(0) end
 if f==260 then check(approach(1,4096,-2048),'first room contains a real door to room one');emu:setKeys(160) end
 if f==264 then emu:setKeys(0) end
 if f==440 then
  check(emu:read8(gMapFloorState+6)==1,'native diagonal movement crosses the generated door')
  check(emu:read16(gNativeParty)==1,'actual door crossing preserves selected Donald')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'actual door crossing charges movement without refilling resources')
  check(emu:read16(gNativeTurn)==0,'door crossing does not advance the enemy phase')
  emu:screenshot('@OUTPUT@/crossed.png')
  check(approach(0,-4096,2048),'next room contains the reciprocal native door');emu:setKeys(80)
 end
 if f==444 then emu:setKeys(0) end
 if f==640 then
  check(emu:read8(gMapFloorState+6)==0,'native movement crosses reciprocal door back to room zero')
  check(emu:read16(gNativeMoveLeft)==1,'actual backtracking preserves total movement cost')
  check(emu:read16(gNativeRoomVisits)==3,'native door crossings create exactly two additional room visits')
  emu:setKeys(8)
 end
 if f==644 then emu:setKeys(0) end
 if f==780 then
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'ending turn after real backtracking renews resources')
  out:close()
 end
end)
