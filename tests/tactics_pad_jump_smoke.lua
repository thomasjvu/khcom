-- Saved original pad position; every gameplay action uses native input.
local f=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local x,y,z=0,0,0
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f>=180 and f<=212 and f%8==4 and (emu:read16(gNativeAssembly)~=0 or emu:read16(gNativeProgressReward)~=0) then emu:setKeys(1) end
 if f>=184 and f<=216 and f%8==0 then emu:setKeys(0) end
 if f==240 then emu:setKeys(4) end
 if f==280 or f==440 then emu:setKeys(1) end
 if f==320 then emu:setKeys(256) end
 if f==244 or f==284 or f==324 or f==444 then emu:setKeys(0) end
 if f==400 then
  check(emu:read16(gNativeMenu)==6,'native Jump command opens on original launcher')
  check(emu:read32(emu:read32(gMapRoomState)+0x1c)>0,'original collider warms native launcher height')
  check(emu:read16(gNativeJumpPrediction)==1,'launcher has resolved landing forecast')
  check(hudText(4,0,'LANDING DIAMOND ON MAP'),'launcher forecast renders resolved status')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'launcher forecast preserves resources')
  x=emu:read32(gNativeJumpLanding);y=emu:read32(gNativeJumpLanding+4);z=emu:read32(gNativeJumpLanding+8)
  out:write('PREDICTION '..x..','..y..','..z..'\n');out:flush()
  emu:screenshot('@OUTPUT@/preview.png')
 end
 if f==1000 then
  local field=emu:read32(gFieldState)
  check(emu:read16(gNativeBusy)==0 and emu:read32(field+0x18)==x and emu:read32(field+0x1c)==y and emu:read32(field+0x20)==z,'actual native launcher landing matches prediction exactly')
  check(emu:read32(field+0x20)==targetHeight,'native launch reaches original upper surface')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==0,'standing launcher costs only one action')
  out:write('ACTUAL '..emu:read32(field+0x18)..','..emu:read32(field+0x1c)..','..emu:read32(field+0x20)..'\n');out:close()
  emu:screenshot('@OUTPUT@/landed.png')
 end
end)
