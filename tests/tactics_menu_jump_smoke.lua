-- Input-only command Move to Jump confirmation and native jump execution.
local f=0
local startX=0
local predictedX,predictedY,predictedZ
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==240 then emu:setKeys(4) end
 if f==280 or f==520 then emu:setKeys(1) end
 if f==320 or f==440 then emu:setKeys(256) end
 if f==360 or f==480 then emu:setKeys(16) end
 if f==400 then emu:setKeys(2) end
 if f==184 or f==244 or f==284 or f==324 or f==364 or f==404 or f==444 or f==484 or f==524 then emu:setKeys(0) end
 if f==360 then
  check(emu:read16(gNativeMenu)==6 and emu:read16(gNativePreview)==0,'Move opens separate Jump confirmation')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'opening Jump spends nothing')
 end
 if f==400 then
  check(emu:read16(gNativeDirection)==16,'D-pad selects moving jump direction')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'direction choice spends nothing')
  emu:screenshot('@OUTPUT@/jump.png')
 end
 if f==440 then check(emu:read16(gNativePreview)==1 and emu:read16(gNativeMenu)==0,'cancel returns to Move preview') end
 if f==520 then startX=emu:read32(emu:read32(gFieldState)+0x18) end
 if f==560 then
  check(emu:read16(gNativeMenu)==0 and emu:read16(gNativeBusy)==2,'confirmation starts native jump')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'moving jump spends one movement and action')
 end
 if f==520 then
  check(emu:read16(gNativeJumpPrediction)==1,'terrain predictor resolves the clear moving jump')
  predictedX=emu:read32(gNativeJumpLanding);predictedY=emu:read32(gNativeJumpLanding+4);predictedZ=emu:read32(gNativeJumpLanding+8)
 end
 if f==800 then
  local field=emu:read32(gFieldState)
  check(emu:read32(field+0x18)==predictedX and emu:read32(field+0x1c)==predictedY and emu:read32(field+0x20)==predictedZ,'native landing agrees exactly with pre-commit terrain prediction')
  check(emu:read32(emu:read32(gFieldState)+0x18)>startX and emu:read16(gNativeBusy)==0,'native moving jump travels and settles');emu:screenshot('@OUTPUT@/landed.png')
 end
 if f==840 then emu:setKeys(4) end
 if f==880 or f==1000 then emu:setKeys(1) end
 if f==920 then emu:setKeys(256) end
 if f==960 then emu:setKeys(16) end
 if f==844 or f==884 or f==924 or f==964 or f==1004 then emu:setKeys(0) end
 if f==1040 then
  check(emu:read16(gNativeMenu)==6 and emu:read16(gNativeBusy)==0,'spent action blocks Jump confirmation')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'blocked Jump consumes no movement');out:close()
 end
end)
