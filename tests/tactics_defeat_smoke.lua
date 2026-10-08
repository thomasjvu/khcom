-- Input-only regression: passing turns without defending must lead to defeat.
local root=TACTICS_EVIDENCE or 'build/tactics-us/defeat-smoke'
local n=0
local lostAt=nil
local events={[70]=1,[82]=0,[120]=1,[132]=0}
for i=0,14 do
 local frame=200+i*100
 events[frame]=8;events[frame+12]=0;events[frame+36]=1;events[frame+48]=0
end
local function log(s)
 local f=io.open(root..'/result.txt','a');f:write(s..'\n');f:close()
end
callbacks:add('frame',function()
 n=n+1
 if not lostAt and events[n]~=nil then emu:setKeys(events[n]) end
 if not lostAt and n>180 and emu:read8(0x0203e000+245)==3 then
  lostAt=n;emu:setKeys(0)
  log(emu:read8(0x0203e000+251)==0 and 'PASS defeat with zero HP' or 'FAIL defeat HP')
 end
 if lostAt and n==lostAt+20 then emu:screenshot(root..'/defeat.png') end
 if n==1800 and not lostAt then log('FAIL no defeat') end
end)
