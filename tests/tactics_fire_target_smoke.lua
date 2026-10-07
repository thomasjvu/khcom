-- Explicit placement/HP fixture; target selection and damage use native input.
local f=0
local out=io.open('@OUTPUT@/fire-target.txt','w')
local enemies={}
local ground=0
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local p=emu:read32(gFieldState);ground=emu:read32(p+0x20)
  for i=0,5 do
   local task=emu:read32(sEnemyTasks+i*4)
   if task~=0 then
    local w=emu:read32(task+4);enemies[i]=w
    emu:write32(w+8,emu:read32(p+0x18)+(i<2 and (i+1)*4096 or 65536))
    emu:write32(w+12,emu:read32(p+0x1c));emu:write32(w+16,ground);emu:write32(w+20,ground)
    emu:write16(gNativeEnemyHp+i*2,30)
   end
  end
  check(enemies[0]~=nil and enemies[1]~=nil,'two original enemy actors available for targeting fixture')
  emu:write8(gNativeDeck+73,1);emu:write8(gNativeDeck+25,6)
 end
 if f==200 then
  check(emu:read16(gNativeFireTarget)==0,'Fire defaults to the nearer eligible enemy')
  emu:setKeys(258)
 end
 if f==204 or f==244 or f==284 or f==304 then emu:setKeys(0) end
 if f==220 then
  check(emu:read16(gNativeFireChoice)==1 and emu:read16(gNativeFireTarget)==1,'R B chooses the farther eligible enemy')
  check(emu:read16(gNativeFireDamage)==12,'selected target previews authoritative damage')
  if enemies[1] then emu:write32(enemies[1]+16,ground+8192) end
 end
 if f==240 then
  check(emu:read16(gNativeFireTarget)==0 and emu:read16(gNativeFireChoice)==65535,'height-invalid choice falls back to the eligible enemy')
  if enemies[1] then emu:write32(enemies[1]+16,ground) end
  emu:setKeys(258)
 end
 if f==260 then
  check(emu:read16(gNativeFireTarget)==1,'restored eligible enemy can be selected again')
  if enemies[1] then emu:write32(enemies[1]+8,emu:read32(emu:read32(gFieldState)+0x18)+65536) end
 end
 if f==280 then
  check(emu:read16(gNativeFireTarget)==0,'out-of-range choice falls back safely')
  if enemies[1] then emu:write32(enemies[1]+8,emu:read32(emu:read32(gFieldState)+0x18)+8192) end
  emu:setKeys(258)
 end
 if f==300 then
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'target cycling preserves movement and action budgets')
  check(emu:read8(gNativeDeck+73)==1 and emu:read8(gNativeDeck+49)==1,'target cycling does not change or consume selected card')
  check(emu:read16(gNativeFireTarget)==1,'final preview points to chosen enemy before attack')
  emu:screenshot('@OUTPUT@/selected.png');emu:setKeys(1)
 end
 if f==330 then
  check(emu:read16(gNativeEnemyHp)==30,'Fire leaves the unselected nearer enemy unharmed')
  check(emu:read16(gNativeEnemyHp+2)==18,'Fire damages the selected farther enemy by previewed amount')
  check(emu:read16(gNativeActionLeft)==0 and emu:read8(gNativeDeck+49)==2,'selected Fire spends one action and discards one card')
  out:close()
 end
end)
