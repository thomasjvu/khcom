-- Input-only bounded exploration. No RAM writes, teleporting, or forced exits.
local out=io.open('@OUTPUT@/traversal.txt','w')
local f=0
-- @INPUT@
local nextFrame=180
local phase='scan'
local index=1
local dirs={16,32,64,128,80,96,144,160}
local best=nil
local planned=nil
local pendingMove=nil
local rejectedOrigin=nil
local rejectedDirections={}
local function rejectionKey(x,y,z)
 return x..':'..y..':'..z
end
local function rejectedDirectionAt(x,y,z,direction)
 if rejectedOrigin~=rejectionKey(x,y,z) then
  rejectedDirections={};rejectedOrigin=rejectionKey(x,y,z)
 end
 return rejectedDirections[direction] or false
end
local function rejectDirectionAt(x,y,z,direction)
 rejectedDirectionAt(x,y,z,direction)
 rejectedDirections[direction]=true
end
local function rejectedMove(before,x,y,z,move,action,preview)
 return preview~=0 and before.x==x and before.y==y and before.z==z and
  before.move==move and before.action==action
end
local visits={}
local commands=0
local done=false
local suspendStage=0
local suspendSnapshot=nil
local cloudRecruited=false
local cloudDeployed=false
local previousRoom=0
local previousWorld=0
local victoryFrame=nil
local completedRuns=0
local retryFrame=nil
local retrySeed=nil
local optionalRoute={0,1,2,3,4,9,8,1,2,3,4,5,10,11,10,5,6,7}
local routeStep=1
local roomVisitMasks={0,0,0}
local observedChests=0
local chestDebugFrame=0
local approachDebugFrame=0
local victoryHealth=nil
local composedRoutes=0
local composedSelection=nil
local composedApproach=nil
local function signed(v) if v>=2147483648 then return v-4294967296 end return v end
local function pos()
 local p=emu:read32(gFieldState)
 return signed(emu:read32(p+0x18)),signed(emu:read32(p+0x1c))+signed(emu:read32(p+0x20)),signed(emu:read32(p+0x20))
end
local function cell(x,y,z) return math.floor((x+2048)/4096)..':'..math.floor((y+1024)/2048)..':'..math.floor(z/2048) end
local jumpAttempts={}
-- A blocked connector may require approaching a prop from another side.
-- Remember attempts instead of resetting the same jump forever.
local function escapeJumpDirection(x,y,z,tx,ty)
 local origin=cell(x,y,z)
 local attempts=jumpAttempts[origin] or {}
 jumpAttempts[origin]=attempts
 local best,score
 for _,key in ipairs(dirs) do
  local px=x+((key&16)~=0 and 4096 or (key&32)~=0 and -4096 or 0)
  local py=y+((key&128)~=0 and 2048 or (key&64)~=0 and -2048 or 0)
  local value=(attempts[key] or 0)*65536+math.abs(px-tx)+2*math.abs(py-ty)
  if not score or value<score then best,score=key,value end
 end
 attempts[best]=(attempts[best] or 0)+1
 return best
end
local function chestGoal()
 local node=emu:read32(emu:read32(gFieldState)+0x80)
 while node~=0 do
  local task=emu:read32(node)
  if emu:read32(task)==gTaskDescMapGmk01 then
   local work=emu:read32(task+4);local placement=emu:read32(work)
   if (emu:read16(placement)&2)==0 then
    local z=signed(emu:read32(work+12))
    return signed(emu:read32(work+4)),signed(emu:read32(work+8))+z+4096,z
   end
  end
  node=emu:read32(node+8)
 end
end
local function chestCardKey()
 local slot=0;local wanted=nil
 for i=0,emu:read8(gNativeDeck+72)-1 do
  if emu:read8(gNativeDeck+48+i)==1 then
   if emu:read8(gNativeDeck+i)==0 and not wanted then wanted=slot end
   slot=slot+1
  end
 end
 if slot==0 then return 768 end
 if emu:read8(gNativeDeck+73)~=(wanted or 0) then return 256 end
 return 1
end
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
     local target=room==7 and 253 or room+1
     if allRooms then
      target=optionalRoute[routeStep+1] or 253
     elseif recruitCloud and emu:read16(gNativeFloor)==0 then
      if room==1 then target=8 elseif room==9 then target=4 end
     end
     if emu:read8(d+7)==target then
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
local function composedStairApproach()
 if composedApproach then return composedApproach end
 local x,y,z=pos();local room=emu:read32(gMapRoomState)
 local cols=emu:read16(room+4);local rows=emu:read16(room+6)
 local cells=emu:read32(sMapCells)
 for cy=1,rows-2 do for cx=1,cols-2 do
  local a=cells+(cy*cols+cx)*32
  local lower=signed(emu:read32(a+12));local upper=signed(emu:read32(a+8))
  if emu:read8(a+2)==4 and (emu:read16(a)&32)~=0 and lower<0x100000 and lower-upper>=16384 then
   local sx=(cx*32+16)*256;local sy=(cy*16+14)*256
   local score=math.abs(sx-x)+2*math.abs(sy-y)+math.abs(lower-z)
   if not composedApproach or score<composedApproach.score then
    composedApproach={x=sx,y=sy,z=lower,score=score}
   end
  end
 end end
 if composedApproach then
  out:write('COMPOSED APPROACH '..composedApproach.x..','..composedApproach.y..','..composedApproach.z..'\n');out:flush()
 end
 return composedApproach
