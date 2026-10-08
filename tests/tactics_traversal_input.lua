-- Hold each tested press until the native loop samples it and finishes its
-- update. Emulator video frames can outnumber native updates during search.
local nativeInput=nil
local function pressNative(keys)
 local release=(emu:read16(sRawKeys)&keys)~=0
 nativeInput={keys=release and 0 or keys,queued=release and keys or nil,sampled=nil,started=f}
 emu:setKeys(nativeInput.keys)
end
local function waitNativeInput()
 if not nativeInput then return false end
 local counter=emu:read32(gFrameCounter)
 if nativeInput.sampled and counter~=nativeInput.sampled then
  if nativeInput.queued then
   nativeInput.keys=nativeInput.queued;nativeInput.queued=nil;nativeInput.sampled=nil
  else nativeInput=nil;return false end
 end
 if nativeInput.sampled==nil and emu:read16(sRawKeys)==nativeInput.keys then
  nativeInput.sampled=counter
 end
 if f-nativeInput.started>360 then error('native input acknowledgement timed out') end
 emu:setKeys(nativeInput.keys)
 return true
end
