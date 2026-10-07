-- Explicit saved-room and approach fixture; facing/attack use native controls.
local f=0
local out=io.open('@OUTPUT@/jump.txt','w')
local node,collider,deckCount
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  node=emu:read32(sColliderPoolObstacle+8)
  while node~=0 do
   local c=emu:read32(node)
   if emu:read32(c+16)==8192 then collider=c;break end
   node=emu:read32(node+8)
  end
  check(collider~=nil,'recorded large Castle prop exists in generated room')
  if not collider then out:close();return end
  local p=emu:read32(gFieldState)
  local taskNode=emu:read32(p+0x80);local static=false
  while taskNode~=0 do
   local t=emu:read32(taskNode);local w=emu:read32(t+4)
   if emu:read32(t)==gTaskDescMapGmk00 and emu:read32(w+4)==emu:read32(collider+4) and
      emu:read32(w+8)==emu:read32(collider+8)//2 then static=true end
   taskNode=emu:read32(taskNode+8)
  end
  check(static,'large pillar uses original static prop task rather than breakable decoration')
  local x=emu:read32(collider+4)+9216
  local y=emu:read32(collider+8)//2
  local z=emu:read32(collider+12)
  for _,a in ipairs({p+0x18,sPartyPos}) do
   emu:write32(a,x);emu:write32(a+4,y);emu:write32(a+8,z);emu:write32(a+12,z)
  end
  -- Isolate one available Keyblade in a valid native starter deck.
  for i=0,11 do
   emu:write8(gNativeDeck+i,0);emu:write8(gNativeDeck+24+i,6)
   emu:write8(gNativeDeck+48+i,i<5 and 1 or 0)
  end
  emu:write8(gNativeDeck+73,0);emu:write8(gNativeDeck+77,0)
  emu:setKeys(288)
 end
 if f==184 then emu:setKeys(0) end
 if f==220 then
  check(emu:read8(emu:read32(gFieldState)+0x2c)==192,'native free facing points Sora toward pillar')
  check(emu:read16(gNativeActionLeft)==1,'facing retains attack action')
  emu:screenshot('@OUTPUT@/before.png');emu:setKeys(34)
 end
 if f==224 then emu:setKeys(0) end
 if f==320 then
  local p=emu:read32(gFieldState)
  local z=emu:read32(p+0x20)
  out:write('LANDING x='..emu:read32(p+0x18)..' y='..emu:read32(p+0x1c)..' z='..z..' ground='..emu:read32(p+0x24)..' busy='..emu:read16(gNativeBusy)..'\n');out:flush()
  check(z==emu:read32(collider+12)-emu:read32(collider+20),'native jump lands on the pillar top')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'pillar jump spends one movement and one action')
  emu:screenshot('@OUTPUT@/top.png');emu:setKeys(8)
 end
 if f==324 then emu:setKeys(0) end
 if f==440 or f==500 or f==560 then emu:setKeys(32) end
 if f==444 or f==504 or f==564 then emu:setKeys(0) end
 if f==680 then
  local p=emu:read32(gFieldState)
  out:write('FAR SIDE x='..emu:read32(p+0x18)..' y='..emu:read32(p+0x1c)..' z='..emu:read32(p+0x20)..' ground='..emu:read32(p+0x24)..' move='..emu:read16(gNativeMoveLeft)..' action='..emu:read16(gNativeActionLeft)..'\n');out:flush()
  check(emu:read32(p+0x18)<emu:read32(collider+4)-emu:read32(collider+16),'native movement crosses beyond the pillar footprint')
  check(emu:read32(p+0x20)==emu:read32(collider+12),'native descent returns to the supporting floor')
  emu:screenshot('@OUTPUT@/far-side.png');out:close()
 end
end)
