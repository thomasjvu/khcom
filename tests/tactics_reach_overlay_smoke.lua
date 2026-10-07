local f=0
local out=io.open('@OUTPUT@/reach.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local originalCount
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(1) end
 if f==184 then emu:setKeys(0) end
 if f==210 then emu:setKeys(528) end
 if f==214 then emu:setKeys(0) end
 if f==260 then
  originalCount=emu:read16(gNativeReachCount)
  check(emu:read16(gNativePreview)==1,'native movement preview opens')
  check(emu:read32(sReachTiles)~=0,'terrain diamond tile allocates')
  check(originalCount>0 and originalCount<=48,'overlay exposes destinations within three movement points')
  local count=0;local bounded=true
  for i=0,80 do local c=emu:read8(gNativeReachCost+i)
   if c~=255 and c>0 then count=count+1;bounded=bounded and c<=3 end
  end
  check(count==originalCount and bounded,'rendered reach mask respects exact movement budget')
  local c=emu:read8(gNativeReachCost+41);local selected=emu:read16(gNativeRouteCost)
  check((c==255 and selected==65535) or c==selected,'cursor route agrees with reach mask')
  emu:screenshot('@OUTPUT@/reach.png');emu:write16(gNativeMoveLeft,1)
 end
 if f==310 then
  check(emu:read16(gNativeReachCount)<=8 and emu:read16(gNativeReachCount)<=originalCount,'one-point budget reduces available destinations')
  emu:write16(gNativeMoveLeft,0)
 end
 if f==360 then
  check(emu:read16(gNativeReachCount)==0,'spent movement removes all available destinations')
  emu:setKeys(2)
 end
 if f==364 then emu:setKeys(0) end
 if f==400 then check(emu:read16(gNativePreview)==0,'B closes movement overlay');out:close() end
end)
