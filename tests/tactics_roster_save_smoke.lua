local f=0
local out=io.open('@OUTPUT@/moves.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function place(distance)
 local p=emu:read32(gFieldState)
 for i=0,1 do
  local t=emu:read32(sEnemyTasks+i*4)
  if t~=0 then
   local w=emu:read32(t+4)
   for j=0,3 do emu:write32(w+8+j*4,emu:read32(p+0x18+j*4)) end
   emu:write32(w+8,emu:read32(w+8)+distance*256)
   emu:write16(gNativeEnemyHp+i*2,30)
  end
 end
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(1) end
 if f==184 then emu:setKeys(0) end
 if f==200 then emu:setKeys(516) end
 if f==204 then emu:setKeys(0) end
 if f==230 then check(emu:read16(gNativeParty)==1,'Select controls Donald');place(64) end
 if f==250 then check(emu:read16(gNativeFireDamage)==14,'Donald attack card previews ranged magic');emu:setKeys(1) end
 if f==254 then emu:setKeys(0) end
 if f==310 then
  check(emu:read16(gNativeEnemyHp)==16,'Donald attack card resolves magic without Sora melee')
  check(emu:read16(gNativeEnemyHp+2)==30,'Donald magic damages one target')
  emu:setKeys(516)
 end
 if f==314 then emu:setKeys(0) end
 if f==350 then
  check(emu:read16(gNativeParty)==2,'Select controls Goofy')
  place(32);emu:write8(gNativeDeck+48,1);emu:write8(gNativeDeck+73,0)
 end
 if f==380 then
  check(emu:read16(gNativeSkillDamage)==9 and emu:read16(gNativeSkillDamage+2)==9,'Goofy spin previews both nearby enemies')
  emu:setKeys(1)
 end
 if f==384 then emu:setKeys(0) end
 if f==450 then
  check(emu:read16(gNativeEnemyHp)==21 and emu:read16(gNativeEnemyHp+2)==21,'Goofy shield spin resolves both previewed hits')
  check(emu:read16(gNativeActionLeft)==0,'character attack spends selected member action')

 end
 if f==470 then
  -- Explicit unlock/upgrade fixture; this does not prove recruitment UI.
  emu:write8(gNativeRoster,15);emu:write8(gNativeRoster+7,3)
  emu:write8(gNativeRoster+9,5);emu:setKeys(12)
 end
 if f==474 then emu:setKeys(0) end
 if f==520 then check(emu:read16(gNativeSaveNotice)==1,'native roster state suspends');emu:reset() end
 if f==850 then
  check(emu:read8(gNativeRoster)==15,'resume preserves unlocked Cloud roster card')
  check(emu:read8(gNativeRoster+7)==3 and emu:read8(gNativeRoster+9)==5,'resume preserves character power and sleight unlocks')
  out:close()
 end
end)
