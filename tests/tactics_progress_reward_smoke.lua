local f=0
local out=io.open('@OUTPUT@/progress.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function place()
 local p=emu:read32(gFieldState)
 for i=0,1 do
  local t=emu:read32(sEnemyTasks+i*4)
  if t~=0 then local w=emu:read32(t+4)
   for j=0,3 do emu:write32(w+8+j*4,emu:read32(p+0x18+j*4)) end
   emu:write32(w+8,emu:read32(w+8)+32*256);emu:write16(gNativeEnemyHp+i*2,1)
  end
 end
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(1) end
 if f==184 then emu:setKeys(0) end
 if f==210 then
  -- Explicit earlier-clear fixture; native attacks perform the next clear.
  emu:write16(gNativeRoster+12,2);place();emu:write8(gNativeDeck+73,1);emu:setKeys(1)
 end
 if f==214 then emu:setKeys(0) end
 if f==260 then emu:setKeys(4) end
 if f==264 then emu:setKeys(0) end
 if f==290 then emu:write8(gNativeDeck+73,0);emu:setKeys(1) end
 if f==294 then emu:setKeys(0) end
 if f==360 then
  check(emu:read16(gNativeEnemyHp)==0 and emu:read16(gNativeEnemyHp+2)==0,'native card attacks clear encounter')
  check(emu:read16(gNativeProgressReward)==1,'second new clear opens character reward')
  check(emu:read8(gNativeRoster+14)==2 and emu:read8(gNativeRoster+15)==1,'pending upgrade is authoritative roster state')
  check(emu:read16(gNativeRoster+12)==3,'clear records each room once')
  emu:screenshot('@OUTPUT@/reward.png');emu:setKeys(12)
 end
 if f==364 then emu:setKeys(0) end
 if f==420 then check(emu:read16(gNativeSaveNotice)==1,'pending character reward suspends');emu:reset() end
 if f==750 then
  check(emu:read16(gNativeProgressReward)==1 and emu:read16(gNativeAssembly)==0,'resume restores reward without opening setup')
  check(emu:read16(gNativeRoster+12)==3,'resume preserves cleared encounter mask')
  emu:setKeys(256)
 end
 if f==754 then emu:setKeys(0) end
 if f==780 then emu:setKeys(1) end
 if f==784 then emu:setKeys(0) end
 if f==830 then
  check(emu:read16(gNativeProgressReward)==0,'confirmation closes reward')
  check(emu:read8(gNativeRoster+5)==1 and emu:read8(gNativeRoster+4)==0,'power reward upgrades Donald only')
  check(emu:read8(gNativeRoster+14)==0,'confirmed reward returns roster to assembly phase')
  emu:setKeys(12)
 end
 if f==834 then emu:setKeys(0) end
 if f==890 then emu:reset() end
 if f==1220 then
  check(emu:read8(gNativeRoster+5)==1,'confirmed Donald power persists through reboot')
  check(emu:read16(gNativeProgressReward)==0,'confirmed reward is not offered twice')

 end
 if f==1250 then emu:setKeys(1) end
 if f==1254 then emu:setKeys(0) end
 if f==1280 then
  local r=emu:read32(gMapRoomState);local p=emu:read32(gFieldState)
  emu:write8(r+15,1);emu:write8(r+16,1);emu:write32(p+0x70,emu:read32(p+0x70)|16)
 end
 if f==1420 then emu:setKeys(1) end
 if f==1424 then emu:setKeys(0) end
 if f==1500 then emu:setKeys(8) end
 if f==1504 then emu:setKeys(0) end
 if f==1600 then
  place();for i=0,1 do emu:write16(gNativeEnemyHp+i*2,30) end
  emu:write8(gNativeDeck+48,1);emu:write8(gNativeDeck+49,1);emu:write8(gNativeDeck+73,1)
 end
 if f==1640 then
  check(emu:read16(gNativeFireDamage)==16,'Donald power upgrade adds one to predicted magic damage')
  emu:setKeys(1)
 end
 if f==1644 then emu:setKeys(0) end
 if f==1720 then
  check(emu:read16(gNativeEnemyHp)==14,'Donald power upgrade adds one to actual magic damage')
  out:close()
 end
end)
