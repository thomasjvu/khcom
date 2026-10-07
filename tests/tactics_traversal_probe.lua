-- Input-only bounded exploration. No RAM writes, teleporting, or forced exits.
local out=io.open('@OUTPUT@/traversal.txt','w')
local f=0
local nextFrame=180
local phase='scan'
local index=1
local dirs={16,32,64,128,80,96,144,160}
local best=nil
local planned=nil
local visits={}
local commands=0
local done=false
local previousRoom=0
local previousWorld=0
local victoryFrame=nil
local victoryHealth=nil
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
     local room=emu:read8(gMapFloorState+6)
     if emu:read8(d+7)==(room==7 and 253 or room+1) then
      return signed(emu:read32(w+4)),signed(emu:read32(w+8))+signed(emu:read32(w+12)),signed(emu:read32(w+12))
     end
    end
    child=emu:read32(child+8)
   end
  end
  n=emu:read32(n+8)
 end
end
-- @GEOMETRY@
-- @REGIONS@
local function stairGoal(tx,ty,tz)
 local goal=terrainGoal(tx,ty,tz)
 if goal then return goal.x,goal.y,goal.z,goal.key end
 local x,y,z=pos()
 -- Prop tops can share the height of a different map platform. Plan the
 -- next map connection from the underlying ground rather than the prop top.
 z=signed(emu:read32(emu:read32(gFieldState)+0x24))
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
   if (kind==4 or kind==6) and math.abs(lower-z)<2048 and math.abs(upper-tz)<math.abs(z-tz) then
    local sx=(cx*32+16)*256;local sy=(cy*16+14)*256
    local score=math.abs(sx-x)+2*math.abs(sy-y)+math.abs(upper-tz)
    if not best or score<best.score then best={x=sx,y=sy,score=score,key=(flags&32)~=0 and 64 or (kind==6 and 82 or 98)} end
   end
   if (kind==3 or kind==5) and math.abs(upper-z)<2048 and math.abs(lower-tz)<math.abs(z-tz) then
    local sx=(cx*32+16)*256;local sy=(cy*16+((flags&32)~=0 and 2 or -4))*256
    local score=math.abs(sx-x)+2*math.abs(sy-y)+math.abs(lower-tz)
    if not best or score<best.score then best={x=sx,y=sy,score=score,key=(flags&32)~=0 and 128 or (kind==5 and 160 or 144)} end
   end
  end
 end
 if best then return best.x,best.y,z,best.key end
 return tx,ty,tz,false
end
local function encounterGoal()
 if emu:read8(gMapFloorState+6)~=7 then return nil end
 local x,y,z=pos();local best=nil
 for i=0,5 do
  local t=emu:read32(sEnemyTasks+i*4)
  if t~=0 then
   local w=emu:read32(t+4);local ex=signed(emu:read32(w+8));local ez=signed(emu:read32(w+16));local ey=signed(emu:read32(w+12))+ez
   local score=math.abs(ex-x)+2*math.abs(ey-y)+math.abs(ez-z)
   if not best or score<best.score then best={x=ex,y=ey,z=ez,score=score} end
  end
 end
 if best then return best.x,best.y,best.z end
