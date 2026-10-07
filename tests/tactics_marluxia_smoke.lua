local f=0
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
callbacks:add('frame',function()
 f=f+1
 if f==180 then transition(7) end
 if f==320 then
  -- Forced floor advancement is fixture setup, not complete-run proof.
  emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253)
 end
 if f==380 then emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(7) end
 if f==420 then emu:write8(gMapFloorState+0x1c+7*16+11,0);transition(253) end
 if f==460 then check(emu:read16(gNativeFloor)==2,'fixture advances to Castle');transition(7) end
 if f==600 then
  check(emu:read16(gNativeMarlReady)==1,'Marluxia replaces Castle elite presentation')
  check(emu:read16(gNativeEnemyHp)==56,'Marluxia starts with 56 HP')
  check(emu:read32(sMarlTiles)~=0 and emu:read32(sMarlTiles+4)~=0 and emu:read32(sMarlPalette)~=0,'original Marluxia idle/scythe art and palette allocate')
  check(cards(),'Marluxia room retains all card artwork')
  emu:screenshot('@OUTPUT@/marluxia.png');place(96,24,0)
 end
 if f==620 then emu:setKeys(8) end
 if f==624 then emu:setKeys(0) end
 if f==730 then
  check(emu:read8(gNativeEnemyCharge)==1 and emu:read8(gNativePartyHealth)==80,'Marluxia winds up without immediate damage')
  check(emu:read16(gNativeThreats)==10,'scythe includes exact 96-pixel range and 24-pixel height')
  check(emu:read16(gNativeThreats+2)==10 and emu:read16(gNativeThreats+4)==10,'scythe previews all members within the sweep')
  check(emu:read16(gNativeMarlPose)==1,'charged scythe displays original scythe windup')
  emu:screenshot('@OUTPUT@/marluxia-charge.png');emu:setKeys(8)
 end
 if f==734 then emu:setKeys(0) end
 if f==840 then
  check(emu:read8(gNativePartyHealth)==70,'scythe resolves ten damage to previewed Sora target')
  check(emu:read8(gNativePartyHealth+1)==46 and emu:read8(gNativePartyHealth+2)==62,'scythe resolves previewed damage to Donald and Goofy')
  check(emu:read8(gNativeEnemyCharge)==0 and emu:read16(gNativeMarlPose)==0,'scythe consumes windup and returns to idle')
  place(97,24,1)
 end
 if f==860 then check(emu:read16(gNativeThreats)==0,'scythe excludes 97-pixel range');emu:setKeys(8) end
 if f==864 then emu:setKeys(0) end
 if f==970 then check(emu:read8(gNativePartyHealth)==70,'out-of-range scythe does no damage');place(96,25,1) end
 if f==990 then check(emu:read16(gNativeThreats)==0,'scythe excludes 25-pixel height');emu:setKeys(8) end
 if f==994 then emu:setKeys(0) end
 if f==1100 then check(emu:read8(gNativePartyHealth)==70,'out-of-height scythe does no damage');place(64,0,1);emu:setKeys(12) end
 if f==1104 then emu:setKeys(0) end
 if f==1150 then check(emu:read16(gNativeSaveNotice)==1,'charged Marluxia encounter suspends');emu:reset() end
 if f==1480 then
  check(emu:read16(gNativeMarlReady)==1 and emu:read16(gNativeEnemyHp)==56,'resume reconstructs Marluxia artwork and HP')
  check(emu:read8(gNativeEnemyCharge)==1 and emu:read16(gNativeMarlPose)==1 and emu:read16(gNativeThreats)==10,'resume preserves charged scythe preview and pose')
  emu:setKeys(256)
 end
 if f==1484 then emu:setKeys(0) end
 if f==1510 or f==1540 then emu:setKeys(256) end
 if f==1514 or f==1544 then emu:setKeys(0) end
 if f==1570 then emu:setKeys(1) end
 if f==1574 then emu:setKeys(0) end
 if f==1620 then check(emu:read16(gNativeGuard)==1,'native Guard card prepares scythe defense');check(emu:read16(gNativeThreats)==1,'Guard preview reduces scythe to one damage');emu:setKeys(8) end
 if f==1624 then emu:setKeys(0) end
 if f==1730 then check(emu:read8(gNativePartyHealth)==69,'Guard reduces actual Marluxia scythe to one damage') end
 if f==1770 then
  place(64,0,1);emu:write16(gNativeEnemyHp,28)
  emu:write16(gNativeGuard,0)
 end
 if f==1790 then
  check(emu:read16(gNativeThreats)==14,'half-health enrage previews fourteen damage')
  local e=emu:read32(emu:read32(sEnemyTasks)+4)
  emu:write32(e+12,emu:read32(e+12)+16*256)
 end
 if f==1810 then
  check(emu:read16(gNativeThreats)==14,'scythe includes exact sixteen-pixel depth')
  local e=emu:read32(emu:read32(sEnemyTasks)+4)
  emu:write32(e+12,emu:read32(e+12)+256)
 end
 if f==1830 then
  check(emu:read16(gNativeThreats)==0,'scythe excludes seventeen-pixel depth')
  place(64,0,1);emu:setKeys(8)
 end
 if f==1834 then emu:setKeys(0) end
 if f==1940 then
  check(emu:read8(gNativePartyHealth)==55,'enraged scythe resolves fourteen damage')
 end
 if f==2230 then
  -- HP/card selection fixture; native Fire performs defeat and task release.
  emu:write16(gNativeEnemyHp,1);emu:write8(gNativeDeck+73,1);place(64,0,0);emu:setKeys(1)
 end
 if f==2234 then emu:setKeys(0) end
 if f==2320 then check(emu:read16(gNativeEnemyHp)==0 and emu:read32(sEnemyTasks)==0,'native Fire defeats Marluxia and releases field task');transition(0) end
 if f==2470 then
  out:write('EXIT ready '..emu:read16(gNativeMarlReady)..' room '..emu:read8(gMapFloorState+6)..'\n')
  for i=0,3 do out:write('CARD '..i..' '..emu:read32(sCardTiles+i*4)..' '..emu:read32(sCardPalettes+i*4)..'\n') end
  check(emu:read16(gNativeMarlReady)==0 and cards(),'room exit releases Marluxia resources and reconstructs card art')
  out:close()
 end
end)
