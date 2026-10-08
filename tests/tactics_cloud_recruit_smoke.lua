local f=0
local out=io.open('@OUTPUT@/cloud.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function transition(room)
 local r=emu:read32(gMapRoomState);local p=emu:read32(gFieldState)
 emu:write8(r+15,room);emu:write8(r+16,1);emu:write32(p+0x70,emu:read32(p+0x70)|16)
end
local function place(distance,height,charge)
 local p=emu:read32(gFieldState);local e=emu:read32(emu:read32(sEnemyTasks)+4)
 local x=emu:read32(p+0x18);local y=emu:read32(p+0x1c)
 for i=0,2 do for j=0,3 do emu:write32(sPartyPos+i*16+j*4,emu:read32(p+0x18+j*4)) end end
 emu:write32(e+8,x+distance*256);emu:write32(e+12,y-height*256)
 emu:write32(e+16,emu:read32(p+0x20)+height*256);emu:write32(e+20,emu:read32(p+0x24))
 emu:write8(gNativeEnemyCharge,charge)
end
local function hudText(row,column,text)
 local control=emu:read16(0x04000008)
 local screen=0x06000000+((control&0x1f00)<<3)
 local tiles=0x06000000+((control&0x000c)<<12)
 for index=1,#text do
  local c=text:byte(index)
  local glyph=c>=48 and c<=57 and c-48+1 or c>=65 and c<=90 and c-65+11 or 0
  local tile=emu:read16(screen+(row*32+column+index-1)*2)&0x3ff
  for y=0,6 do
   local bits=emu:read8(sUiGlyphs+glyph*7+y)
   local pixels=emu:read32(tiles+tile*32+y*4)
   for x=0,4 do
    local expected=(bits&(1<<(4-x)))~=0 and 3 or 1
    if ((pixels>>((x+1)*4))&15)~=expected then return false end
   end
  end
 end
 return true
end
callbacks:add('frame',function()
 f=f+1
 if f==180 or f==400 then emu:setKeys(1) end
 if f==184 or f==404 then emu:setKeys(0) end
 if f==240 then transition(9) end
 if f==460 then
  check(emu:read16(gNativeCloudReady)==1 and emu:read16(gNativeEnemyHp)==40,'optional room spawns original Cloud challenger')
  check(emu:read32(sCloudTiles)~=0 and emu:read32(sCloudPalette)~=0,'Cloud battle sprite and palette allocate')
  check(emu:read32(sEnemyTasks+4)==0,'Cloud challenge is solo')
  emu:screenshot('@OUTPUT@/cloud.png');place(64,24,0)
 end
 if f==500 then emu:setKeys(8) end
 if f==504 then emu:setKeys(0) end
 if f==610 then
  check(emu:read8(gNativeEnemyCharge)==1 and emu:read16(gNativeCloudPose)==1,'Cloud warns and animates sword windup')
  check(emu:read16(gNativeThreats)==12 and emu:read16(gNativeThreats+2)==12,'warned sweep includes sixty-four pixel reach and twenty-four pixel height')
  emu:setKeys(8)
 end
 if f==614 then emu:setKeys(0) end
 if f==720 then
  check(emu:read8(gNativePartyHealth)==68 and emu:read8(gNativePartyHealth+1)==44,'Cloud sword sweep resolves previewed party damage')
  place(65,0,1)
 end
 if f==750 then check(emu:read16(gNativeThreats)==0,'sword sweep excludes sixty-five pixel reach');place(64,25,1) end
 if f==780 then check(emu:read16(gNativeThreats)==0,'sword sweep excludes twenty-five pixel height');place(32,0,1) end
 if f==930 then emu:setKeys(12) end
 if f==934 then emu:setKeys(0) end
 if f==1000 then emu:reset() end
 if f==1330 then
  check(emu:read16(gNativeCloudReady)==1 and emu:read16(gNativeCloudPose)==1 and emu:read8(gNativeEnemyCharge)==1,'charged Cloud encounter survives reboot')
  emu:write16(gNativeEnemyHp,1);emu:write8(gNativeDeck+73,1);place(32,0,0);emu:setKeys(1)
 end
 if f==1334 then emu:setKeys(0) end
 if f==1400 then
  check(emu:read16(gNativeEnemyHp)==0 and emu:read16(gNativeProgressReward)==1 and emu:read8(gNativeRoster+17)==2,'native Fire defeats Cloud and offers boss reward')
  check(emu:read32(sRecruitTiles)~=0 and emu:read32(sRecruitPalette)~=0,'original Cloud summon card allocates for recruit reward')
  emu:setKeys(64)
 end
 if f==1404 then emu:setKeys(0) end
 if f==1460 then
  check(emu:read16(gWin0V)==56 and emu:read16(gWin1V)==37024,'boss reward uses compact contextual windows')
  check(hudText(5,2,'RECRUIT CLOUD'),'fifth reward option is fully rendered')
  check(hudText(6,0,'UP DOWN  L R HERO  A CHOOSE'),'reward controls fit the shortened panel')
  emu:screenshot('@OUTPUT@/recruit.png');emu:setKeys(1) end
 if f==1464 then emu:setKeys(0) end
 if f==1530 then
  check(emu:read8(gNativeRoster)==31 and emu:read16(gNativeProgressReward)==0,'recruit choice unlocks Cloud roster card')
  check(emu:read32(sRecruitTiles)==0 and emu:read32(sCardTiles+4)~=0 and emu:read32(sCardPalettes+4)~=0,'recruit choice restores combat card artwork')
 end
 if f==1600 then emu:setKeys(12) end
 if f==1604 then emu:setKeys(0) end
 if f==1680 then emu:reset() end
 if f==2010 then
  check(emu:read8(gNativeRoster)==31 and emu:read16(gNativeProgressReward)==0,'Cloud unlock persists without duplicate recruit reward')
  emu:setKeys(1)
 end
 if f==2014 then emu:setKeys(0) end
 if f==2100 then transition(0) end
 if f==2280 then
  check(emu:read16(gNativeCloudReady)==0,'room exit releases Cloud challenger presentation')
  out:close()
 end
end)
