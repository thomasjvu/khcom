-- mGBA input replay plus deliberate SRAM corruption to verify slot fallback.
-- Run on kh_tactics.gba; evidence directory is supplied by the launcher.
local root=TACTICS_EVIDENCE or 'build/tactics-us/save-smoke'
local events={[70]=1,[82]=0,[110]=1,[122]=0,[150]=4,[162]=0,[190]=1,[202]=0,
 [230]=2,[242]=0,[270]=16,[282]=0,[310]=1,[322]=0,[350]=4,[362]=0,
 [390]=1,[402]=0,[480]=2,[492]=0,[610]=2,[622]=0}
local n=0
local first,second
local failed=false
local function log(s)
 local f=io.open(root..'/result.txt','a');f:write(s..'\n');f:close()
end
local function check(state,label)
 if emu:readRange(0x0203e000,254)~=state then
  failed=true;log('FAIL '..label);emu:screenshot(root..'/failure.png')
 else log('PASS '..label) end
end
callbacks:add('frame',function()
 n=n+1
 if events[n]~=nil then emu:setKeys(events[n]) end
 if n==145 then first=emu:readRange(0x0203e000,254) end
 if n==260 then check(first,'suspend and resume');emu:screenshot(root..'/resume.png') end
 if n==345 then second=emu:readRange(0x0203e000,254) end
 if n==430 then emu:reset() end
 if n==520 then check(second,'reset and resume newest slot') end
 if n==550 then
  local old=emu:read8(0x0E0003FF)
  emu:write8(0x0E0003FF,(old+1)%256)
  emu:reset()
 end
 if n==650 then check(first,'corrupt newest slot falls back');emu:screenshot(root..'/fallback.png') end
 if n==700 then log(failed and 'FAILED' or 'ALL SAVE CHECKS PASSED') end
end)