end
local function combatInput()
 local x,y,z=pos();local near=false
 for i=0,5 do
  local t=emu:read32(sEnemyTasks+i*4)
  if t~=0 then
   local w=emu:read32(t+4);local ez=signed(emu:read32(w+16))
   local ex=signed(emu:read32(w+8));local ey=signed(emu:read32(w+12))+ez
   if math.abs(ex-x)+math.abs(ey-y)<32768 and math.abs(ez-z)<=6144 then near=true end
  end
 end
 if not near or emu:read16(gNativeActionLeft)==0 then return nil end
 local hand={};local wanted=nil
 for i=0,emu:read8(gNativeDeck+72)-1 do
  if emu:read8(gNativeDeck+48+i)==1 then
   local kind=emu:read8(gNativeDeck+i);hand[#hand+1]=kind
   if kind==1 and not wanted then wanted=#hand-1 end
  end
 end
 if emu:read8(gNativePartyHealth)<60 then
  for i,kind in ipairs(hand) do if kind==2 then wanted=i-1;break end end
 end
 if #hand==0 then return 768 end
 if not wanted then
  -- Cycle remaining cards into discard so native draw can expose more Fire.
  wanted=0
 end
 if emu:read8(gNativeDeck+73)~=wanted then return 256 end
 return 1
end
local endingTurn=false
local returningToSora=false
local function turnKey()
 if returningToSora then
  if emu:read16(gNativeEnemyFrames)>0 then return 0 end
  if emu:read16(gNativeParty)~=0 then return 4 end
  returningToSora=false;return 0
 end
 local guard=nil;local slot=0
 for i=0,emu:read8(gNativeDeck+72)-1 do
  if emu:read8(gNativeDeck+48+i)==1 then
   if emu:read8(gNativeDeck+i)==3 and not guard then guard=slot end
   slot=slot+1
  end
 end
 local p=emu:read32(gFieldState);local task=emu:read32(emu:read32(p+0x94));local state=emu:read32(emu:read32(task+4)+0x94)
 local threat=emu:read16(gNativeThreats)+emu:read16(gNativeThreats+2)+emu:read16(gNativeThreats+4)
 local party=emu:read16(gNativeParty)
 local action=party==2 and emu:read16(gNativeActionLeft) or emu:read16(sPartyAction+4)
 if state==0 and threat>0 and guard and emu:read16(gNativeGuard)==0 and emu:read8(gNativePartyHealth+2)>0 and action>0 then
  if party~=2 then return 4 end
  if emu:read8(gNativeDeck+73)~=guard then return 256 end
  out:write('GOOFY GUARD frame='..f..' threat='..threat..'\n');out:flush();return 1
 end
 endingTurn=false;returningToSora=true;return 8
end
local function requestTurn()
 endingTurn=true;best=nil;index=1;emu:setKeys(turnKey());phase='release';nextFrame=f+4
end
local function finish(ok,why)
 emu:setKeys(0)
 local x,y,z=pos()
 local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6);local cells=emu:read32(sMapCells)
 for cy=0,math.min(rows,64)-1 do for cx=0,math.min(cols,32)-1 do
  local a=cells+(cy*cols+cx)*32
  if (emu:read16(a)&32)~=0 or (emu:read8(a+2)>=3 and emu:read8(a+2)<=6 and signed(emu:read32(a+8))~=-1048576 and signed(emu:read32(a+12))~=1048576) then out:write('EDGE '..cx..' '..cy..' '..emu:read8(a+2)..' '..signed(emu:read32(a+8))..' '..signed(emu:read32(a+12))..'\n') end
 end end
 local field=emu:read32(gFieldState);local task=emu:read32(emu:read32(field+0x94));local w=emu:read32(task+4)
 out:write('NATIVE busy='..emu:read16(gNativeBusy)..' flags='..string.format('%x',emu:read32(field+0x70))..' update='..string.format('%x',emu:read32(gCurrentModeUpdate))..' timer='..emu:read32(w+0x98)..' taskUpdate='..string.format('%x',emu:read32(task+0x20))..'\n')
 out:write('ACTOR state='..emu:read32(w+0x94)..' ground='..signed(emu:read32(field+0x24))..' angle='..emu:read8(field+0x2c)..' collision='..emu:read8(w+0x64)..' other='..emu:read32(w+0x6c)..'\n')
 for cy=13,21 do for cx=5,9 do local a=cells+(cy*cols+cx)*32;out:write('CELL '..cx..' '..cy..' '..emu:read8(a+2)..' '..signed(emu:read32(a+8))..' '..signed(emu:read32(a+12))..'\n') end end
 local platforms=emu:read32(sMapPlatforms)
 for i=0,11 do local p=platforms+i*24;out:write('PLATFORM '..i..' '..emu:read16(p)..' '..emu:read16(p+2)..' '..signed(emu:read32(p+4))..' '..emu:read8(p+8)..' '..emu:read16(p+10)..' '..emu:read16(p+12)..' '..emu:read8(p+14)..' '..signed(emu:read32(p+16))..' '..signed(emu:read32(p+20))..'\n') end
 local dx,dy,dz=door();out:write('DOOR '..tostring(dx)..','..tostring(dy)..','..tostring(dz)..'\n')
 out:write((ok and 'PASS ' or 'FAIL ')..why..' frames='..f..' kills='..emu:read16(gNativeKills)..' commands='..commands..' position='..x..','..y..','..z..'\n')
 out:flush();out:close();done=true
 emu:screenshot('@OUTPUT@/final.png')
