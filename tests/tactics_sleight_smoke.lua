local f=0
local predicted=0
local out=io.open('@OUTPUT@/sleights.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==140 then emu:setKeys(1) end
 if f==144 then emu:setKeys(0) end
 if f==180 or f==220 or f==260 then emu:setKeys(513) end
 if f==184 or f==224 or f==264 then emu:setKeys(0) end
 if f==300 then
  check(emu:read8(gNativeDeck+77)==3,'L A stocks three distinct cards')
  check(emu:read16(gNativeActionLeft)==1,'stocking does not spend active action')
  check(emu:read8(gNativeDeck+48)==4,'stocked first card leaves the hand')
  emu:screenshot('@OUTPUT@/stocked.png');emu:setKeys(12)
 end
 if f==304 then emu:setKeys(0) end
 if f==340 then check(emu:read16(gNativeSaveNotice)==1,'stocked hand saves');emu:reset() end
 if f==580 then
  check(emu:read8(gNativeDeck+77)==3,'reset restores stocked cards')
  local p=emu:read32(gFieldState)
  local t=emu:read32(sEnemyTasks)
  if t~=0 then
   local e=emu:read32(t+4)
   for j=0,3 do emu:write32(e+8+j*4,emu:read32(p+0x18+j*4)) end
  end
 end
 if f==590 then
  predicted=emu:read16(gNativeSleightDamage)
  check(predicted>0 and predicted==emu:read16(gNativeEnemyHp),
    'sleight preview caps damage at nearby enemy HP')
  emu:screenshot('@OUTPUT@/area-preview.png');emu:setKeys(1)
 end
 if f==594 then emu:setKeys(0) end
 if f==620 then
  check(emu:read16(gNativeSleights)==1,'A resolves stocked sleight in native field')
  check(emu:read32(gNativeKills)>=1,'melee sleight damages nearby original field enemy')
  check(emu:read8(gNativeDeck+48)==3,'first sleight card is exhausted')
  check(emu:read8(gNativeDeck+49)==2 and emu:read8(gNativeDeck+50)==2,'other sleight cards enter discard')
  check(emu:read16(gNativeActionLeft)==0,'sleight costs active action')
  emu:setKeys(8)
 end
 if f==624 then emu:setKeys(0) end
 if f==760 then emu:setKeys(768) end
 if f==764 then emu:setKeys(0) end
 if f==800 then
  check(emu:read8(gNativeDeck+48)==3,'reload does not recover exhausted card')
  check(emu:read8(gNativeDeck+49)~=2,'reload recovers non-exhausted sleight cards')
  emu:setKeys(513)
 end
 if f==804 then emu:setKeys(0) end
 if f==840 then emu:setKeys(514) end
 if f==844 then emu:setKeys(0) end
 if f==880 then
  check(emu:read8(gNativeDeck+77)==0,'L B cancels stock without exhaustion')
  emu:screenshot('@OUTPUT@/sleight.png');out:close()
 end
end)
