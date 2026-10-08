local f=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s)out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush()end
local function matches(address,expected)
 for i,value in ipairs(expected)do if emu:read8(address+i-1)~=value then return false end end
 return true
end
callbacks:add('frame',function()
 f=f+1
 if f==360 then
  check(matches(gNativeRoster,expectedRoster),'native legacy resume preserves all six hero semantic records')
  check(matches(gNativeDeck,expectedDeck),'native legacy resume preserves every deck byte')
  check(matches(gNativePartyHealth,expectedHealth) and emu:read16(gNativeParty)==expectedParty,'native legacy resume preserves party health and control')
 end
 if f==400 then emu:setKeys(12)end
 if f==404 then emu:setKeys(0)end
 if f==440 then
  local slot=emu:read16(sSaveSlot)
  check(emu:read16(gNativeSaveNotice)==1 and emu:read8(0x0e000004+slot*1024)==13,'native save upgrades legacy SRAM to format 13')
 end
 if f==460 then emu:reset()end
 if f==800 then
  check(matches(gNativeRoster,expectedRoster),'upgraded native reboot preserves all hero records')
  check(matches(gNativeDeck,expectedDeck),'upgraded native reboot preserves entire deck')
  check(matches(gNativePartyHealth,expectedHealth) and emu:read16(gNativeParty)==expectedParty,'upgraded native reboot preserves health and control')
  out:close()
 end
end)