end
local function replayFrame()
 f=f+1
 if done or f<180 then return end
 local room=emu:read8(gMapFloorState+6)
 local world=emu:read16(gNativeFloor)
 if emu:read32(gCurrentMode)~=sNativeMode or (emu:read32(gCurrentModeUpdate)&0xfffffffe)~=NativeUpdate then
  out:write('MODE current='..string.format('%x',emu:read32(gCurrentMode))..' update='..string.format('%x',emu:read32(gCurrentModeUpdate))..' pending='..string.format('%x',emu:read32(gPendingMode))..' busy='..emu:read16(gNativeBusy)..'\n')
  finish(false,'left native tactics mode');return
 end
 if goalWorlds==3 and world>=3 and emu:read16(gNativeResult)==2 then
  emu:setKeys(0)
  if not victoryFrame then
   victoryFrame=f;victoryHealth=emu:read8(gNativePartyHealth)
   out:write('VICTORY frame='..f..' hp='..victoryHealth..'\n');out:flush()
  end
  if f-victoryFrame>=120 then
   finish(world==3 and emu:read8(gNativePartyHealth)==victoryHealth,'completed three worlds; terminal floor and Sora HP remain stable for 120 frames')
  end
  return
 end
 if goalWorlds>0 and goalWorlds<3 and world>=goalWorlds then finish(true,'completed '..goalWorlds..' worlds through native input');return end
 if goalWorlds==0 and room>=goalRoom then finish(true,'walked from native spawn to room '..room);return end
 if world~=previousWorld then
  out:write('WORLD '..world..' frames='..f..'\n');out:flush()
  previousWorld=world;previousRoom=-1
 end
 if room~=previousRoom then
  out:write('ROOM '..room..' frames='..f..'\n');out:flush()
  previousRoom=room;terrainPlan=nil;best=nil;visits={};index=1;phase='release';nextFrame=f+60
 end
 if emu:read16(gNativeResult)~=0 then finish(false,'run ended before traversal goal');return end
 if f>goalFrames then finish(false,'bounded explorer did not reach '..(goalWorlds>0 and ('world '..goalWorlds) or ('room '..goalRoom)));return end
 if f<nextFrame then return end
 emu:setKeys(0)
 if emu:read16(gNativeBusy)~=0 then nextFrame=f+8;return end
 if phase=='release' then phase='scan';nextFrame=f+8;return end
 if endingTurn or returningToSora then
  emu:setKeys(turnKey());best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 if emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativePreview)==0 then
  requestTurn();return
 end
 local dx,dy,dz=door()
 if not dx then finish(false,'forward door missing');return end
 local ex,ey,ez=encounterGoal()
 if ex then
  dx,dy,dz=ex,ey,ez
  local x,y,z=pos()
  if phase=='scan' and index==1 and math.abs(x-ex)+math.abs(y-ey)<32768 and math.abs(z-ez)<=6144 and emu:read16(gNativeActionLeft)==0 then
   requestTurn();return
  end
 end
 if emu:read16(gNativeClimbing)~=0 then
  -- Original stairs ascend with Up; each command is budgeted by the ROM.
  local _,_,z=pos();emu:setKeys(dz>z and 128 or 64);commands=commands+1;phase='release';nextFrame=f+4;return
 end
 if phase=='scan' and index==1 then
  local x,y,z=pos()
  local key=combatInput()
  if key then emu:setKeys(key);phase='release';nextFrame=f+4;return end
  if (visits[cell(x,y,z)] or 0)>=3 and emu:read16(gNativeActionLeft)==0 then
   requestTurn();return
  end
  if (visits[cell(x,y,z)] or 0)>=3 and emu:read16(gNativeActionLeft)>0 and emu:read16(gNativeMoveLeft)>0 then
   local tx,ty=stairGoal(dx,dy,dz)
   local key=(tx<x and 32 or 16)+(ty<y and 64 or 128)
   out:write('JUMP '..f..' '..x..' '..y..' '..z..'\n');out:flush()
   emu:setKeys(key+2);commands=commands+1;visits[cell(x,y,z)]=0;phase='release';nextFrame=f+4;return
  end
 end
 local x0,y0,z0=pos()
 if not ex and phase=='scan' and math.abs(x0-dx)<8192 and math.abs(y0-dy)<4096 and math.abs(z0-dz)<2048 then
  emu:setKeys((dx<x0 and 32 or 16)+(dy<y0 and 64 or 128));commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 local stair
 dx,dy,dz,stair=stairGoal(dx,dy,dz)
 local px,py=pos()
 if phase=='scan' and stair and math.abs(px-dx)<(stair~=64 and stair~=128 and 4096 or 2048) and math.abs(py-dy)<(stair~=64 and stair~=128 and 4096 or 4096) then
  if (stair&2)~=0 and emu:read16(gNativeActionLeft)==0 then requestTurn();return end
  emu:setKeys(stair);commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 if phase=='scan' then
  if index==1 then planned=walkingDirection(dx,dy) end
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
   if planned==dirs[index] and (visits[cell(px,py,z)] or 0)<2 then score=score-10000000 end
   if not best or score<best.score then best={dir=dirs[index],score=score,key=cell(px,py,z)} end
  end
  emu:setKeys(2);phase='cancel';nextFrame=f+4;return
 end
 if phase=='cancel' then
  index=index+1
  if index<=#dirs then phase='scan';nextFrame=f+4;return end
  index=1
  if best then emu:setKeys(512+best.dir);phase='commit';nextFrame=f+4
  else
   -- Walking into a stair or a door uses the original controller, not flat route edges.
   local x,y,z=pos();local key=dy<y and 64 or 128
   if math.abs(dx-x)>math.abs(dy-y)*2 then key=dx<x and 32 or 16 end
   local k=cell(x,y,z);visits[k]=(visits[k] or 0)+1
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
end
callbacks:add('frame',function()
 if done then return end
 local ok,err=pcall(replayFrame)
 if not ok then
  emu:setKeys(0);out:write('ERROR frame='..f..' '..tostring(err)..'\n');out:flush();done=true
 end
end)
