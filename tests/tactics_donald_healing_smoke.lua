-- Native Donald deployment, stocking, healing, and suspend/reset.
-- Explicit health/card/upgrade fixtures isolate capped healing and revival.
-- Five-hero FieldRoster.sleights starts at byte9; Donald is hero1 (byte10).
local f=0
local out=io.open('@OUTPUT@/donald-healing.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(256) end
 if f==184 then emu:setKeys(0) end
 if f==210 then emu:setKeys(1) end
 if f==214 then emu:setKeys(0) end
 if f==240 then
  check(emu:read16(gNativeParty)==1 and emu:read16(gNativeAssembly)==0,'native assembly begins with Donald selected')
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then local w=emu:read32(t+4);emu:write32(w+8,2048);emu:write32(w+12,2048) end
  end
  emu:write8(gNativePartyHealth,20);emu:write8(gNativePartyHealth+1,46);emu:write8(gNativePartyHealth+2,0)
  -- Selected actor HP also lives in the original engine's game state.
  emu:write16(gGameState+0x32,46)
 end
 if f==260 or f==300 or f==340 then emu:setKeys(513) end
 if f==264 or f==304 or f==344 then emu:setKeys(0) end
 if f==360 then
  check(emu:read8(gNativeDeck+77)==3,'Donald stocks three cards through native input')
  for i=0,2 do emu:write8(gNativeDeck+i,2);emu:write8(gNativeDeck+24+i,6) end
 end
 if f==380 then
  check(emu:read16(gNativeSleightHeal)==46,'Donald party healing preview includes his eight-point specialty')
  check(emu:read16(gNativeSleightHeal+2)==10,'healing preview caps Donald recovery at missing health')
  check(emu:read16(gNativeSleightHeal+4)==46,'party healing preview includes knocked-out Goofy')
  check(emu:read16(gNativeActionLeft)==1,'healing preview preserves the action')
  emu:write8(gNativeRoster+10,4)
 end
 if f==400 then
  check(emu:read16(gNativeSleightHeal)==50 and emu:read16(gNativeSleightHeal+4)==50,'owned Cure enhancement stacks with Donald specialty')
  check(emu:read16(gNativeSleightHeal+2)==10,'owned enhancement retains missing-health cap')
  emu:screenshot('@OUTPUT@/donald-healing-preview.png');emu:setKeys(1)
 end
 if f==404 then emu:setKeys(0) end
 if f==430 then
  check(emu:read8(gNativePartyHealth)==70 and emu:read8(gNativePartyHealth+1)==56 and emu:read8(gNativePartyHealth+2)==50,'native execution matches every predicted heal and revives Goofy')
  check(emu:read16(gNativeActionLeft)==0 and emu:read16(gNativeSleights)==1,'healing sleight resolves once and spends one action')
  check(emu:read8(gNativeDeck+48)==3 and emu:read8(gNativeDeck+49)==2 and emu:read8(gNativeDeck+50)==2,'healing sleight exhausts only first card and discards the other two')
  check(emu:read16(gNativeSleightHeal)==0 and emu:read16(gNativeSleightHeal+4)==0,'resolved healing clears previews')
  emu:screenshot('@OUTPUT@/donald-healing-arrival.png');emu:setKeys(12)
 end
 if f==434 then emu:setKeys(0) end
 if f==470 then
  check(emu:read16(gNativeSaveNotice)==1,'healed party and depleted deck suspend successfully')
  emu:reset()
 end
 if f==710 then
  check(emu:read16(gNativeParty)==1 and emu:read16(gNativeActionLeft)==0,'reset retains Donald selection and spent action')
  check(emu:read8(gNativePartyHealth)==70 and emu:read8(gNativePartyHealth+1)==56 and emu:read8(gNativePartyHealth+2)==50,'reset retains capped healing and revived companion')
  check(emu:read8(gNativeRoster+10)==4,'reset retains Donald owned Cure enhancement')
  check(emu:read8(gNativeDeck+48)==3 and emu:read8(gNativeDeck+49)==2 and emu:read8(gNativeDeck+50)==2,'reset retains exact first-card exhaustion and discarded cards')
  out:close()
 end
end)
