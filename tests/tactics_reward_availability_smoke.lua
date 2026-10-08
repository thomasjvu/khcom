-- Explicit near-cap reward fixture; selection/confirmation use controller input.
local frame=0
local out=io.open('@OUTPUT@/availability.txt','w')
local function check(ok,message)
 out:write((ok and 'PASS ' or 'FAIL ')..message..'\n');out:flush()
end
callbacks:add('frame',function()
 frame=frame+1
 if frame==180 then emu:setKeys(1) end
 if frame==184 then emu:setKeys(0) end
 if frame==210 then
  emu:write16(gNativeProgressReward,1)
  emu:write8(gNativeRoster+rosterPhaseOffset,2);emu:write8(gNativeRoster+rosterRewardOffset,1)
  emu:write8(gNativeRoster+4,8);emu:write8(gNativeRoster+rosterSleightOffset,1)
  emu:write16(sProgressHero,0);emu:write16(sProgressKind,0)
 end
 if frame==240 then
  check(emu:read16(gNativeProgressReward)==1,'reward fixture opens without assembly')
  emu:screenshot('@OUTPUT@/power-max.png');emu:setKeys(1)
 end
 if frame==244 then emu:setKeys(0) end
 if frame==270 then
  check(emu:read8(gNativeRoster+4)==8,'capped power does not overflow')
  check(emu:read16(gNativeProgressReward)==1,'unavailable power preserves reward')
  emu:setKeys(128)
 end
 if frame==274 then emu:setKeys(0) end
 if frame==300 then
  check(emu:read16(sProgressKind)==1,'controller selects owned Key sleight')
  emu:screenshot('@OUTPUT@/sleight-owned.png');emu:setKeys(1)
 end
 if frame==304 then emu:setKeys(0) end
 if frame==330 then
  check(emu:read16(gNativeProgressReward)==1 and emu:read8(gNativeRoster+rosterSleightOffset)==1,'owned sleight preserves reward and unlocks')
  emu:setKeys(128)
 end
 if frame==334 then emu:setKeys(0) end
 if frame==360 then
  check(emu:read16(sProgressKind)==2,'controller selects available Fire sleight')
  emu:screenshot('@OUTPUT@/sleight-available.png');emu:setKeys(1)
 end
 if frame==364 then emu:setKeys(0) end
 if frame==400 then
  check(emu:read16(gNativeProgressReward)==0 and emu:read8(gNativeRoster+rosterSleightOffset)==3,'available choice consumes reward and adds only Fire unlock')
  out:close()
 end
end)
