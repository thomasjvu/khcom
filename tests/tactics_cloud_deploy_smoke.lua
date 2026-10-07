local f=0
local out=io.open('@OUTPUT@/deploy.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function place(distance)
 local p=emu:read32(gFieldState)
 for i=0,1 do local t=emu:read32(sEnemyTasks+i*4)
  if t~=0 then local w=emu:read32(t+4)
   for j=0,3 do emu:write32(w+8+j*4,emu:read32(p+0x18+j*4)) end
   emu:write32(w+8,emu:read32(w+8)+distance*256);emu:write16(gNativeEnemyHp+i*2,30)
  end
 end
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(256) end
 if f==184 then emu:setKeys(0) end
 if f==210 then emu:setKeys(256) end
 if f==214 then emu:setKeys(0) end
 if f==240 then emu:setKeys(64) end
 if f==244 then emu:setKeys(0) end
 if f==270 then
  check(emu:read8(gNativeRoster+3)~=3,'locked Cloud cannot deploy')
  -- Explicit recruitment/health fixture; assembly itself uses controller input.
  emu:write8(gNativeRoster,15);emu:write8(gNativeRoster+7,2)
  emu:write8(gNativePartyHealth+2,51);emu:write16(sPartyAction+4,0)
 end
 -- Slot 2 currently Donald after swapping with the only other unlocked hero.
 if f==300 or f==330 then emu:setKeys(64) end
 if f==304 or f==334 then emu:setKeys(0) end
 if f==360 then
  check(emu:read8(gNativeRoster+3)==3,'unlocked Cloud enters selected companion slot')
  check(emu:read8(gNativePartyHealth+2)==72 and emu:read8(gNativePartyHealth+5)==72,'Cloud uses his own health cap')
  check(emu:read32(sFriends+56)~=0 and emu:read32(sFriends+60)~=0,'original Cloud party art allocates')
  check(emu:read32(sAssemblyOtherTiles)~=0 and emu:read32(sAssemblyOtherPalette)~=0,'Cloud summon card appears during assembly')
  emu:screenshot('@OUTPUT@/cloud-party-setup.png');emu:setKeys(1)
 end
 if f==364 then emu:setKeys(0) end
 if f==400 then
  check(emu:read16(gNativeParty)==2 and emu:read16(gNativeAssembly)==0,'confirmation gives direct control of Cloud')
  place(32)
 end
 if f==430 then
  check(emu:read16(gNativeFireDamage)==19,'Cloud sword and personal power preview nineteen damage')
  emu:setKeys(1)
 end
 if f==434 then emu:setKeys(0) end
 if f==490 then
  check(emu:read16(gNativeEnemyHp)==11 and emu:read16(gNativeEnemyHp+2)==30,'Cloud sword resolves one previewed target')
  check(emu:read16(gNativeActionLeft)==0,'Cloud attack spends Cloud action')
  emu:setKeys(12)
 end
 if f==494 then emu:setKeys(0) end
 if f==550 then check(emu:read16(gNativeSaveNotice)==1,'deployed Cloud suspends');emu:reset() end
 if f==880 then
  check(emu:read8(gNativeRoster+3)==3 and emu:read16(gNativeParty)==2,'resume preserves Cloud deployment and control')
  check(emu:read8(gNativePartyHealth+2)==72 and emu:read8(gNativePartyHealth+5)==72,'resume preserves Cloud health and cap')
  check(emu:read8(gNativeRoster+7)==2 and emu:read16(gNativeAssembly)==0,'resume preserves hero upgrade and active battle')
  check(emu:read8(gNativeRoster+18)==51,'benching Donald preserves his injured health')
  check(emu:read8(gNativeRoster+26)==0,'benching Donald preserves his spent action')
  check(emu:read32(sFriends+56)~=0 and emu:read32(sCardTiles+12)~=0,'resume reconstructs party and combat Guard artwork')
  emu:screenshot('@OUTPUT@/cloud-controlled.png');out:close()
 end
end)
