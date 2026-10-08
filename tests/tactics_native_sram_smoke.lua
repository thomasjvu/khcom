-- Save inputs are native. Only the final newest-slot corruption is a fixture.
local f=0;local first,second
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,label)out:write((v and 'PASS ' or 'FAIL ')..label..'\n');out:flush()end
local function sramWord(address)
 -- SRAM is an eight-bit bus; assemble its bytes rather than using read32.
 local value=0
 for i=0,3 do value=value|(emu:read8(address+i)<<(i*8)) end
 return value
end
local function state()
 return emu:readRange(gNativeDeck,78)..emu:readRange(gNativeRoster,rosterBytes)..
  emu:readRange(gNativePartyHealth,3)..emu:readRange(sPartyPos,48)..
  emu:readRange(gNativeMoveLeft,2)..emu:readRange(gNativeActionLeft,2)
end
local function dump(name)
 local file=io.open('@OUTPUT@/'..name,'wb')
 for i=0,32767 do file:write(string.char(emu:read8(0x0e000000+i))) end
 file:close()
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==184 or f==244 or f==304 or f==344 then emu:setKeys(0) end
 if f==240 or f==340 then emu:setKeys(12) end
 if f==280 then
  check(emu:read16(gNativeSaveNotice)==1,'first suspend reports verified success')
  check(emu:read16(sSaveSlot)==0,'first suspend uses slot zero')
  check(sramWord(0x0e000000)==0x5346544b,'slot zero signature committed')
  check(sramWord(0x0e000008)==1,'first generation committed')
  first=state();dump('first-sram.bin')
 end
 if f==300 then emu:setKeys(256) end
 if f==380 then
  check(emu:read16(gNativeSaveNotice)==1,'second suspend reports verified success')
  check(emu:read16(sSaveSlot)==1,'second suspend alternates to slot one')
  check(sramWord(0x0e000408)==2,'second generation committed')
  second=state();check(first~=second,'second save records changed card selection')
  dump('second-sram.bin')
 end
 if f==400 then emu:reset() end
 if f==720 then
  check(state()==second,'reset restores newest complete party deck and resources')
  check(emu:read16(sSaveSlot)==1,'resume selects newest valid slot')
 end
 if f==740 then
  -- Deliberate corruption, not a claimed naturally occurring write failure.
  emu:write8(0x0e0007ff,(emu:read8(0x0e0007ff)+1)%256);emu:reset()
 end
 if f==1080 then
  check(state()==first,'corrupt newest slot restores older complete state')
  check(emu:read16(sSaveSlot)==0,'corrupt newest slot selects valid fallback')
  out:close()
 end
end)
