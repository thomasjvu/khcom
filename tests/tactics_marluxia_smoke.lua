local f=0
local assemblyHeld=false
local out=io.open('@OUTPUT@/boss.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function transition(room)
 local r=emu:read32(gMapRoomState);local p=emu:read32(gFieldState)
 emu:write8(r+15,room);emu:write8(r+16,1)
 emu:write32(p+0x70,emu:read32(p+0x70)|16)
end
local function place(distance,height,charge)
 -- Explicit position fixture: all members are equally close, only one is hit.
 local p=emu:read32(gFieldState);local e=emu:read32(emu:read32(sEnemyTasks)+4)
 local x=emu:read32(p+0x18);local y=emu:read32(p+0x1c)
 for i=0,2 do for j=0,3 do emu:write32(sPartyPos+i*16+j*4,emu:read32(p+0x18+j*4)) end end
 emu:write32(e+8,x+distance*256);emu:write32(e+12,y-height*256)
 emu:write32(e+16,emu:read32(p+0x20)+height*256);emu:write32(e+20,emu:read32(p+0x24))
 emu:write8(gNativeEnemyCharge,charge)
end
local function cards()
 local ok=emu:read32(sValueTiles)~=0 and emu:read32(sValuePalette)~=0
 for i=0,3 do ok=ok and emu:read32(sCardTiles+i*4)~=0 and emu:read32(sCardPalettes+i*4)~=0 end
 return ok
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
 -- These encounter fixtures still deploy every room through native input.
 if emu:read16(gNativeAssembly)~=0 then emu:setKeys(1);assemblyHeld=true
 elseif assemblyHeld then emu:setKeys(0);assemblyHeld=false end
 if f==180 then transition(7) end
 if f==320 then
  -- Forced floor advancement is fixture setup, not complete-run proof.
  emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253)
 end
 if f==480 then transition(7) end
 if f==640 then emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253) end
 if f==800 then check(emu:read16(gNativeFloor)==2,'fixture advances to Castle');transition(7) end
 if f==1000 then
  check(emu:read16(gNativeMarlReady)==1,'Marluxia replaces Castle elite presentation')
  check(emu:read16(gNativeEnemyHp)==56,'Marluxia starts with 56 HP')
  check(emu:read32(sMarlTiles)~=0 and emu:read32(sMarlTiles+4)~=0 and emu:read32(sMarlPalette)~=0,'original Marluxia idle/scythe art and palette allocate')
  check(cards(),'Marluxia room retains all card artwork')
  emu:screenshot('@OUTPUT@/marluxia.png');place(96,24,0)
 end
 if f==1020 then emu:setKeys(8) end
 if f==1024 then emu:setKeys(0) end
 if f==1130 then
  check(emu:read8(gNativeEnemyCharge)==1 and emu:read8(gNativePartyHealth)==80,'Marluxia winds up without immediate damage')
  check(emu:read16(gNativeThreats)==10,'scythe includes exact 96-pixel range and 24-pixel height')
  check(emu:read16(gNativeThreats+2)==10 and emu:read16(gNativeThreats+4)==10,'scythe previews all members within the sweep')
  check(emu:read16(gNativeMarlPose)==1,'charged scythe displays original scythe windup')
  check(hudText(2,0,'SCYTHE CHARGED'),'compact HUD names scythe windup')
  emu:screenshot('@OUTPUT@/marluxia-charge.png');emu:setKeys(8)
 end
 if f==1134 then emu:setKeys(0) end
 if f==1240 then
  check(emu:read8(gNativePartyHealth)==70,'scythe resolves ten damage to previewed Sora target')
  check(emu:read8(gNativePartyHealth+1)==46 and emu:read8(gNativePartyHealth+2)==62,'scythe resolves previewed damage to Donald and Goofy')
  check(emu:read8(gNativeEnemyCharge)==0 and emu:read16(gNativeMarlPose)==0,'scythe consumes windup and returns to idle')
  place(97,24,1)
 end
 if f==1260 then check(emu:read16(gNativeThreats)==0,'scythe excludes 97-pixel range');emu:setKeys(8) end
 if f==1264 then emu:setKeys(0) end
 if f==1370 then check(emu:read8(gNativePartyHealth)==70,'out-of-range scythe does no damage');place(96,25,1) end
 if f==1390 then check(emu:read16(gNativeThreats)==0,'scythe excludes 25-pixel height');emu:setKeys(8) end
 if f==1394 then emu:setKeys(0) end
 if f==1500 then check(emu:read8(gNativePartyHealth)==70,'out-of-height scythe does no damage');place(64,0,1);emu:setKeys(12) end
 if f==1504 then emu:setKeys(0) end
 if f==1550 then check(emu:read16(gNativeSaveNotice)==1,'charged Marluxia encounter suspends');emu:reset() end
 if f==1880 then
  check(emu:read16(gNativeMarlReady)==1 and emu:read16(gNativeEnemyHp)==56,'resume reconstructs Marluxia artwork and HP')
  check(emu:read8(gNativeEnemyCharge)==1 and emu:read16(gNativeMarlPose)==1 and emu:read16(gNativeThreats)==10,'resume preserves charged scythe preview and pose')
  emu:setKeys(256)
 end
 if f==1884 then emu:setKeys(0) end
 if f==1910 or f==1940 then emu:setKeys(256) end
 if f==1914 or f==1944 then emu:setKeys(0) end
 if f==1970 then emu:setKeys(1) end
 if f==1974 then emu:setKeys(0) end
 if f==2020 then check(emu:read16(gNativeGuard)==1,'native Guard card prepares scythe defense');check(emu:read16(gNativeThreats)==1,'Guard preview reduces scythe to one damage');emu:setKeys(8) end
 if f==2024 then emu:setKeys(0) end
 if f==2130 then check(emu:read8(gNativePartyHealth)==69,'Guard reduces actual Marluxia scythe to one damage') end
 if f==2170 then
  place(64,0,1);emu:write16(gNativeEnemyHp,28)
  emu:write16(gNativeGuard,0)
 end
 if f==2190 then
  check(emu:read16(gNativeThreats)==14,'half-health enrage previews fourteen damage')
  check(hudText(2,0,'RAGE SCYTHE CHARGED'),'compact HUD names enraged scythe windup')
  local e=emu:read32(emu:read32(sEnemyTasks)+4)
  emu:write32(e+12,emu:read32(e+12)+16*256)
 end
 if f==2210 then
  check(emu:read16(gNativeThreats)==14,'scythe includes exact sixteen-pixel depth')
  local e=emu:read32(emu:read32(sEnemyTasks)+4)
  emu:write32(e+12,emu:read32(e+12)+256)
 end
 if f==2230 then
  check(emu:read16(gNativeThreats)==0,'scythe excludes seventeen-pixel depth')
  place(64,0,1);emu:setKeys(8)
 end
 if f==2234 then emu:setKeys(0) end
 if f==2340 then
  check(emu:read8(gNativePartyHealth)==55,'enraged scythe resolves fourteen damage')
 end
 if f==2630 then
  -- HP/card selection fixture; native Fire performs defeat and task release.
  emu:write16(gNativeEnemyHp,1);emu:write8(gNativeDeck+73,1);place(64,0,0);emu:setKeys(1)
 end
 if f==2634 then emu:setKeys(0) end
 if f==2720 then check(emu:read16(gNativeEnemyHp)==0 and emu:read32(sEnemyTasks)==0,'native Fire defeats Marluxia and releases field task') end
 if f==2750 then emu:setKeys(1)end
 if f==2754 then emu:setKeys(0)end
 if f==2780 then transition(0)end
 if f==2870 then
  out:write('EXIT ready '..emu:read16(gNativeMarlReady)..' room '..emu:read8(gMapFloorState+6)..'\n')
  for i=0,3 do out:write('CARD '..i..' '..emu:read32(sCardTiles+i*4)..' '..emu:read32(sCardPalettes+i*4)..'\n') end
  check(emu:read16(gNativeMarlReady)==0 and cards(),'room exit releases Marluxia resources and reconstructs card art')
  out:close()
 end
end)
