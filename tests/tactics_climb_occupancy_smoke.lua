-- Position and occupancy fixtures; all preview/confirm/execution uses input.
local f=0
local startZ,originZ,friend,enemy,enemyWork
local out=io.open('@OUTPUT@/climb-preview.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function restore(address,values) for i=0,3 do emu:write32(address+i*4,values[i+1]) end end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(1) end
 if f==184 then emu:setKeys(0) end
 if f==210 then
  local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
  local cells=emu:read32(sMapCells);local found=false
  for y=1,rows-2 do for x=1,cols-2 do
   local c=cells+(y*cols+x)*32
   if not found and (emu:read16(c)&32)~=0 and emu:read8(c+2)==4 then
    local upper=emu:read32(c+8);local lower=emu:read32(c+12)
    if lower<0x100000 and lower-upper>=16384 then
     startZ=lower;restore(field()+0x18,{(x*32+16)*256,(y*16+14)*256-startZ,startZ,lower});found=true
    end
   end
  end end
  check(found,'generated room provides native stair geometry for occupancy checks')
  for i=0,5 do local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then emu:write32(emu:read32(t+4)+8,131072) end
  end
  emu:setKeys(64)
 end
 if f==340 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeBusy)==0,'native stair attachment completes before occupancy fixture')
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-4096))<=48,'actor begins on the expected attached height')
  originZ=emu:read32(field()+0x20);emu:setKeys(0)
 end
 if f==350 then
  friend={};for i=0,3 do friend[i+1]=emu:read32(sPartyPos+16+i*4) end
  restore(sPartyPos+16,{emu:read32(field()+0x18),emu:read32(field()+0x1c),originZ-4096,originZ-4096})
 end
 if f==360 then emu:setKeys(576) end
 if f==364 then emu:setKeys(0) end
 if f==380 then emu:setKeys(64) end
 if f==384 then emu:setKeys(0) end
 if f==410 then
  check(emu:read16(gNativeRouteCost)==65535,'party member blocks a climb route through the first vertical destination')
  check((emu:read16(gNativeClimbReachMask)&5)==0,'party occupancy removes both ascent destinations from reachable markers')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'blocked inspection preserves movement and action')
  emu:screenshot('@OUTPUT@/party-blocked.png');emu:setKeys(1)
 end
 if f==414 then emu:setKeys(0) end
 if f==430 then
  check(emu:read16(gNativeBusy)==0 and emu:read16(gNativeMoveLeft)==2 and emu:read32(field()+0x20)==originZ,'blocked confirmation cannot move through the party member')
  restore(sPartyPos+16,friend)
 end
 if f==460 then
  check(emu:read16(gNativeRouteCost)==2 and (emu:read16(gNativeClimbReachMask)&5)==5,'moving the party member restores both climb destinations')
  local task=emu:read32(sEnemyTasks);check(task~=0,'native enemy is available for vertical occupancy checks')
  if task~=0 then
   enemyWork=emu:read32(task+4)+8;enemy={};for i=0,3 do enemy[i+1]=emu:read32(enemyWork+i*4) end
   restore(enemyWork,{emu:read32(field()+0x18),emu:read32(field()+0x1c),originZ-4096,originZ-4096})
  end
 end
 if f==500 then
  check(emu:read16(gNativeRouteCost)==65535,'enemy blocks a route through an occupied climb destination')
  check((emu:read16(gNativeClimbReachMask)&5)==0,'enemy occupancy removes ascent destinations from reachable markers')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'enemy-blocked inspection preserves both budgets')
  emu:screenshot('@OUTPUT@/enemy-blocked.png');if enemyWork then restore(enemyWork,enemy) end
 end
 if f==540 then
  check(emu:read16(gNativeRouteCost)==2,'removing enemy restores the two-segment route');emu:setKeys(1)
 end
 if f==544 then emu:setKeys(0) end
 if f==760 then
  check(startZ and emu:read16(gNativeBusy)==0 and math.abs(emu:read32(field()+0x20)-(startZ-12288))<=48,'original controller executes both restored vertical segments')
  check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'restored climb charges movement exactly once and preserves action')
  out:close()
 end
end)
