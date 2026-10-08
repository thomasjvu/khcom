local f=0
local pillar,x,y,z,predX,predY,predZ
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(ok,msg) out:write((ok and 'PASS ' or 'FAIL ')..msg..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local node=emu:read32(sColliderPoolObstacle+8)
  while node~=0 do
   local c=emu:read32(node)
   if emu:read32(c+16)==8192 then pillar=c;break end
   node=emu:read32(node+8)
  end
  check(pillar~=nil,'original generated pillar available in disclosed saved-room fixture')
  if not pillar then out:close();return end
  x=emu:read32(pillar+4)+9216;y=emu:read32(pillar+8)//2;z=emu:read32(pillar+12)
  local field=emu:read32(gFieldState)
  for _,a in ipairs({field+0x18,sPartyPos}) do
   emu:write32(a,x);emu:write32(a+4,y);emu:write32(a+8,z);emu:write32(a+12,z)
  end
  emu:write16(gNativeMoveLeft,3);emu:write16(gNativeActionLeft,1)
  emu:setKeys(4)
 end
 if f==184 or f==244 or f==284 or f==324 or f==364 then emu:setKeys(0) end
 if f==240 or f==360 then emu:setKeys(1) end
 if f==280 then emu:setKeys(256) end
 if f==320 then emu:setKeys(32) end
 if f==360 and pillar then
  check(emu:read16(gNativeMenu)==6,'native Jump menu opens beside pillar')
  check(emu:read16(gNativeJumpPrediction)==1,'prop-contact jump has resolved prediction')
  predX=emu:read32(gNativeJumpLanding);predY=emu:read32(gNativeJumpLanding+4);predZ=emu:read32(gNativeJumpLanding+8)
  out:write('PREDICT '..predX..','..predY..','..predZ..'\n');out:flush()
 end
 if f==650 and pillar then
  local a=emu:read32(gFieldState)
  check(emu:read16(gNativeBusy)==0 and emu:read32(a+0x18)==predX and emu:read32(a+0x1c)==predY and emu:read32(a+0x20)==predZ,'native prop landing agrees exactly')
  check(emu:read32(a+0x20)==emu:read32(pillar+12)-emu:read32(pillar+20),'native jump lands on original pillar top')
  out:write('ACTUAL '..emu:read32(a+0x18)..','..emu:read32(a+0x1c)..','..emu:read32(a+0x20)..'\n');out:close()
 end
end)
