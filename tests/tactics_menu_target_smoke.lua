-- Explicit party-health and valid Cure hand fixture; all menu and target
-- selection, confirmation and cancellation use native controller input.
local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(16) end
 if f==200 then emu:setKeys(128) end
 if f==220 then emu:setKeys(8) end
 if f==184 or f==204 or f==224 or f==264 or f==304 or f==344 or f==384 or f==424 or f==464 or f==504 or f==544 or f==584 then emu:setKeys(0) end
 if f==260 then
  check(emu:read16(gNativeParty)==1 and emu:read8(gNativeRoster+2)==4,'Rally deployed through setup input')
  emu:write8(gNativePartyHealth,20);emu:write8(gNativePartyHealth+1,30)
  emu:write16(gGameState+0x32,30)
  for i=0,11 do emu:write8(gNativeDeck+i,2);emu:write8(gNativeDeck+24+i,6);emu:write8(gNativeDeck+48+i,i<5 and 1 or 0) end
  emu:write8(gNativeDeck+73,0);emu:write8(gNativeDeck+77,0)
  emu:setKeys(260)
 end
 if f==300 or f==340 then emu:setKeys(128) end
 if f==380 or f==420 then emu:setKeys(1) end
 if f==460 then
  check(emu:read16(gNativeMenu)==3,'selected skill opens confirmation instead of firing')
  check(emu:read16(gNativeCureTarget)==0 and emu:read16(gNativeCureHeal)==18,'preview chooses injured Sora with Rally healing bonus')
  check(emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+48)==1,'target selection retains action and card')
  emu:setKeys(128)
 end
 if f==500 then
  check(emu:read16(gNativeCureTarget)==1 and emu:read16(gNativeCureHeal)==18,'D-pad targets Rally and updates recovery preview')
  emu:screenshot('@OUTPUT@/target.png');emu:setKeys(2)
 end
 if f==540 then
  check(emu:read16(gNativeMenu)==2,'cancel returns to skill selection')
  check(emu:read8(gNativePartyHealth+1)==30 and emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+48)==1,'cancel changes no health action or card')
  emu:setKeys(1)
 end
 if f==580 then emu:setKeys(1) end
 if f==620 then
  check(emu:read8(gNativePartyHealth+1)==48 and emu:read8(gNativePartyHealth)==20,'confirm heals only selected Rally by previewed amount')
  check(emu:read16(gNativeActionLeft)==0 and emu:read16(gNativeMoveLeft)==3,'confirmed Cure charges one action and no movement')
  check(emu:read8(gNativeDeck+48)~=1 and emu:read16(gNativeMenu)==0,'confirmed skill spends its card and closes menu')
  emu:screenshot('@OUTPUT@/healed.png')
 end
 if f==660 then
  emu:write16(gNativeActionLeft,1)
  for i=0,11 do emu:write8(gNativeDeck+i,1) end
  emu:write8(gNativeDeck+73,0)
  local field=emu:read32(gFieldState)
  for i=0,5 do
   local task=emu:read32(sEnemyTasks+i*4)
   if task~=0 then local w=emu:read32(task+4);emu:write32(w+8,emu:read32(field+0x18)+65536);emu:write16(gNativeEnemyHp+i*2,30) end
  end
  emu:setKeys(260)
 end
 if f==664 or f==704 or f==744 or f==784 or f==944 or f==984 then emu:setKeys(0) end
 if f==700 or f==740 or f==780 then emu:setKeys(1) end
 if f==820 then
  check(emu:read16(gNativeMenu)==3 and emu:read16(gNativeFireTarget)==65535,'empty ranged target keeps confirmation open')
  check(emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+49)==1,'empty target consumes no card or action')
  local field=emu:read32(gFieldState)
  for i=0,1 do
   local task=emu:read32(sEnemyTasks+i*4)
   if task~=0 then
    local w=emu:read32(task+4)
    emu:write32(w+8,emu:read32(field+0x18)+(i+1)*4096)
    emu:write32(w+12,emu:read32(field+0x1c));emu:write32(w+16,emu:read32(field+0x20))
   end
  end
 end
 if f==940 then
  check(emu:read16(gNativeFireTarget)==0,'restored eligible enemies update target confirmation')
  emu:setKeys(128)
 end
 if f==980 then
  check(emu:read16(gNativeFireTarget)==1 and emu:read16(gNativeFireDamage)==12,'menu cycles to farther enemy with predicted damage')
  check(emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+49)==1,'ranged target cycling spends no action or card')
  emu:screenshot('@OUTPUT@/fire-target.png');emu:setKeys(1)
 end
 if f==1020 then
  check(emu:read16(gNativeEnemyHp+2)==18,'confirmed Fire deals predicted damage to selected target')
  check(emu:read16(gNativeEnemyHp)==30,'confirmed Fire preserves unselected enemy health')
  check(emu:read16(gNativeActionLeft)==0 and emu:read8(gNativeDeck+49)==2,'confirmed ranged skill spends one card and action')
 end
 if f==1060 then
  emu:write16(gNativeActionLeft,1);emu:write8(gNativePartyHealth,20);emu:write8(gNativePartyHealth+1,30);emu:write16(gGameState+0x32,30)
  for i=0,11 do emu:write8(gNativeDeck+i,2);emu:write8(gNativeDeck+48+i,i<5 and 1 or 0) end
  emu:write8(gNativeDeck+73,0);emu:write8(gNativeDeck+77,0);emu:setKeys(260)
 end
 if f==1064 or f==1104 or f==1144 or f==1184 or f==1224 or f==1264 or f==1304 then emu:setKeys(0) end
 if f==1100 or f==1260 or f==1300 then emu:setKeys(1) end
 if f==1140 or f==1180 or f==1220 then emu:setKeys(64) end
 if f==1260 then
  check(emu:read8(gNativeDeck+77)==3,'Skills menu stocks three cards through D-pad input')
  check(emu:read16(gNativeActionLeft)==1,'stocking preserves available action')
 end
 if f==1300 then
  check(emu:read16(gNativeMenu)==3 and emu:read8(gNativeDeck+77)==3,'sleight has a separate confirmation screen')
  emu:screenshot('@OUTPUT@/sleight.png')
 end
 if f==1340 then
  check(emu:read8(gNativePartyHealth)==62 and emu:read8(gNativePartyHealth+1)==64,'confirmed Rally Curaga heals party with bonus and independent caps')
  check(emu:read16(gNativeActionLeft)==0 and emu:read8(gNativeDeck+77)==0,'sleight consumes stocked cards and one action')
  out:close()
 end
end)
