-- Input-only native Rally attack and airborne animation coverage.
local f=0
local phase="slot"
local done=false
local front,back={},{}
local rise,fall=false,false
local origin,minZ=nil,nil
local captured={}
local frontLoops=false
local backLoops=false
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(ok,name) out:write((ok and 'PASS ' or 'FAIL ')..name..'\n');out:flush() end
local function gfx()
 local frames=emu:read32(sFriends+20)
 return emu:read16(frames+emu:read16(sFriends+14)*4)
end
local function height()
 local z=emu:read32(emu:read32(gFieldState)+0x20)
 return z>=0x80000000 and z-0x100000000 or z
end
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
local function command(keys,nextPhase)
 pressNative(keys);phase=nextPhase
end
callbacks:add('frame',function()
 f=f+1
 if done or f<180 then return end
 if f>5000 then check(false,'bounded native action regression completes');out:close();done=true;return end
 if phase=='front_wait' and emu:read16(gNativeFriendPose)==1 then
  local index=gfx();front[index]=true
  frontLoops=frontLoops or (emu:read16(sFriends+8)&1)~=0
  if index==21 and not captured.front then captured.front=true;emu:screenshot('@OUTPUT@/front-strike.png') end
 end
 if phase=='back_wait' and emu:read16(gNativeFriendPose)==1 then
  local index=gfx();back[index]=true
  backLoops=backLoops or (emu:read16(sFriends+8)&1)~=0
  if index==26 and not captured.back then captured.back=true;emu:screenshot('@OUTPUT@/back-strike.png') end
 end
 if phase=='jump_wait' then
  minZ=math.min(minZ,height())
  local pose=emu:read16(gNativeFriendPose);local index=gfx()
  if pose==3 and index==28 then
   rise=true
   if not captured.rise then captured.rise=true;emu:screenshot('@OUTPUT@/rising.png') end
  end
  if pose==4 and index==29 then
   fall=true
   if not captured.fall then captured.fall=true;emu:screenshot('@OUTPUT@/falling.png') end
  end
 end
 if waitNativeInput() then return end
 emu:setKeys(0)
 if phase=='slot' then command(16,'hero')
 elseif phase=='hero' then command(128,'deploy')
 elseif phase=='deploy' then command(8,'front_face')
 elseif phase=='front_face' then
  check(emu:read16(gNativeParty)==1 and emu:read8(gNativeRoster+2)==4,'native setup deploys and controls Rally')
  command(288,'front_attack')
 elseif phase=='front_attack' then command(1,'front_wait')
 elseif phase=='front_wait' then
  if emu:read16(gNativeBusy)~=0 or emu:read16(sFriends+50)~=0 then return end
  check(not frontLoops,'front strike plays once rather than looping')
  check(front[20] and front[21] and front[22],'native front attack plays dedicated windup strike recovery')
  check(emu:read16(gNativeActionLeft)==0,'front attack spends one action')
  command(8,'front_rest_wait')
 elseif phase=='front_rest_wait' then
  if emu:read16(gNativeEnemyFrames)~=0 then return end
  command(320,'back_attack')
 elseif phase=='back_attack' then
  check(emu:read16(sFriends+16)==5,'north facing uses the authored rear view')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'facing leaves renewed resources untouched')
  phase='back_select'
 elseif phase=='back_select' then
  local selected=emu:read8(gNativeDeck+73);local slot=0;local kind=nil
  for i=0,emu:read8(gNativeDeck+72)-1 do
   if emu:read8(gNativeDeck+48+i)==1 then
    if slot==selected then kind=emu:read8(gNativeDeck+i) end
    slot=slot+1
   end
  end
  if kind==0 then
   check(true,'rear melee explicitly selects Key card after draw advances hand')
   command(1,'back_wait')
  else command(256,'back_select') end
 elseif phase=='back_wait' then
  if emu:read16(gNativeBusy)~=0 or emu:read16(sFriends+50)~=0 then return end
  check(not backLoops,'rear strike plays once rather than looping')
  check(back[25] and back[26] and back[27],'native rear attack plays dedicated windup strike recovery')
  command(8,'back_rest_wait')
 elseif phase=='back_rest_wait' then
  if emu:read16(gNativeEnemyFrames)~=0 then return end
  origin=height();minZ=origin;command(2,'jump_wait')
 elseif phase=='jump_wait' then
  if emu:read16(gNativeBusy)~=0 then return end
  check(rise,'native jump selects dedicated rear rising art')
  check(fall,'native fall selects dedicated rear falling art')
  check(emu:read16(gNativeFriendPose)==0,'native landing returns Rally to idle')
  check(origin-minZ>=45*256,'new artwork preserves native full-height jump')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==0,'stationary jump spends action without movement')
  out:close();done=true
 end
end)
