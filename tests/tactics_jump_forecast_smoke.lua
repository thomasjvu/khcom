-- Input-only live terrain prediction; unresolved means no guaranteed marker.
local f=0
local output=assert(io.open('@OUTPUT@/checks.txt','w'))
local resolved,x,y,z=0,0,0,0
local function check(ok,message) output:write((ok and 'PASS ' or 'FAIL ')..message..'\n');output:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==240 then emu:setKeys(4) end
 if f==280 or f==440 then emu:setKeys(1) end
 if f==320 then emu:setKeys(256) end
 if f==360 then emu:setKeys(@DIRECTION@) end
 if f==184 or f==244 or f==284 or f==324 or f==364 or f==444 then emu:setKeys(0) end
 if f==400 then
  resolved=emu:read16(gNativeJumpPrediction)
  x=emu:read32(gNativeJumpLanding);y=emu:read32(gNativeJumpLanding+4);z=emu:read32(gNativeJumpLanding+8)
  check(emu:read16(gNativeMenu)==6 and emu:read16(gNativeDirection)==@DIRECTION@,'native menu retains chosen direction')
  check(resolved==1 or resolved==2,'terrain reports resolved or explicit unresolved route')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'prediction preserves movement and action')
  output:write('PREDICTION '..resolved..' '..x..','..y..','..z..'\n');output:flush()
 end
 if f==800 then
  local field=emu:read32(gFieldState)
  if resolved==1 then
   check(emu:read16(gNativeBusy)==0 and emu:read32(field+0x18)==x and emu:read32(field+0x1c)==y and emu:read32(field+0x20)==z,'actual native landing matches resolved forecast exactly')
  else check(resolved==2,'unresolved terrain withheld guaranteed landing') end
  output:write('ACTUAL '..emu:read32(field+0x18)..','..emu:read32(field+0x1c)..','..emu:read32(field+0x20)..'\n');output:close()
 end
end)
