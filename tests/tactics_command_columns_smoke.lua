-- Input-only compact command navigation.
local f=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s)out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush()end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8)end
 if f==184 or f==244 or f==284 or f==324 or f==364 or f==404 or f==444 or f==484 or f==524 then emu:setKeys(0)end
 if f==240 then emu:setKeys(4)end
 if f==280 then check(emu:read16(gNativeMenu)==1 and emu:read16(gNativeMenuChoice)==0,'open Commands at Move');emu:setKeys(16)end
 if f==320 then check(emu:read16(gNativeMenuChoice)==3,'Right selects Party on same row');emu:setKeys(32)end
 if f==360 then check(emu:read16(gNativeMenuChoice)==0,'Left returns to Move');emu:setKeys(128)end
 if f==400 then check(emu:read16(gNativeMenuChoice)==1,'Down selects Attack');emu:setKeys(16)end
 if f==440 then check(emu:read16(gNativeMenuChoice)==4,'Right selects End Turn on same row');emu:setKeys(32)end
 if f==480 then check(emu:read16(gNativeMenuChoice)==1,'Left returns to Attack');emu:setKeys(2)end
 if f==520 then check(emu:read16(gNativeMenu)==0,'cancel closes Commands');emu:setKeys(4)end
 if f==560 then check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'column navigation preserves turn budgets');out:close()end
end)
