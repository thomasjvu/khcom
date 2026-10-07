local f=0
local out=io.open('@OUTPUT@/assembly.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local origin
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  check(emu:read16(gNativeAssembly)==1,'fresh round enters setup')
  check(emu:read32(sAssemblyTiles)~=0 and emu:read32(sAssemblyPalette)~=0,'original Donald character card allocates within palette budget')
  check(emu:read32(sCardTiles+4)==0 and emu:read32(sCardPalettes+4)==0,'setup borrows Fire artwork budget')
  local p=emu:read32(gFieldState);origin=emu:read32(p+0x18)
  emu:screenshot('@OUTPUT@/setup.png');emu:setKeys(64)
 end
 if f==184 then emu:setKeys(0) end
 if f==210 then
  check(emu:read32(emu:read32(gFieldState)+0x18)==origin,'setup does not move selected hero')
  check(emu:read16(gNativeGuard)==0 and emu:read16(gNativeActionLeft)==1,'setup preserves combat action and Guard')
  emu:setKeys(256)
 end
 if f==214 then emu:setKeys(0) end
 if f==240 then emu:setKeys(1) end
 if f==244 then emu:setKeys(0) end
 if f==280 then
  check(emu:read16(gNativeAssembly)==0 and emu:read16(gNativeParty)==1,'confirm starts round controlling Donald')
  check(emu:read8(gNativeRoster+14)==1,'roster enters battle phase')
  check(emu:read32(sAssemblyTiles)==0 and emu:read32(sAssemblyPalette)==0,'confirmation releases setup artwork')
  check(emu:read32(sCardTiles+4)~=0 and emu:read32(sCardPalettes+4)~=0,'confirmation reconstructs Fire artwork')
  check(emu:read16(gNativeActionLeft)==1,'setup confirmation does not also attack')
  emu:screenshot('@OUTPUT@/deployed.png');out:close()
 end
end)