end
local function encounterGoal()
 local room=emu:read8(gMapFloorState+6)
 if room~=7 and not (recruitCloud and emu:read16(gNativeFloor)==0 and room==9) then return nil end
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
-- Keep nearby companions available for their own actions, including Guard.
-- Match native Cure's raw-plane range and height checks; do not cycle toward
-- a recipient the game cannot heal from the controlled actor's position.
local function healingRecipient(x,y,z)
 if emu:read8(gNativePartyHealth)<60 then return 0 end
 local target,lowest=nil,24
 for member=1,2 do
  local hp=emu:read8(gNativePartyHealth+member)
  if emu:read8(gNativePartyHealth+3+member)>0 and hp<lowest then
   local a=sPartyPos+member*16
   local px=signed(emu:read32(a));local py=signed(emu:read32(a+4));local pz=signed(emu:read32(a+8))
   if math.abs(px-x)+math.abs(py-(y-z))<=24576 and math.abs(pz-z)<=6144 then target,lowest=member,hp end
  end
 end
 return target
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
 if emu:read16(gNativeActionLeft)==0 then return nil end
 local hand={};local wanted=nil;local healing=false
 local healTarget=healingRecipient(x,y,z)
 for i=0,emu:read8(gNativeDeck+72)-1 do
  if emu:read8(gNativeDeck+48+i)==1 then
   local kind=emu:read8(gNativeDeck+i);hand[#hand+1]=kind
   if kind==1 and not wanted then wanted=#hand-1 end
  end
 end
 if healTarget~=nil then
  for i,kind in ipairs(hand) do if kind==2 then wanted=i-1;healing=true;break end end
 end
 if not near and not healing then return nil end
 local threat=emu:read16(gNativeThreats)
 if not healing and threat>0 and emu:read8(gNativePartyHealth)<=threat+16 then
  for i,kind in ipairs(hand) do if kind==3 then wanted=i-1;break end end
 end
 if #hand==0 then return 768 end
 if not wanted then
  -- Cycle remaining cards into discard so native draw can expose more Fire.
  wanted=0
 end
 if emu:read8(gNativeDeck+73)~=wanted then return 256 end
 if healing and hand[wanted+1]==2 and
    emu:read16(gNativeCureTarget)~=healTarget then return 258 end
 return 1
end
-- Roster HP belongs to hero identity, including reserves. Replace KO
-- companions only at native assembly; never manufacture health or revive.
local function reserveReplacement()
 local unlocked=emu:read8(gNativeRoster)
 for slot=1,2 do
  if emu:read8(gNativePartyHealth+slot)==0 then
   local bestHero,bestHp=nil,0
   for hero=1,4 do
    local deployed=false
    for i=0,2 do if emu:read8(gNativeRoster+1+i)==hero then deployed=true end end
    local hp=emu:read8(gNativeRoster+19+hero)
    if (unlocked&(1<<hero))~=0 and not deployed and hp>bestHp then
     bestHero,bestHp=hero,hp
    end
   end
   if bestHero then return slot,bestHero end
  end
 end
end
local assemblyReplacement=nil
local endingTurn=false
local returningToSora=false
local function turnKey()
 if returningToSora then
  if emu:read16(gNativeEnemyFrames)>0 then return 0 end
  if emu:read16(gNativeParty)~=0 then return 516 end
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
  if party~=2 then return 516 end
  if emu:read8(gNativeDeck+73)~=guard then return 256 end
  out:write('GOOFY GUARD frame='..f..' threat='..threat..'\n');out:flush();return 1
 end
 out:write('TURN frame='..f..' hp='..emu:read8(gNativePartyHealth)..','..emu:read8(gNativePartyHealth+1)..','..emu:read8(gNativePartyHealth+2)..' threat='..threat..' guard='..emu:read16(gNativeGuard)..'\n');out:flush()
 endingTurn=false;returningToSora=true;return 8
end
local function requestTurn()
 endingTurn=true;best=nil;index=1;pressNative(turnKey());phase='release';nextFrame=f+4
end
-- Read-only structured collision evidence for investigating failed navigation.
local function navigationSnapshot(name)
 local file=io.open('@OUTPUT@/'..(name or 'navigation-snapshot.json'),'w')
 local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
 local x,y,z=pos();local dx,dy,dz=door();local cells=emu:read32(sMapCells)
 file:write(string.format('{"seed":%u,"floor":%u,"room":%u,"cols":%u,"rows":%u,"position":[%d,%d,%d],"goal":[%d,%d,%d],"cells":[',emu:read32(gNativeSeed),emu:read16(gNativeFloor),emu:read8(gMapFloorState+6),cols,rows,x,y,z,dx or 0,dy or 0,dz or 0))
 for cy=0,rows-1 do for cx=0,cols-1 do
  local a=cells+(cy*cols+cx)*32
  if cx+cy*cols>0 then file:write(',') end
  file:write(string.format('{"kind":%u,"flags":%u,"upper":%d,"lower":%d,"mask":[',emu:read8(a+2),emu:read16(a),signed(emu:read32(a+8)),signed(emu:read32(a+12))))
  local table=emu:read32(a+16)
  for py=0,15 do
   local bits=0
   for px=0,31 do
    local block=emu:read8(table+math.floor(px/8)+math.floor(py/8)*4)
    local bit=(emu:read8(gCellMasks+block*8+py%8)>>(7-px%8))&1
    bits=(bits<<1)|bit
   end
   file:write((py>0 and ',' or '')..string.format('%u',bits))
  end
  file:write(']}')
 end end
 file:write('],"props":[')
 local node=emu:read32(sColliderPoolObstacle+8);local count=0
 while node~=0 and count<128 do
  if (emu:read16(node+12)&2)==0 then
   local c=emu:read32(node)
   file:write((count>0 and ',' or '')..string.format('[%d,%d,%d,%d,%d]',signed(emu:read32(c+4)),signed(emu:read32(c+8)),signed(emu:read32(c+12)),signed(emu:read32(c+16)),signed(emu:read32(c+20))))
   count=count+1
  end
  node=emu:read32(node+8)
 end
 file:write('],"runtime":{')
 file:write(string.format('"party":%u,"move":%u,"action":%u,"busy":%u,"preview":%u,"menu":%u,"menu_choice":%u,"field_flags":%u,"roster":[',emu:read16(gNativeParty),emu:read16(gNativeMoveLeft),emu:read16(gNativeActionLeft),emu:read16(gNativeBusy),emu:read16(gNativePreview),emu:read16(gNativeMenu),emu:read16(gNativeMenuChoice),emu:read32(emu:read32(gFieldState)+0x70)))
 for i=0,rosterBytes-1 do file:write((i>0 and ',' or '')..emu:read8(gNativeRoster+i)) end
 file:write('],"deck":[')
 for i=0,77 do file:write((i>0 and ',' or '')..emu:read8(gNativeDeck+i)) end
 file:write('],"party_positions":[')
 for i=0,2 do
  file:write((i>0 and ',' or '')..'[')
  for j=0,3 do file:write((j>0 and ',' or '')..signed(emu:read32(sPartyPos+(i*4+j)*4))) end
  file:write(']')
 end
 file:write('],"party_health":[')
 for i=0,2 do file:write((i>0 and ',' or '')..emu:read8(gNativePartyHealth+i)) end
 file:write('],"enemies":[')
 for i=0,5 do
  local task=emu:read32(sEnemyTasks+i*4)
  file:write((i>0 and ',' or '')..'{"hp":'..emu:read16(gNativeEnemyHp+i*2)..',"charge":'..emu:read8(gNativeEnemyCharge+i)..',"position":[')
  if task~=0 then
   local work=emu:read32(task+4)
   for j=0,3 do file:write((j>0 and ',' or '')..signed(emu:read32(work+8+j*4))) end
  end
  file:write(']}')
 end
 file:write('],"save_notice":'..emu:read16(gNativeSaveNotice)..'}}\n');file:close()
 local raw=io.open('@OUTPUT@/'..(name or 'navigation-snapshot.json')..'.save-state.bin','wb')
 for i=0,suspendBytes-1 do raw:write(string.char(emu:read8(sSuspend+i))) end
 raw:close()
 if name=='save-attempt.json' then
  local encoded=io.open('@OUTPUT@/save-encoded.bin','wb')
  for i=0,1023 do encoded:write(string.char(emu:read8(sSaveBytes+i))) end
  encoded:close()
  local sram=io.open('@OUTPUT@/save-sram.bin','wb')
  for i=0,32767 do sram:write(string.char(emu:read8(0x0e000000+i))) end
  sram:close()
 end
end
local function finish(ok,why)
 navigationSnapshot()
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
-- A jump can end at a ledge whose original controller waits for held Up.
-- Treat it as native input work, not an animation that settles on its own.
local ledgeActive=false
local function busyInput()
 local busy=emu:read16(gNativeBusy)
 if busy~=0 then
  if busy==2 then
   local field=emu:read32(gFieldState)
   local task=emu:read32(emu:read32(field+0x94))
   local work=emu:read32(task+4)
   local state=emu:read32(work+0x94)
   if state==8 or state==9 then
    if not ledgeActive then
     out:write('LEDGE CLIMB frame='..f..'\n');out:flush()
     commands=commands+1;ledgeActive=true
    end
    emu:setKeys(64);nextFrame=f+4;return true
   end
  end
  nextFrame=f+8;return true
 end
 if ledgeActive then
  local x,y,z=pos()
  out:write('LEDGE SETTLED frame='..f..' position='..x..','..y..','..z..'\n');out:flush()
  ledgeActive=false;terrainPlan=nil;best=nil;index=1;phase='release';nextFrame=f+4
  return true
 end
 return false
end
local function replayFrame()
 f=f+1
 if done or f<180 or (suspendStage==2 and f<nextFrame) then return end
 local room=emu:read8(gMapFloorState+6)
 local world=emu:read16(gNativeFloor)
 if retryFrame then
  if f<retryFrame then emu:setKeys(4);return end
  emu:setKeys(0)
  if world==0 and emu:read16(gNativeResult)==0 and emu:read32(gCurrentMode)==sNativeMode and
     (emu:read32(gCurrentModeUpdate)&0xfffffffe)==NativeUpdate then
   local seed=emu:read32(gNativeSeed)
   if seed~=((retrySeed+0x9e3779b9)&0xffffffff) or emu:read8(gNativeRoster)~=initialRosterMask then
    finish(false,'retry did not advance seed and reset the recruited roster');return
   end
   out:write('RETRY VERIFIED frame='..f..' seed='..seed..'\n');out:flush()
   retryFrame=nil;retrySeed=nil;nextFrame=f+120;return
  end
  if f>retryFrame+180 then finish(false,'native retry did not initialize the next run');return end
  return
 end
 if composedSelection and emu:read16(gNativeBusy)==3 then composedSelection.walking=true end
 if collectChests and emu:read16(gNativeChests)~=observedChests then
  observedChests=emu:read16(gNativeChests)
  out:write('CHESTS '..observedChests..' frame='..f..'\n');out:flush()
 end
 if allRooms and world<3 then roomVisitMasks[world+1]=roomVisitMasks[world+1]|(1<<room) end
 if emu:read32(gCurrentMode)~=sNativeMode or (emu:read32(gCurrentModeUpdate)&0xfffffffe)~=NativeUpdate then
  out:write('MODE current='..string.format('%x',emu:read32(gCurrentMode))..' update='..string.format('%x',emu:read32(gCurrentModeUpdate))..' pending='..string.format('%x',emu:read32(gPendingMode))..' busy='..emu:read16(gNativeBusy)..'\n')
  finish(false,'left native tactics mode');return
 end
 if f==180 then navigationSnapshot('navigation-initial.json') end
 if recruitCloud and not cloudRecruited and (emu:read8(gNativeRoster)&8)~=0 then
  cloudRecruited=true;out:write('CLOUD RECRUITED frame='..f..'\n');out:flush()
 end
 if recruitCloud and not cloudDeployed and emu:read8(gNativeRoster+2)==3 and emu:read16(gNativeAssembly)==0 then
  cloudDeployed=true;out:write('CLOUD DEPLOYED frame='..f..'\n');out:flush()
  emu:screenshot('@OUTPUT@/cloud-deployed.png')
 end
 if goalWorlds==3 and world>=3 and emu:read16(gNativeResult)==2 then
  emu:setKeys(0)
  if not victoryFrame then
   victoryFrame=f;victoryHealth=emu:read8(gNativePartyHealth)
   out:write('VICTORY frame='..f..' hp='..victoryHealth..'\n');out:flush()
  end
  if f-victoryFrame>=120 then
   local valid=world==3 and emu:read8(gNativePartyHealth)==victoryHealth and (not recruitCloud or (cloudRecruited and cloudDeployed))
   -- A requested suspend must actually run; branch routing can skip its room.
   if suspendRoom and suspendRoom>0 then
    out:write('SUSPEND COVERAGE '..suspendStage..'\n');out:flush()
    valid=valid and suspendStage==3
   end
   if allRooms then
    out:write('ROOM MASKS '..roomVisitMasks[1]..','..roomVisitMasks[2]..','..roomVisitMasks[3]..'\n');out:flush()
    valid=valid and roomVisitMasks[1]==4095 and roomVisitMasks[2]==4095 and roomVisitMasks[3]==4095
   end
   if collectChests then valid=valid and emu:read16(gNativeChests)==9 end
   if composedDescent then
    out:write('COMPOSED ROUTES '..composedRoutes..'\n');out:flush()
    valid=valid and composedRoutes>0
   end
   if not valid then finish(false,'terminal state or required coverage/recruitment missing');return end
   completedRuns=completedRuns+1
   out:write('RUN COMPLETE '..completedRuns..' frame='..f..'\n');out:flush()
   if completedRuns<(goalRuns or 1) then
    retryFrame=f+12;retrySeed=emu:read32(gNativeSeed)
    emu:setKeys(4)
    victoryFrame=nil;victoryHealth=nil;cloudRecruited=false;cloudDeployed=false;assemblyReplacement=nil
    previousWorld=0;previousRoom=-1;suspendStage=0
    routeStep=1;roomVisitMasks={0,0,0}
    observedChests=0
    composedRoutes=0;composedSelection=nil;composedApproach=nil
    terrainPlan=nil;best=nil;visits={};jumpAttempts={};index=1;phase='release';nextFrame=f+120
   else
    finish(true,'completed '..completedRuns..' three-world runs; terminal floor and Sora HP remain stable for 120 frames')
   end
  end
  return
 end
 if goalWorlds>0 and goalWorlds<3 and world>=goalWorlds then finish(true,'completed '..goalWorlds..' worlds through native input');return end
 if goalWorlds==0 and room>=goalRoom then finish(true,'walked from native spawn to room '..room);return end
 if world~=previousWorld then
  out:write('WORLD '..world..' frames='..f..'\n');out:flush()
  previousWorld=world;previousRoom=-1
  routeStep=1
 end
 if room~=previousRoom then
  if allRooms and room==optionalRoute[routeStep+1] then routeStep=routeStep+1 end
  out:write('ROOM '..room..' frames='..f..' hp='..emu:read8(gNativePartyHealth)..','..emu:read8(gNativePartyHealth+1)..','..emu:read8(gNativePartyHealth+2)..'\n');out:flush()
  previousRoom=room;terrainPlan=nil;best=nil;visits={};jumpAttempts={};rejectedOrigin=nil;rejectedDirections={};index=1;phase='release';nextFrame=f+60
  composedSelection=nil;composedApproach=nil
 end
 if emu:read16(gNativeResult)~=0 then finish(false,'run ended before traversal goal');return end
 if f>goalFrames then finish(false,'bounded explorer did not reach '..(goalWorlds>0 and ('world '..goalWorlds) or ('room '..goalRoom)));return end
 if f<nextFrame then return end
 if waitNativeInput() then nextFrame=f+1;return end
 emu:setKeys(0)
 if busyInput() then return end
 if pendingMove then
  local x,y,z=pos()
  out:write('MOVE SETTLED frame='..f..' origin='..pendingMove.x..','..pendingMove.y..','..pendingMove.z..
   ' actual='..x..','..y..','..z..' move='..pendingMove.move..' TO '..emu:read16(gNativeMoveLeft)..
   ' action='..pendingMove.action..' TO '..emu:read16(gNativeActionLeft)..'\n');out:flush()
  local rejected=rejectedMove(pendingMove,x,y,z,emu:read16(gNativeMoveLeft),emu:read16(gNativeActionLeft),emu:read16(gNativePreview))
  local rejectedDirection=pendingMove.dir
  pendingMove=nil
  if rejected then
   rejectDirectionAt(x,y,z,rejectedDirection)
   local rejectedCost=emu:read16(gNativeRouteCost);if rejectedCost>=32768 then rejectedCost=rejectedCost-65536 end
   out:write('MOVE REJECTED cancel stale preview frame='..f..' direction='..rejectedDirection..' cost='..rejectedCost..'\n');out:flush()
   pressNative(2);best=nil;index=1;phase='release';nextFrame=f+4;return
  end
 end
 if suspendStage==2 and (world~=0 or room~=suspendRoom) then finish(false,'resume changed world or room');return end
 if suspendRoom and suspendRoom>0 and suspendStage<3 and world==0 and room==suspendRoom then
  if suspendStage==0 and phase=='scan' and (emu:read32(emu:read32(gFieldState)+0x70)&0xc1010)==0 and
     emu:read16(gNativePreview)==0 and
     emu:read16(gNativeAssembly)==0 and emu:read16(gNativeProgressReward)==0 and
     emu:read16(gNativeEnemyFrames)==0 and emu:read16(gNativeClimbing)==0 and emu:read16(gNativeReward)==0 then
   suspendSnapshot={}
   for i=0,2 do suspendSnapshot[i+1]=emu:read8(gNativePartyHealth+i) end
   suspendSnapshot.roster={}
   for i=0,rosterBytes-1 do suspendSnapshot.roster[i]=emu:read8(gNativeRoster+i) end
   suspendSnapshot.party=emu:read16(gNativeParty)
   suspendSnapshot.positions={};suspendSnapshot.enemies={};suspendSnapshot.charges={}
   for i=0,11 do suspendSnapshot.positions[i]=emu:read32(sPartyPos+i*4) end
   for i=0,5 do
    suspendSnapshot.enemies[i]=emu:read16(gNativeEnemyHp+i*2)
    suspendSnapshot.charges[i]=emu:read8(gNativeEnemyCharge+i)
   end
   suspendSnapshot.move=emu:read16(gNativeMoveLeft)
   suspendSnapshot.action=emu:read16(gNativeActionLeft)
   suspendSnapshot.guard=emu:read16(gNativeGuard)
   suspendSnapshot.deck={}
   for i=0,73 do suspendSnapshot.deck[i]=emu:read8(gNativeDeck+i) end
   out:write('SUSPEND REQUEST frame='..f..' flags='..string.format('%x',emu:read32(emu:read32(gFieldState)+0x70))..' room='..room..'\n');out:flush()
   pressNative(12);suspendStage=1;nextFrame=f+4;return
  elseif suspendStage==1 then
   navigationSnapshot('save-attempt.json')
   if emu:read16(gNativeSaveNotice)~=1 then finish(false,'input-only suspend failed');return end
   out:write('SUSPEND frame='..f..' room='..room..'\n');out:flush()
   emu:reset();suspendStage=2;nextFrame=f+330;return
  elseif suspendStage==2 then
   local matches=emu:read16(gNativeMoveLeft)==suspendSnapshot.move and
    emu:read16(gNativeActionLeft)==suspendSnapshot.action and emu:read16(gNativeGuard)==suspendSnapshot.guard
   for i=0,2 do matches=matches and emu:read8(gNativePartyHealth+i)==suspendSnapshot[i+1] end
   for i=0,73 do matches=matches and emu:read8(gNativeDeck+i)==suspendSnapshot.deck[i] end
   for i=0,rosterBytes-1 do matches=matches and emu:read8(gNativeRoster+i)==suspendSnapshot.roster[i] end
   matches=matches and emu:read16(gNativeParty)==suspendSnapshot.party
   for i=0,11 do matches=matches and emu:read32(sPartyPos+i*4)==suspendSnapshot.positions[i] end
   for i=0,5 do matches=matches and emu:read16(gNativeEnemyHp+i*2)==suspendSnapshot.enemies[i] and emu:read8(gNativeEnemyCharge+i)==suspendSnapshot.charges[i] end
   if not matches then finish(false,'resume changed party state enemy HP windups or deck');return end
   out:write('RESUME verified frame='..f..' room='..room..'\n');out:flush()
   suspendStage=3;terrainPlan=nil;best=nil;visits={};jumpAttempts={};index=1;phase='release';nextFrame=f+8;return
  end
 end
 if phase=='progress_release' then phase='scan';nextFrame=f+4;return end
 if gNativeProgressReward and emu:read16(gNativeProgressReward)~=0 then
  if recruitCloud and world==0 and room==9 and not cloudRecruited then
   pressNative(emu:read16(sProgressKind)==4 and 1 or 64)
   phase='progress_release';nextFrame=f+4;return
  end
  local hero=emu:read8(gNativeRoster+1+emu:read16(sProgressHero))
  pressNative(emu:read8(gNativeRoster+4+hero)>=8 and 256 or 1)
  phase='progress_release';nextFrame=f+4;return
 end
 if phase=='assembly_release' then phase='scan';nextFrame=f+4;return end
 if gNativeAssembly and emu:read16(gNativeAssembly)~=0 then
  if recruitCloud and cloudRecruited and not cloudDeployed and emu:read8(gNativeRoster+2)~=3 then
   local slot=emu:read16(sAssemblyChoice)
   -- Select Donald's slot, then cycle to unlocked Cloud.
   pressNative(slot==1 and 128 or 256)
   phase='assembly_release';nextFrame=f+4;return
  end
  if assemblyReplacement and emu:read8(gNativeRoster+1+assemblyReplacement.slot)==assemblyReplacement.hero then
   assemblyReplacement=nil
  end
  if not assemblyReplacement then
   local slot,hero=reserveReplacement()
   if slot then assemblyReplacement={slot=slot,hero=hero} end
  end
  if assemblyReplacement then
   local replaceSlot,replaceHero=assemblyReplacement.slot,assemblyReplacement.hero
   local slot=emu:read16(sAssemblyChoice)
   out:write('RESERVE SETUP frame='..f..' slot='..replaceSlot..' hero='..replaceHero..' hp='..emu:read8(gNativeRoster+19+replaceHero)..'\n');out:flush()
   pressNative(slot==replaceSlot and 128 or 256)
   phase='assembly_release';nextFrame=f+4;return
  end
  if emu:read16(sAssemblyChoice)~=0 then
   pressNative(512);phase='assembly_release';nextFrame=f+4;return
  end
  pressNative(8);phase='release';nextFrame=f+4;return
 end
 if phase=='release' then phase='scan';nextFrame=f+8;return end
 if emu:read16(gNativeReward)~=0 then
  out:write('REWARD confirm frame='..f..'\n');out:flush()
  pressNative(1);phase='release';nextFrame=f+4;return
 end
 if endingTurn or returningToSora then
  pressNative(turnKey());best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 if composedSelection and phase=='compose_arrival' then
  local x,y,z=pos();local selected=composedSelection
  local reached=math.abs(x-selected.x)<=512 and math.abs(y-selected.y)<=512 and z==selected.z
  if selected.walking and reached and emu:read16(gNativeMoveLeft)==selected.move-selected.cost and
     emu:read16(gNativeActionLeft)==selected.action then
   composedRoutes=composedRoutes+1
   out:write('COMPOSED DESCENT verified frame='..f..' count='..composedRoutes..' cost='..selected.cost..' position='..x..','..y..','..z..'\n');out:flush()
   emu:screenshot('@OUTPUT@/composed-descent-arrival.png')
  else
   out:write('COMPOSED DESCENT stopped before verified arrival frame='..f..'\n');out:flush()
  end
  composedSelection=nil;phase='release';nextFrame=f+4;return
 end
 if emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativePreview)==0 then
  requestTurn();return
 end
 local dx,dy,dz=door()
 if not dx then finish(false,'forward door missing');return end
 local approach=nil
 if composedDescent and composedRoutes==0 and world==0 and room==0 then
  approach=composedStairApproach()
  if approach then dx,dy,dz=approach.x,approach.y,approach.z end
 end
 local ex,ey,ez=encounterGoal()
 local chest=false
 if collectChests and not ex then
  local cx,cy,cz=chestGoal()
  if cx then dx,dy,dz=cx,cy,cz;chest=true end
 end
 if ex then
  dx,dy,dz=ex,ey,ez
  local x,y,z=pos()
  if phase=='scan' and index==1 and math.abs(x-ex)+math.abs(y-ey)<32768 and math.abs(z-ez)<=6144 and emu:read16(gNativeActionLeft)==0 then
   requestTurn();return
  end
 end
 if emu:read16(gNativeClimbing)~=0 then
  if phase=='compose_open' then
   emu:setKeys(256);phase='compose_floor';nextFrame=f+4;return
  elseif phase=='compose_floor' then
   phase='compose_choose';nextFrame=f+12;return
  elseif phase=='compose_choose' then
   local landing=emu:read8(gNativeReachCost+24);local choice=nil
   local x,y,z=pos()
   out:write('COMPOSED INSPECT frame='..f..' preview='..emu:read16(gNativePreview)..' landing='..landing..' reachable='..emu:read16(gNativeReachCount)..' floorvalid='..emu:read8(sRouteValid+31)..' climbmask='..emu:read16(gNativeClimbReachMask)..' move='..emu:read16(gNativeMoveLeft)..' position='..x..','..y..','..z..' ground='..signed(emu:read32(emu:read32(gFieldState)+0x24))..'\n');out:flush()
   emu:screenshot('@OUTPUT@/composed-descent-inspect.png')
   if emu:read16(gNativePreview)==3 and landing~=255 then
    local candidates={{16,25},{32,23},{64,17},{128,31},{80,18},{96,16},{144,32},{160,30}}
    for _,candidate in ipairs(candidates) do
     local cost=emu:read8(gNativeReachCost+candidate[2])
     if cost>landing and cost<=emu:read16(gNativeMoveLeft) then
      local address=sRoutePos+(candidate[2]+7)*16
      local x=signed(emu:read32(address));local z=signed(emu:read32(address+8));local y=signed(emu:read32(address+4))+z
      local score=math.abs(x-dx)+2*math.abs(y-dy)+math.abs(z-dz)+(visits[cell(x,y,z)] or 0)*16384
      if not choice or score<choice.score then choice={dir=candidate[1],x=x,y=y,z=z,cost=cost,score=score,move=emu:read16(gNativeMoveLeft),action=emu:read16(gNativeActionLeft)} end
     end
    end
   end
   if not choice then emu:setKeys(2);phase='compose_fallback';nextFrame=f+4;return end
   composedSelection=choice;emu:setKeys(choice.dir);phase='compose_target';nextFrame=f+4;return
  elseif phase=='compose_target' then
   phase='compose_confirm';nextFrame=f+8;return
  elseif phase=='compose_confirm' then
   if composedSelection and emu:read16(gNativeRouteCost)==composedSelection.cost and
      emu:read8(sPlayerEdge)==1 and emu:read8(sPlayerEdge+composedSelection.cost-1)==0 then
    emu:setKeys(1);commands=commands+1;phase='compose_arrival';nextFrame=f+4;return
   end
   composedSelection=nil;emu:setKeys(2);phase='compose_fallback';nextFrame=f+4;return
  elseif phase=='compose_fallback' then
   emu:setKeys(128);commands=commands+1;phase='release';nextFrame=f+4;return
  end
  -- Original stairs ascend with Up; each command is budgeted by the ROM.
  local _,_,z=pos()
  -- Exercise one composed route deliberately, then resume normal traversal.
  -- Descend toward the supporting floor when it is outside the current
  -- walking budget; refreshing a turn still uses native Start input.
  if composedDescent and composedRoutes==0 and emu:read16(gNativeMoveLeft)<2 then
   requestTurn();return
  end
  if composedDescent and composedRoutes==0 and emu:read16(gNativeMoveLeft)>=2 then
   out:write('COMPOSED START frame='..f..' move='..emu:read16(gNativeMoveLeft)..' height='..z..'\n');out:flush()
   emu:setKeys(640);phase='compose_open';nextFrame=f+4;return
  end
  emu:setKeys(dz>z and 128 or 64);commands=commands+1;phase='release';nextFrame=f+4;return
 end
 if phase=='scan' and index==1 then
  local x,y,z=pos()
  if approach and math.abs(x-dx)<2048 and math.abs(y-dy)<4096 and z==dz then
   if emu:read16(gNativePreview)~=0 then emu:setKeys(2);phase='release';nextFrame=f+4;return end
   if emu:read16(gNativeMoveLeft)<3 then requestTurn();return end
   emu:setKeys(64);commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
  end
  if chest and math.abs(x-dx)<2048 and math.abs(y-dy)<4096 and math.abs(z-dz)<2048 and
     (emu:read8(emu:read32(gFieldState)+0x2c)~=0 or (y>=dy-4096 and y<=dy+1024)) then
   if emu:read16(gNativePreview)~=0 then
    emu:setKeys(2);phase='release';nextFrame=f+4;return
   end
   if emu:read16(gNativeActionLeft)==0 then requestTurn();return end
   local key=emu:read8(emu:read32(gFieldState)+0x2c)~=0 and 320 or chestCardKey()
   if f-chestDebugFrame>=1000 then
    out:write('CHEST INPUT frame='..f..' position='..x..','..y..','..z..' goal='..dx..','..dy..','..dz..' key='..key..' angle='..emu:read8(emu:read32(gFieldState)+0x2c)..'\n');out:flush()
    emu:screenshot('@OUTPUT@/chest-approach.png');chestDebugFrame=f
   end
   pressNative(key);phase='release';nextFrame=f+4;return
  end
  local key=combatInput()
  if key then pressNative(key);phase='release';nextFrame=f+4;return end
  if (visits[cell(x,y,z)] or 0)>=3 and emu:read16(gNativeActionLeft)==0 then
   requestTurn();return
  end
  if (visits[cell(x,y,z)] or 0)>=3 and emu:read16(gNativeActionLeft)>0 and emu:read16(gNativeMoveLeft)>0 then
   local tx,ty=stairGoal(dx,dy,dz)
   local key=escapeJumpDirection(x,y,z,tx,ty)
   out:write('JUMP '..f..' '..x..' '..y..' '..z..'\n');out:flush()
   pressNative(key+2);commands=commands+1;visits[cell(x,y,z)]=0;phase='release';nextFrame=f+4;return
  end
 end
 local x0,y0,z0=pos()
 if not ex and not approach and phase=='scan' and math.abs(x0-dx)<8192 and math.abs(y0-dy)<4096 and math.abs(z0-dz)<2048 then
  local direct=directApproachDirection(x0,y0,z0,dx,dy,dz)
  local safe=walkingDirection(dx,dy)
  if direct and safe==direct then
   pressNative(direct);commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
  end
  if f-approachDebugFrame>=1000 then
   out:write('APPROACH DETOUR frame='..f..' position='..x0..','..y0..','..z0..' target='..dx..','..dy..','..dz..' direct='..tostring(direct)..' planned='..tostring(safe)..'\n');out:flush()
   approachDebugFrame=f
  end
 end
 local stair
 dx,dy,dz,stair=stairGoal(dx,dy,dz)
 local px,py=pos()
 if phase=='scan' and stair and math.abs(px-dx)<(stair~=64 and stair~=128 and 4096 or 2048) and math.abs(py-dy)<(stair~=64 and stair~=128 and 4096 or 4096) then
  if (stair&2)~=0 and emu:read16(gNativeActionLeft)==0 then requestTurn();return end
  pressNative(stair);commands=commands+1;best=nil;index=1;phase='release';nextFrame=f+4;return
 end
 if phase=='scan' then
  if index==1 then
   local x,y,z=pos()
   rejectedDirectionAt(x,y,z,0)
   planned=walkingDirection(dx,dy)
  end
  pressNative(512+dirs[index]);phase='inspect';nextFrame=f+4;return
 end
 if phase=='inspect' then
  local cost=emu:read16(gNativeRouteCost)
  if emu:read16(gNativePreview)==1 and cost==1 and not rejectedDirections[dirs[index]] then
   local x=emu:read16(sCursorX);if x>=32768 then x=x-65536 end
   local y=emu:read16(sCursorY);if y>=32768 then y=y-65536 end
   local a=sRoutePos+((y+4)*9+x+4)*16
   local px=signed(emu:read32(a));local z=signed(emu:read32(a+8))
   local py=signed(emu:read32(a+4))+z
   local score=math.abs(px-dx)+2*math.abs(py-dy)+math.abs(z-dz)+(visits[cell(px,py,z)] or 0)*16384
   local preferred=planned==dirs[index] and (visits[cell(px,py,z)] or 0)<2
   if preferred then score=score-10000000 end
   if not best or score<best.score then best={dir=dirs[index],score=score,key=cell(px,py,z)} end
   -- A negative preferred score cannot be beaten by any other direction.
   -- Keep this native-validated preview open instead of cancelling it and
   -- spending the rest of the scan merely to reopen the same destination.
   if preferred and score<0 then
    index=1;emu:setKeys(0);phase='confirm';nextFrame=f+4;return
   end
  end
  pressNative(2);phase='cancel';nextFrame=f+4;return
 end
 if phase=='cancel' then
  index=index+1
  if index<=#dirs then phase='scan';nextFrame=f+4;return end
  index=1
  if best then pressNative(512+best.dir);phase='commit';nextFrame=f+4
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
  local x,y,z=pos()
  pendingMove={x=x,y=y,z=z,move=emu:read16(gNativeMoveLeft),action=emu:read16(gNativeActionLeft),dir=best.dir}
  out:write('MOVE CONFIRM frame='..f..' direction='..best.dir..' cost='..emu:read16(gNativeRouteCost)..' cursor='..emu:read16(sCursorX)..','..emu:read16(sCursorY)..'\n');out:flush()
  pressNative(1);commands=commands+1
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
