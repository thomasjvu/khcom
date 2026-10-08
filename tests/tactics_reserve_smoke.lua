-- Explicit KO fixture followed exclusively by native assembly button input.
local frame=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(ok,name) out:write((ok and 'PASS ' or 'FAIL ')..name..'\n');out:flush() end
callbacks:add('frame',function()
 frame=frame+1
 if frame==180 then
  check(emu:read16(gNativeAssembly)==1,'fixture starts in party assembly')
  emu:write8(gNativePartyHealth+1,0);emu:write8(gNativePartyHealth+2,0)
  emu:write8(gNativeRoster+20,0);emu:write8(gNativeRoster+21,0)
  emu:setKeys(16)
 end
 if frame==184 or frame==224 or frame==264 or frame==304 then emu:setKeys(0) end
 if frame==220 then emu:setKeys(128) end
 if frame==300 then
  check(emu:read8(gNativeRoster+2)==4,'native hero cycling deploys reserve Rally')
  check(emu:read8(gNativePartyHealth+1)==64,'reserve carries her actual health into deployed slot')
  check(emu:read8(gNativeRoster+20)==0 and emu:read8(gNativeRoster+21)==0,'cycling does not revive knocked-out Donald or Goofy')
  check(emu:read8(gNativePartyHealth+2)==0,'swapped knockout remains knocked out')
  emu:screenshot('@OUTPUT@/reserve-selected.png');emu:setKeys(8)
 end
 if frame==360 then
  check(emu:read16(gNativeAssembly)==0 and emu:read16(gNativeParty)==1,'deploy takes control of healthy Rally')
  check(emu:read8(gNativePartyHealth)==80 and emu:read8(gNativePartyHealth+1)==64 and emu:read8(gNativePartyHealth+2)==0,'deployment preserves per-hero health')
  out:close()
 end
end)
