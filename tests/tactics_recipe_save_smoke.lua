-- Matching-card setup fixture; stocking, suspend, reset and resolution are native.
local f=0
local out=io.open('@OUTPUT@/recipe-save.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==170 then
  for i=0,2 do emu:write8(gNativeDeck+i,0);emu:write8(gNativeDeck+24+i,6) end
 end
 if f==180 or f==220 or f==260 then emu:setKeys(513) end
 if f==184 or f==224 or f==264 or f==304 or f==604 then emu:setKeys(0) end
 if f==300 then emu:setKeys(12) end
 if f==340 then
  check(emu:read16(gNativeSaveNotice)==1,'matching-card recipe stock saves successfully')
  emu:reset()
 end
 if f==580 then
  check(emu:read8(gNativeDeck+77)==3,'reset restores all three enhanced recipe cards')
  local exact=true
  for i=0,2 do exact=exact and emu:read8(gNativeDeck+i)==0 and emu:read8(gNativeDeck+24+i)==6 and emu:read8(gNativeDeck+48+i)==4 end
  check(exact,'reset preserves matching kinds values and stock piles exactly')
  check(emu:read16(gNativeActionLeft)==1,'reset preserves unspent recipe action')
  local p=emu:read32(gFieldState)
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then
    local w=emu:read32(t+4)
    emu:write32(w+8,emu:read32(p+0x18)+(i==0 and 4096 or 65536))
    emu:write32(w+12,emu:read32(p+0x1c));emu:write32(w+16,emu:read32(p+0x20));emu:write16(gNativeEnemyHp+i*2,40)
   end
  end
 end
 if f==600 then
  check(emu:read16(gNativeSleightDamage)==32,'restored stock reconstructs enhanced recipe damage preview')
  emu:screenshot('@OUTPUT@/restored-recipe.png');emu:setKeys(1)
 end
 if f==640 then
  check(emu:read16(gNativeEnemyHp)==8,'restored recipe applies exactly its enhanced preview damage')
  check(emu:read16(gNativeSleights)==1 and emu:read16(gNativeActionLeft)==0,'restored recipe resolves once for one action')
  check(emu:read8(gNativeDeck+48)==3 and emu:read8(gNativeDeck+49)==2 and emu:read8(gNativeDeck+50)==2,'restored recipe applies normal first-card exhaustion')
  check(emu:read8(gNativeDeck+77)==0,'restored recipe clears stock after play')
  out:close()
 end
end)
