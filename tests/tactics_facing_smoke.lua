-- Input-only facing commands after native round assembly.
local frame=0
local out=io.open('@OUTPUT@/facing.txt','w')
local directions={64,80,16,144,128,160,32,96}
local angles={0,45,64,83,128,173,192,211}
local initial,selected
local function check(ok,message) out:write((ok and 'PASS ' or 'FAIL ')..message..'\n');out:flush() end
callbacks:add('frame',function()
 frame=frame+1
 if frame==180 then emu:setKeys((testParty or 0)>0 and 256 or 1) end
 if frame==184 then emu:setKeys(0) end
 if frame==190 and testParty==2 then emu:setKeys(256) end
 if frame==194 then emu:setKeys(0) end
 if frame==200 and (testParty or 0)>0 then emu:setKeys(1) end
 if frame==204 then emu:setKeys(0) end
 if frame==210 then
  check(emu:read16(gNativeParty)==(testParty or 0),'assembly selects the requested controllable hero')
  local p=emu:read32(gFieldState)
  initial={emu:read32(p+0x18),emu:read32(p+0x1c),emu:read32(p+0x20)}
  selected=emu:read8(gNativeDeck+73)
 end
 if frame>=220 and frame<860 then
  local slot=math.floor((frame-220)/80)+1;local phase=(frame-220)%80
  if phase==0 then emu:setKeys(256+directions[slot]) end
  if phase==4 then emu:setKeys(0) end
  if phase==40 then
   local p=emu:read32(gFieldState)
   check(emu:read8(p+0x2c)==angles[slot],'native facing matches direction '..slot)
   check(emu:read32(p+0x18)==initial[1] and emu:read32(p+0x1c)==initial[2] and emu:read32(p+0x20)==initial[3],'facing preserves exact position '..slot)
   check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1 and emu:read16(gNativeBusy)==0,'facing preserves turn budgets '..slot)
   check(emu:read8(gNativeDeck+73)==selected,'facing does not cycle selected card '..slot)
  end
 end
 if frame==870 then emu:screenshot('@OUTPUT@/facing.png');out:close() end
end)
