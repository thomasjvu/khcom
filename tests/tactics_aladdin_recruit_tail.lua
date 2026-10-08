 if f==1825 then emu:setKeys(64)end
 if f==1829 then emu:setKeys(0)end
 if f==1850 then
  check(emu:read16(gNativeProgressReward)==1,'Jafar defeat opens boss rewards')
  check(hudText(5,2,'RECRUIT ALADDIN'),'Jafar offers Aladdin beside four personal upgrades')
  local card=emu:read32(sRecruitCard)
  check(card~=0 and emu:read32(sRecruitTiles)~=0 and emu:read32(sRecruitPalette)~=0,'original Aladdin character card allocates')
  emu:screenshot('@OUTPUT@/aladdin-reward.png')
 end
 if f==1860 then emu:setKeys(1)end
 if f==1864 then emu:setKeys(0)end
 if f==1920 then
  check(emu:read8(gNativeRoster)==55 and emu:read16(gNativeProgressReward)==0,'Aladdin unlock resolves native reward selection')
  check(emu:read32(sRecruitTiles)==0 and cards(),'reward restores original combat card banks')
 end
 if f==1940 then emu:setKeys(12)end
 if f==1944 then emu:setKeys(0)end
 if f==1980 then
  check(emu:read16(gNativeSaveNotice)==1,'resolved Jafar reward suspends')
  emu:reset()
 end
 if f==2310 then
  check(emu:read8(gNativeRoster)==55 and emu:read16(gNativeProgressReward)==0,'Aladdin unlock survives native reboot without duplicate reward')
  check(cards(),'resume reconstructs original combat artwork')
  out:close()
 end
end)
