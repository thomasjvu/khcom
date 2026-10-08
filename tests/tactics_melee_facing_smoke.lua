-- Disclosed card/enemy setup; facing is selected solely through native input.
local dirs={64,80,16,144,128,160,32,96}
local angles={0,45,64,83,128,173,192,211}
callbacks:add('frame',function()
 f=f+1
 if testRally and f==100 then emu:setKeys(256) end
 if testRally and f==120 then emu:setKeys(128) end
 if f==180 then emu:setKeys(8) end
 if f==240 then emu:setKeys(4) end
 if f==280 then emu:setKeys(128) end
 if f==320 then emu:setKeys(1) end
 if f==104 or f==124 or f==184 or f==244 or f==284 or f==324 then emu:setKeys(0) end
 if f==380 then setup(5,0) end
 if f==400 then
  check(emu:read16(gNativeMenu)==4,'native sword confirmation opens')
  if testRally then check(emu:read16(gNativeParty)==1 and emu:read8(gNativeRoster+2)==4,'native assembly controls Rally') end
 end
 for i=1,8 do
  local frame=420+(i-1)*24
  if f==frame then emu:setKeys(dirs[i]) end
  if f==frame+4 then emu:setKeys(0) end
  if f==frame+12 then
   local field=emu:read32(gFieldState)
   check(emu:read8(field+0x2c)==angles[i],'native sword facing direction '..dirs[i]..' angle '..angles[i])
   check(emu:read16(gNativeMenu)==4 and emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'aim preserves menu and resources '..dirs[i])
  end
 end
 if f==620 then emu:screenshot('@OUTPUT@/diagonal.png');out:close() end
end)
