-- Explicit position fixture; Fire is selected and played through native input.
local f=0
local out=io.open('@OUTPUT@/spawn.txt','w')
local slot=nil
local target=nil
local targetX=nil
local targetZ=nil
local initialHp=nil
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local aligned=true;local count=0
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then
    local w=emu:read32(t+4);count=count+1
    aligned=aligned and emu:read32(w+16)==emu:read32(w+20)
    if emu:read8(gNativeEnemyKind+i)==1 then slot=i;target=w
    else emu:write32(w+8,0);emu:write32(w+12,0);emu:write32(w+16,0) end
   end
  end
  check(count==2,'native entry encounter contains both seeded actors')
  check(aligned,'all tactical actors spawn on their assigned floor instead of above the ceiling')
  check(target~=nil,'seeded encounter includes an original Red Nocturne')
  if target then
   local p=emu:read32(gFieldState)
   emu:write32(p+0x18,emu:read32(target+8));emu:write32(p+0x1c,emu:read32(target+12)+2048)
   emu:write32(p+0x20,emu:read32(target+20));emu:write32(p+0x24,emu:read32(target+20));emu:write8(p+0x2c,0)
  end
 end
 if f==200 then emu:setKeys(256) end
 if f==204 then emu:setKeys(0) end
 if f==220 then
  check(slot and emu:read16(gNativeFireTarget)==slot,'Fire preview identifies the same fresh enemy before play')
  check(slot and emu:read16(gNativeFireDamage)==emu:read16(gNativeEnemyHp+slot*2),'Fire preview caps lethal damage at remaining enemy HP')
  emu:screenshot('@OUTPUT@/fire-preview.png')
 end
 if f==224 and target then targetX=emu:read32(target+8);emu:write32(target+8,458752) end
 if f==230 then
  check(emu:read16(gNativeFireTarget)==65535 and emu:read16(gNativeFireDamage)==0,'Fire preview clears when no enemy is in range')
  check(emu:read16(gNativeActionLeft)==1,'inspecting an empty Fire target preserves the action')
 end
 if f==234 and targetX then emu:write32(target+8,targetX) end
 if f==240 and target then targetZ=emu:read32(target+16);emu:write32(target+16,targetZ-8192) end
 if f==246 then check(emu:read16(gNativeFireTarget)==65535,'Fire preview rejects a nearby enemy above the height limit') end
 if f==250 and targetZ then emu:write32(target+16,targetZ) end
 if f==256 then
  initialHp=emu:read16(gNativeEnemyHp+slot*2)
  emu:write8(gNativeDeck+25,1)
 end
 if f==262 then
  check(emu:read16(gNativeFireTarget)==slot and emu:read16(gNativeFireDamage)==0,'low-value Fire warns of a card break on the correct target')
  emu:screenshot('@OUTPUT@/fire-break.png');emu:setKeys(1)
 end
 if f==266 then emu:setKeys(0) end
 if f==280 then
  check(emu:read16(gNativeEnemyHp+slot*2)==initialHp and emu:read16(gNativeBreaks)==1,'playing the previewed broken card deals no damage and records one break')
  check(emu:read16(gNativeActionLeft)==0,'broken Fire still spends its combat action')
  emu:setKeys(8)
 end
 if f==284 then emu:setKeys(0) end
 if f==350 then emu:setKeys(768) end
 if f==354 then emu:setKeys(0) end
 if f==420 then emu:setKeys(8) end
 if f==424 then emu:setKeys(0) end
 if f==490 then
  emu:write8(gNativeDeck+25,6)
  -- Restore one selected hand card as a fixture for the independent lethal
  -- case; the preceding break remains actual native input and damage.
  for i=0,emu:read8(gNativeDeck+72)-1 do emu:write8(gNativeDeck+48+i,i==1 and 1 or 0) end
  emu:write8(gNativeDeck+73,0)
  local p=emu:read32(gFieldState)
  emu:write32(p+0x18,emu:read32(target+8));emu:write32(p+0x1c,emu:read32(target+12)+2048)
  emu:write32(p+0x20,emu:read32(target+16));emu:write32(p+0x24,emu:read32(target+20))
 end
 if f==510 then emu:setKeys(1) end
 if f==514 then emu:setKeys(0) end
 if f==650 then
  check(slot and emu:read32(sEnemyTasks+slot*4)==0,'a real Fire card defeats the fresh ranged actor from its floor')
  check(emu:read16(gNativeKills)==1,'Fire awards exactly one native kill')
  check(emu:read16(gNativeActionLeft)==0,'Fire targeting consumes the normal combat action')
  emu:screenshot('@OUTPUT@/fire.png');out:close()
 end
end)
