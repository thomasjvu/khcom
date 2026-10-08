-- Read-only oracle for the actual native controller; not a landing predictor.
local trace=assert(io.open('@OUTPUT@/jump-trajectory.csv','w'))
trace:write('video_frame,native_frame,x,y,z,ground,menu,busy,direction,move,action\n')
local traceFrame,lastNative=0,-1
local function signed32(v) return v>=0x80000000 and v-0x100000000 or v end
callbacks:add('frame',function()
 traceFrame=traceFrame+1
 if traceFrame>1040 then return end
 local frame=emu:read32(gFrameCounter)
 local field=emu:read32(gFieldState)
 if field~=0 and frame~=lastNative then
  lastNative=frame
  trace:write(string.format('%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n',traceFrame,frame,
   signed32(emu:read32(field+0x18)),signed32(emu:read32(field+0x1c)),
   signed32(emu:read32(field+0x20)),signed32(emu:read32(field+0x24)),
   emu:read16(gNativeMenu),emu:read16(gNativeBusy),emu:read16(gNativeDirection),
   emu:read16(gNativeMoveLeft),emu:read16(gNativeActionLeft)))
 end
 if traceFrame==1040 then trace:close() end
end)
