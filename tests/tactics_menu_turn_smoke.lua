-- Input-only menu turn confirmation, cancellation and enemy phase.
local f=0
local turn=0
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==240 then emu:setKeys(4) end
 if f==280 or f==320 then emu:setKeys(64) end
 if f==360 or f==440 or f==480 then emu:setKeys(1) end
 if f==400 then emu:setKeys(2) end
 if f==184 or f==244 or f==284 or f==324 or f==364 or f==404 or f==444 or f==484 then emu:setKeys(0) end
 if f==400 then
  turn=emu:read16(gNativeTurn)
  check(emu:read16(gNativeMenu)==7,'End Turn opens party confirmation')
  check(emu:read16(gNativeEnemyFrames)==0,'confirmation does not start enemy phase')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'confirmation preserves current resources')
  emu:screenshot('@OUTPUT@/turn.png')
 end
 if f==440 then
  check(emu:read16(gNativeMenu)==1 and emu:read16(gNativeTurn)==turn,'cancel returns to Commands without advancing turn')
 end
 if f==480 then check(emu:read16(gNativeMenu)==7,'reselecting End Turn requires confirmation again') end
 if f==490 then
  check(emu:read16(gNativeMenu)==0 and emu:read16(gNativeEnemyFrames)>0,'confirm starts native enemy phase')
 end
 if f==600 then
  check(emu:read16(gNativeTurn)==turn+1 and emu:read16(gNativeEnemyFrames)==0,'exactly one enemy phase completes')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'new party turn refreshes resources');out:close()
 end
end)
