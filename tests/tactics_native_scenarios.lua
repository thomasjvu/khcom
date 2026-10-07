local f=0
local out=io.open('@OUTPUT@/scenarios.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function enemy()
 local n=emu:read32(field()+0xbc)
 if n==0 then return 0 end
 return emu:read32(emu:read32(n)+4)
end
local function transition(room)
 local r=emu:read32(gMapRoomState)
 emu:write8(r+15,room); emu:write8(r+16,1)
 emu:write32(field()+0x70,emu:read32(field()+0x70)|16)
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local p=field();local e=enemy()
  check(e~=0,'seeded field enemy exists')
  if e~=0 then
   -- Fixture places target beside Sora; A still uses the original sword animation and hitbox.
   emu:write32(e+8,emu:read32(p+0x18));emu:write32(e+12,emu:read32(p+0x1c)-0x800)
   emu:write32(e+16,emu:read32(p+0x20));emu:write32(e+20,emu:read32(p+0x24))
  end
  emu:write8(p+0x2c,0);emu:setKeys(1)
 end
 if f==184 then emu:setKeys(0) end
 if f==270 then check(emu:read32(gNativeKills)>0,'native sword hit removes field enemy');transition(9) end
 if f==390 then
  check(emu:read8((gMapFloorState+6))==9,'room transition uses generated graph destination')
  emu:screenshot('@OUTPUT@/chest-room.png');emu:setKeys(8)
 end
 if f==394 then emu:setKeys(0) end
 if f==460 then
  local n=emu:read32(field()+0x80)
  local chest=0
  while n~=0 do
   local t=emu:read32(n)
   if emu:read32(t)==gTaskDescMapGmk01 then chest=emu:read32(t+4);break end
   n=emu:read32(n+8)
  end
  check(chest~=0,'generated reward room contains original chest sprite')
  if chest~=0 then
   local p=field()
   emu:write32(p+0x18,emu:read32(chest+4));emu:write32(p+0x1c,emu:read32(chest+8)+0x1000)
   emu:write32(p+0x20,emu:read32(chest+12));emu:write32(p+0x24,emu:read32(chest+16))
   emu:write8(p+0x2c,0);emu:write16((gGameState+0x32),40);emu:write8(gNativeDeck+73,3);emu:setKeys(1)
  end
 end
 if f==464 then emu:setKeys(0) end
 if f==530 then
  check(emu:read8(gNativeDeck+72)==13,'chest adds a real card to the persistent deck')
  check(emu:read16(gNativeChests)==1,'native chest opens and awards reward once')
  check(emu:read16((gGameState+0x32))==52,'chest restores persistent run HP')
  emu:screenshot('@OUTPUT@/chest-open.png')
  transition(7)
 end
 if f==650 then
  -- Fixture clears exit encounter to exercise advancement, not a claim of full-run input QA.
  emu:write8(gMapFloorState+0x1c+7*16+11,0)
  transition(253)
 end
 if f==770 then
  check(emu:read16(gNativeFloor)==1,'advance to Agrabah floor')
  emu:screenshot('@OUTPUT@/agrabah.png')
  transition(7)
 end
 if f==890 then emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253) end
 if f==1010 then
  check(emu:read16(gNativeFloor)==2,'advance to Castle Oblivion floor')
  emu:screenshot('@OUTPUT@/castle.png')
  transition(7)
 end
 if f==1130 then emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253) end
 if f==1250 then check(emu:read16(gNativeResult)==2,'third floor completes run');emu:setKeys(4) end
 if f==1254 then emu:setKeys(0) end
 if f==1380 then
  check(emu:read16(gNativeFloor)==0 and emu:read16(gNativeResult)==0,'retry starts a new procedural run')
  check(emu:read16((gGameState+0x32))==80,'retry restores HP')
 end
 if f==1420 then emu:write16(gGameState+0x32,4) end
 if f>=1460 and f<=1910 and (f-1460)%90==0 then
  local p=field();local e=enemy()
  if e~=0 then
   emu:write32(e+8,emu:read32(p+0x18));emu:write32(e+12,emu:read32(p+0x1c))
   emu:write32(e+16,emu:read32(p+0x20));emu:write32(e+20,emu:read32(p+0x24))
  end
  emu:setKeys(8)
 end
 if f>=1464 and f<=1914 and (f-1464)%90==0 then emu:setKeys(0) end
 if f==2070 then
  check(emu:read16(gNativeResult)==1 and emu:read16(gGameState+0x32)==0,'field contact reaches defeat at zero HP')
  emu:screenshot('@OUTPUT@/defeat.png')
  out:close()
 end
end)
