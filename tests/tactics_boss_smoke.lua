local f=0
local out=io.open('@OUTPUT@/boss.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function enemy(i) local t=emu:read32(sEnemyTasks+i*4);if t==0 then return 0 end return emu:read32(t+4) end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local r=emu:read32(gMapRoomState)
  emu:write8(r+15,7);emu:write8(r+16,1)
  emu:write32(field()+0x70,emu:read32(field()+0x70)|16)
 end
 if f==340 then
  check(emu:read8(gNativeEnemyKind)==2,'exit encounter spawns native Large Body guardian task')
  check(emu:read16(gNativeEnemyHp)==40,'Large Body guardian uses boss health')
  local p=field();local e=enemy(0)
  for j=0,3 do emu:write32(e+8+j*4,emu:read32(p+0x18+j*4)) end
  emu:write32(e+8,emu:read32(e+8)+8192)
  emu:write32(sPartyPos+16,emu:read32(p+0x18)+49152)
  for i=1,5 do
   e=enemy(i)
   if e~=0 then emu:write32(e+8,emu:read32(p+0x18)+131072) end
  end
 end
 if f==380 then emu:screenshot('@OUTPUT@/guardian.png');emu:setKeys(8) end
 if f==384 then emu:setKeys(0) end
 if f==460 then
  check(emu:read8(gNativeEnemyCharge)==1,'first boss decision telegraphs a charged attack')
  check(emu:read8(gNativePartyHealth)==80,'windup deals no immediate damage')
  check(emu:read16(gNativeThreats)==10,'NEXT warns Sora of charged area damage')
  check(emu:read16(gNativeThreats+2)==0,'NEXT excludes Donald outside boss area')
  emu:screenshot('@OUTPUT@/charging.png');emu:setKeys(12)
 end
 if f==464 then emu:setKeys(0) end
 if f==500 then check(emu:read16(gNativeSaveNotice)==1,'charged boss encounter saves');emu:reset() end
 if f==740 then
  check(emu:read8(gNativeEnemyKind)==2 and emu:read8(gNativeEnemyCharge)==1,'reset restores Large Body guardian identity and windup')
  check(emu:read16(gNativeEnemyHp)==40,'reset preserves boss health')
  emu:write32(sPartyPos+32,emu:read32(field()+0x18)+49152)
 end
 if f==780 then emu:setKeys(8) end
 if f==784 then emu:setKeys(0) end
 if f==880 then
  check(emu:read8(gNativeEnemyCharge)==0,'charged attack resolves once')
  check(emu:read8(gNativePartyHealth)==70,'charged attack damages Sora inside area')
  check(emu:read8(gNativePartyHealth+1)==56 and emu:read8(gNativePartyHealth+2)==72,'party outside area evades charged attack')
  emu:setKeys(8)
 end
 if f==884 then emu:setKeys(0) end
 if f==980 then check(emu:read8(gNativeEnemyCharge)==1,'boss returns to windup before its next attack') end
 if f==1000 or f==1030 or f==1060 then emu:setKeys(256) end
 if f==1004 or f==1034 or f==1064 then emu:setKeys(0) end
 if f==1090 then emu:setKeys(1) end
 if f==1094 then emu:setKeys(0) end
 if f==1140 then check(emu:read16(gNativeGuard)==1,'real Guard card prepares defense');emu:setKeys(8) end
 if f==1144 then emu:setKeys(0) end
 if f==1240 then
  check(emu:read8(gNativePartyHealth)==69,'Guard reduces boss area attack to one damage')
  emu:write16(gNativeEnemyHp,1)
  local slot=0
  for i=0,emu:read8(gNativeDeck+72)-1 do
   if emu:read8(gNativeDeck+48+i)==1 then
    if emu:read8(gNativeDeck+i)==1 then emu:write8(gNativeDeck+73,slot);break end
    slot=slot+1
   end
  end
  emu:setKeys(1)
 end
 if f==1244 then emu:setKeys(0) end
 if f==1320 then
  check(emu:read16(gNativeEnemyHp)==0 and emu:read32(sEnemyTasks)==0,'native Fire card defeats boss and releases its task')
  emu:screenshot('@OUTPUT@/boss-defeated.png');out:close()
 end
end)
