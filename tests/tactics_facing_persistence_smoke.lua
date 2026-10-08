local frame=0
local out=io.open('@OUTPUT@/facing-save.txt','w')
local function check(ok,message) out:write((ok and 'PASS ' or 'FAIL ')..message..'\n');out:flush() end
local function facing(party,angle)
 local p=emu:read32(gFieldState)
 return emu:read16(gNativeParty)==party and emu:read8(p+0x2c)==angle and
  emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1
end
callbacks:add('frame',function()
 frame=frame+1
 local keys={[180]=1,[220]=272,[270]=4,[310]=384,[350]=4,[390]=288,[430]=4,[500]=4,[570]=4,[640]=12,[1060]=4,[1130]=4,[1200]=4}
 if keys[frame] then emu:setKeys(keys[frame]) end
 if keys[frame-4] then emu:setKeys(0) end
 if frame==460 then check(facing(0,64),'switching back restores Sora facing and budgets') end
 if frame==530 then check(facing(1,128),'switching back restores Donald facing and budgets') end
 if frame==600 then check(facing(2,192),'switching back restores Goofy facing and budgets') end
 if frame==690 then
  check(emu:read16(gNativeSaveNotice)==1,'per-hero facing suspend succeeds');emu:reset()
 end
 if frame==1020 then check(facing(2,192),'resume restores active Goofy facing and budgets') end
 if frame==1090 then check(facing(0,64),'resume preserves Sora facing independently') end
 if frame==1160 then check(facing(1,128),'resume preserves Donald facing independently') end
 if frame==1230 then check(facing(2,192),'resume preserves Goofy facing independently');out:close() end
end)
