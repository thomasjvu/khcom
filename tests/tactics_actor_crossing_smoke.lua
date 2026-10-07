-- Explicit actor-position fixture. Preview and confirmation use native input.
local frame=0
local out=io.open('@OUTPUT@/crossings.txt','w')
local directions={80,96,144,160}
local chosen,origin,target,node
local function check(ok,message) out:write((ok and 'PASS ' or 'FAIL ')..message..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function signed(v) return v>=0x80000000 and v-0x100000000 or v end
local function enemyPos(i,x,y,z)
 local task=emu:read32(sEnemyTasks+i*4)
 if task~=0 then local work=emu:read32(task+4)
  emu:write32(work+8,x);emu:write32(work+12,y);emu:write32(work+16,z);emu:write32(work+20,z)
 end
 return task~=0
end
local function friendPos(x,y,z)
 emu:write32(sPartyPos+16,x);emu:write32(sPartyPos+20,y);emu:write32(sPartyPos+24,z);emu:write32(sPartyPos+28,z)
end
callbacks:add('frame',function()
 frame=frame+1
 if frame==180 then emu:setKeys(1) end
 if frame==184 then emu:setKeys(0) end
 if frame>=210 and frame<530 then
  local slot=math.floor((frame-210)/80)+1;local phase=(frame-210)%80
  if phase==0 then
   for i=0,5 do enemyPos(i,131072,131072,0) end
   emu:setKeys(512+directions[slot])
  end
  if phase==4 then emu:setKeys(0) end
  if phase==24 then
   if not chosen and emu:read16(gNativeRouteCost)==1 then chosen=directions[slot] end
   emu:setKeys(2)
  end
  if phase==28 then emu:setKeys(0) end
 end
 if frame==550 then check(chosen~=nil,'generated terrain has a free diagonal edge');if chosen then emu:setKeys(512+chosen) end end
 if frame==554 then emu:setKeys(0) end
 if frame==580 and chosen then
  check(emu:read16(gNativeRouteCost)==1,'unobstructed diagonal costs one point')
  local x=chosen&16~=0 and 1 or -1;local y=chosen&64~=0 and -1 or 1
  node=(y+4)*9+x+4;local address=sRoutePos+node*16
  origin={signed(emu:read32(field()+0x18)),signed(emu:read32(field()+0x1c)),signed(emu:read32(field()+0x20))}
  target={signed(emu:read32(address)),signed(emu:read32(address+4)),signed(emu:read32(address+8))}
  friendPos(origin[1]+x*3072,origin[2]-y*1024,origin[3])
 end
 if frame==620 and chosen then
  check(emu:read16(gNativeRouteCost)~=1,'party corner crossing rejects the direct edge')
  check(emu:read8(gNativeReachCost+node)~=1,'reachable mask agrees with swept party occupancy')
  emu:screenshot('@OUTPUT@/party-block.png')
  friendPos(origin[1],origin[2],origin[3])
  local x=chosen&16~=0 and 1 or -1;local y=chosen&64~=0 and -1 or 1
  check(enemyPos(0,origin[1]+x*3072,origin[2]-y*1024,origin[3]),'native enemy is available for crossing fixture')
 end
 if frame==660 and chosen then
  check(emu:read16(gNativeRouteCost)~=1,'enemy corner crossing rejects the direct edge')
  check(emu:read8(gNativeReachCost+node)~=1,'reachable mask agrees with swept enemy occupancy')
  enemyPos(0,131072,131072,0)
 end
 if frame==700 and chosen then
  check(emu:read16(gNativeRouteCost)==1,'moving the blocker restores the one-point edge')
  emu:setKeys(1)
 end
 if frame==704 then emu:setKeys(0) end
 if frame==950 then
  check(chosen and emu:read16(gNativeBusy)==0 and emu:read16(gNativeMoveLeft)==2 and
   math.abs(signed(emu:read32(field()+0x18))-target[1])<512 and
   math.abs(signed(emu:read32(field()+0x1c))-target[2])<512,'native movement executes restored edge and charges once')
  out:close()
 end
end)
