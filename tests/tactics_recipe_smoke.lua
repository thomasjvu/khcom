-- Card/HP/position fixtures; stocking, turns and sleights use native input.
local f=0
local out=io.open('@OUTPUT@/recipes.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==170 then
  for i=0,2 do emu:write8(gNativeDeck+i,2);emu:write8(gNativeDeck+24+i,6) end
  emu:write16(gGameState+0x32,30);emu:write8(gNativePartyHealth+1,0);emu:write8(gNativePartyHealth+2,65)
 end
 if f==180 or f==220 or f==260 or f==540 or f==580 or f==620 then emu:setKeys(513) end
 if f==184 or f==224 or f==264 or f==544 or f==584 or f==624 then emu:setKeys(0) end
 if f==300 then
  check(emu:read8(gNativeDeck+77)==3,'three Cure cards stock through native input')
  check(emu:read16(gNativeActionLeft)==1,'Curaga inspection preserves action')
  check(emu:read16(gNativeSleightHeal)==38,'Curaga previews enhanced recovery for Sora')
  check(emu:read16(gNativeSleightHeal+2)==38,'Curaga previews exact Donald revival HP')
  check(emu:read16(gNativeSleightHeal+4)==7,'Curaga preview caps Goofy recovery at missing HP')
  emu:screenshot('@OUTPUT@/curaga.png');emu:setKeys(1)
 end
 if f==304 or f==404 or f==664 then emu:setKeys(0) end
 if f==340 then
  check(emu:read16(gNativeSleightHeal)==0 and emu:read16(gNativeSleightHeal+2)==0 and emu:read16(gNativeSleightHeal+4)==0,'resolved Curaga clears all party recovery previews')
  check(emu:read16(gGameState+0x32)==68,'Curaga adds eight healing to the base party sleight')
  check(emu:read8(gNativePartyHealth+1)==38,'Curaga revives knocked-out Donald with enhanced healing')
  check(emu:read8(gNativePartyHealth+2)==72,'Curaga caps Goofy healing at maximum HP')
  check(emu:read8(gNativeDeck+48)==3 and emu:read8(gNativeDeck+49)==2 and emu:read8(gNativeDeck+50)==2,'Curaga exhausts first card and discards the other two')
  check(emu:read16(gNativeActionLeft)==0 and emu:read16(gNativeSleights)==1,'Curaga costs exactly one action')
 end
 if f==400 then emu:setKeys(8) end
 if f==520 then
  -- Fresh hand fixture isolates enhanced melee after the real enemy phase.
  for i=0,11 do emu:write8(gNativeDeck+48+i,i<5 and 1 or 0) end
  emu:write8(gNativeDeck+73,0)
  for i=0,2 do emu:write8(gNativeDeck+i,0);emu:write8(gNativeDeck+24+i,6) end
  local p=emu:read32(gFieldState)
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then
    local w=emu:read32(t+4)
    emu:write32(w+8,emu:read32(p+0x18)+(i==0 and 4096 or 65536))
    emu:write32(w+12,emu:read32(p+0x1c));emu:write32(w+16,emu:read32(p+0x20))
    emu:write16(gNativeEnemyHp+i*2,40)
   end
  end
 end
 if f==660 then
  check(emu:read16(gNativeSleightDamage)==32,'Triple Key previews enhanced melee power')
  check(emu:read16(gNativeSleightDamage+2)==0,'Triple Key excludes distant enemies')
  emu:screenshot('@OUTPUT@/triple-key.png');emu:setKeys(1)
 end
 if f==700 then
  check(emu:read16(gNativeEnemyHp)==8,'Triple Key deals the exact enhanced preview damage')
  check(emu:read16(gNativeEnemyHp+2)==40,'Triple Key leaves distant enemies unharmed')
  check(emu:read16(gNativeActionLeft)==0 and emu:read16(gNativeSleights)==2,'second recipe uses one refreshed action')
  out:close()
 end
end)
