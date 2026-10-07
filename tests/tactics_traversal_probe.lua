-- Input-only bounded exploration. No RAM writes, teleporting, or forced exits.
local out=io.open('@OUTPUT@/traversal.txt','w')
local f=0
local nextFrame=180
local phase='scan'
local index=1
local dirs={16,32,64,128}
local best=nil
local visits={}
local commands=0
local done=false
local previousRoom=0
local function signed(v) if v>=2147483648 then return v-4294967296 end return v end
local function pos()
 local p=emu:read32(gFieldState)
 return signed(emu:read32(p+0x18)),signed(emu:read32(p+0x1c))+signed(emu:read32(p+0x20)),signed(emu:read32(p+0x20))
end
local function cell(x,y,z) return math.floor((x+2048)/4096)..':'..math.floor((y+1024)/2048)..':'..math.floor(z/2048) end
local function door()
 local p=emu:read32(gFieldState)
 local n=emu:read32(p+0x80)
 while n~=0 do
  local t=emu:read32(n)
  if emu:read32(t)==gTaskDescMapRnd then
   local child=emu:read32(emu:read32(t+4)+8)
   while child~=0 do
    local task=emu:read32(child)
    if emu:read32(task)==gTaskDescMapDoor then
     local w=emu:read32(task+4)
     local d=emu:read32(w)
     if emu:read8(d+7)==emu:read8(gMapFloorState+6)+1 then
      return signed(emu:read32(w+4)),signed(emu:read32(w+8))+signed(emu:read32(w+12)),signed(emu:read32(w+12))
     end
    end
    child=emu:read32(child+8)
   end
  end
  n=emu:read32(n+8)
 end
end
local function stairGoal(tx,ty,tz)
 local x,y,z=pos()
 if math.abs(z-tz)<2048 then return tx,ty,tz,false end
 local room=emu:read32(gMapRoomState)
 local cols=emu:read16(room+4);local rows=emu:read16(room+6)
 local cells=emu:read32(sMapCells)
 local best=nil
 for cy=0,rows-1 do
  for cx=0,cols-1 do
   local a=cells+(cy*cols+cx)*32
   local flags=emu:read16(a);local kind=emu:read8(a+2)
   local upper=signed(emu:read32(a+8));local lower=signed(emu:read32(a+12))
   if (flags&32)~=0 and (kind==4 or kind==6) and math.abs(lower-z)<2048 and math.abs(upper-tz)<math.abs(z-tz) then
    local sx=(cx*32+16)*256;local sy=(cy*16+14)*256
    local score=math.abs(sx-x)+2*math.abs(sy-y)+math.abs(upper-tz)
    if not best or score<best.score then best={x=sx,y=sy,score=score,key=64} end
   end
   if (flags&32)~=0 and (kind==3 or kind==5) and math.abs(upper-z)<2048 and math.abs(lower-tz)<math.abs(z-tz) then
    local sx=(cx*32+16)*256;local sy=(cy*16+2)*256
    local score=math.abs(sx-x)+2*math.abs(sy-y)+math.abs(lower-tz)
    if not best or score<best.score then best={x=sx,y=sy,score=score,key=128} end
   end
  end
 end
 if best then return best.x,best.y,z,best.key end
 return tx,ty,tz,false
end
local function finish(ok,why)
 emu:setKeys(0)
 local x,y,z=pos()
 local dx,dy,dz=door();out:write('DOOR '..tostring(dx)..','..tostring(dy)..','..tostring(dz)..'\n')
 out:write((ok and 'PASS ' or 'FAIL ')..why..' frames='..f..' commands='..commands..' position='..x..','..y..','..z..'\n')
 out:flush();out:close();done=true
 emu:screenshot('@OUTPUT@/final.png')
end
callbacks:add('frame',function()
 f=f+1
 if done then return end
 local room=emu:read8(gMapFloorState+6)
 if room>=goalRoom then finish(true,'walked from native spawn to room '..room);return end
 if room~=previousRoom then
  out:write('ROOM '..room..' frames='..f..'\n');out:flush()
  previousRoom=room;best=nil;visits={};index=1;phase='release';nextFrame=f+60
 end
 if f>12000 then finish(false,'bounded explorer did not reach room '..goalRoom);return end
 if f<nextFrame then return end
 emu:setKeys(0)
 if emu:read16(gNativeBusy)~=0 then nextFrame=f+8;return end
 if phase=='release' then phase='scan';nextFrame=f+8;return end
 if emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativePreview)==0 then
  emu:setKeys(8);phase='release';nextFrame=f+4;return
 end
 local dx,dy,dz=door()
 if not dx then finish(false,'forward door missing');return end
 if emu:read16(gNativeClimbing)~=0 then
  -- Original stairs ascend with Up; each command is budgeted by the ROM.
  local _,_,z=pos();emu:setKeys(dz>z and 128 or 64);commands=commands+1;phase='release';nextFrame=f+4;return
 end
 local x0,y0,z0=pos()
 if phase=='scan' and math.abs(x0-dx)<8192 and math.abs(y0-dy)<4096 and math.abs(z0-dz)<2048 then
  emu:setKeys((dx<x0 and 32 or 16)+(dy<y0 and 64 or 128));commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 local stair
 dx,dy,dz,stair=stairGoal(dx,dy,dz)
 local px,py=pos()
 if phase=='scan' and stair and math.abs(px-dx)<2048 and math.abs(py-dy)<4096 then
  emu:setKeys(stair);commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 if phase=='scan' then
  emu:setKeys(512+dirs[index]);phase='inspect';nextFrame=f+4;return
 end
 if phase=='inspect' then
  local cost=emu:read16(gNativeRouteCost)
  if emu:read16(gNativePreview)==1 and cost==1 then
   local x=emu:read16(sCursorX);if x>=32768 then x=x-65536 end
   local y=emu:read16(sCursorY);if y>=32768 then y=y-65536 end
   local a=sRoutePos+((y+4)*9+x+4)*16
   local px=signed(emu:read32(a));local z=signed(emu:read32(a+8))
   local py=signed(emu:read32(a+4))+z
   local score=math.abs(px-dx)+2*math.abs(py-dy)+math.abs(z-dz)+(visits[cell(px,py,z)] or 0)*16384
   if not best or score<best.score then best={dir=dirs[index],score=score,key=cell(px,py,z)} end
  end
  emu:setKeys(2);phase='cancel';nextFrame=f+4;return
 end
 if phase=='cancel' then
  index=index+1
  if index<=4 then phase='scan';nextFrame=f+4;return end
  index=1
  if best then emu:setKeys(512+best.dir);phase='commit';nextFrame=f+4
  else
   -- Walking into a stair or a door uses the original controller, not flat route edges.
   local x,y,z=pos();local key=dy<y and 64 or 128
   if math.abs(dx-x)>math.abs(dy-y)*2 then key=dx<x and 32 or 16 end
   emu:setKeys(key);commands=commands+1;phase='release';nextFrame=f+4
  end
  return
 end
 if phase=='commit' then phase='confirm';nextFrame=f+4;return end
 if phase=='confirm' then
  emu:setKeys(1);commands=commands+1
  local x,y,z=pos();local k=best.key;visits[k]=(visits[k] or 0)+1
  out:write('STEP '..commands..' '..x..' '..y..' '..z..'\n');out:flush()
  best=nil;phase='release';nextFrame=f+4
 end
end)
