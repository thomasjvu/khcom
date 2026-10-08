-- Explicit discarded hand; all menu navigation and reload use native input.
local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==184 or f==244 or f==284 or f==324 or f==364 or f==404 or f==444 or f==484 or f==524 or f==604 or f==644 or f==684 or f==724 then emu:setKeys(0) end
 if f==240 then
  for i=0,11 do emu:write8(gNativeDeck+i,0);emu:write8(gNativeDeck+24+i,6);emu:write8(gNativeDeck+48+i,2) end
  emu:write8(gNativeDeck+73,0);emu:write8(gNativeDeck+77,0);emu:setKeys(4)
 end
 if f==280 or f==320 or f==400 or f==480 or f==680 then emu:setKeys(128) end
 if f==360 or f==520 or f==640 or f==720 then emu:setKeys(1) end
 if f==440 then
  check(emu:read16(gNativeMenu)==5,'Skills opens Reload confirmation')
  check(emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+48)==2,'opening Reload spends nothing')
  emu:screenshot('@OUTPUT@/reload.png');emu:setKeys(2)
 end
 if f==480 then check(emu:read16(gNativeMenu)==2 and emu:read8(gNativeDeck+48)==2,'cancel returns to Skills with discard intact') end
 if f==560 then
  check(emu:read16(gNativeMenu)==0,'confirmed reload closes menu')
  check(emu:read16(gNativeActionLeft)==0 and emu:read16(gNativeMoveLeft)==3,'reload costs one action and no movement')
  local hand=0;local draw=0
  for i=0,11 do local pile=emu:read8(gNativeDeck+48+i);if pile==1 then hand=hand+1 elseif pile==0 then draw=draw+1 end end
  check(hand==5 and draw==7,'reload returns discard and draws five cards')
 end
 if f==600 then
  for i=0,11 do emu:write8(gNativeDeck+48+i,2) end
  emu:setKeys(4)
 end
 if f==720 then check(emu:read16(gNativeMenu)==5,'spent hero can inspect Reload cost') end
 if f==760 then
  check(emu:read16(gNativeMenu)==5 and emu:read16(gNativeActionLeft)==0,'spent action blocks Reload confirmation')
  check(emu:read8(gNativeDeck+48)==2,'blocked reload preserves discarded cards');out:close()
 end
end)
