-- Native top-boundary route; initial approach uses a real stair position fixture.
local f=0
local upperZ,lowerZ,lastTarget,started=0,0,nil,0
local out=io.open('@OUTPUT@/climb-preview.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function player() return emu:read32(emu:read32(emu:read32(field()+0x94))+4) end
local function signed(v) return v>=0x80000000 and v-0x100000000 or v end
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
    local upper=signed(emu:read32(c+8));local lower=signed(emu:read32(c+12))
    if lower<0x100000 and lower-upper==16384 then
     upperZ=upper;lowerZ=lower;local p=field()
     emu:write32(p+0x18,(x*32+16)*256);emu:write32(p+0x1c,(y*16+14)*256-lower)
     emu:write32(p+0x20,lower);emu:write32(p+0x24,lower);found=true
    end
   end
  end end
  check(found,'generated room contains a native four-level stair with a top platform')
  for i=0,5 do local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then emu:write32(emu:read32(t+4)+8,131072) end
  end
  emu:setKeys(64)
 end
 if f==340 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeBusy)==0,'native attachment completes before top route')
  check(math.abs(signed(emu:read32(field()+0x20))-(lowerZ-4096))<=48,'entry advances exactly one vertical level')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'entry has independent movement and action costs')
  emu:setKeys(0)
 end
 if f==360 then emu:setKeys(8) end
 if f==364 then emu:setKeys(0) end
 if f==420 then check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeClimbing)==1,'new turn restores movement before the top route') end
 if f==440 then emu:setKeys(576) end
 if f==444 then emu:setKeys(0) end
 if f==460 or f==480 then emu:setKeys(64) end
 if f==464 or f==484 then emu:setKeys(0) end
 if f==520 then
  check(emu:read16(gNativeRouteCost)==3 and signed(emu:read32(previewPos()+8))==upperZ,'three-level ascent previews the actual top platform height')
  check((emu:read16(gNativeClimbReachMask)&21)==21,'all three affordable top-route heights are marked reachable')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'top preview preserves movement and action')
  emu:screenshot('@OUTPUT@/top-preview.png');lastTarget=signed(emu:read32(player()+0xb8))
 end
 if f==540 then emu:setKeys(1) end
 if f==544 then emu:setKeys(0) end
 if f>540 and f<850 then
  local target=signed(emu:read32(player()+0xb8))
  if lastTarget and target<lastTarget then started=started+math.floor((lastTarget-target)/4096);lastTarget=target end
 end
 if f==560 then check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'confirmation reserves the three-point route without spending action') end
 if f==850 then
  check(emu:read32(player()+0x94)==0 and emu:read16(gNativeBusy)==0,'original climb-over animation lands on the top platform and releases control')
  check(math.abs(signed(emu:read32(field()+0x20))-upperZ)<=48,'native top landing reaches the selected platform height')
  check(started>0 and started<=3 and emu:read16(gNativeMoveLeft)==3-started,'net movement cost matches independently observed native vertical commands')
  check(emu:read16(gNativeActionLeft)==1,'top boundary transition preserves the combat action')
  emu:screenshot('@OUTPUT@/top-arrival.png')
 end
 if f==880 then emu:setKeys(516) end
 if f==884 then emu:setKeys(0) end
 if f==910 then check(emu:read16(gNativeParty)==1,'party selection works after top-platform landing');out:close() end
end)
