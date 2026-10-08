-- Input-only: native full-height jumps and tactical travel/action bounds.
local sawRise=false
local sawFall=false
local f=0
local out=io.open('@OUTPUT@/jumps.txt','w')
local dirs={16,32,64,128,80,96,144,160}
local chosen=nil
local origin=nil
local minimum=nil
local maximumTravel=0
local moving=nil
local movingMinimum=nil
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function signed(v) if v>=2147483648 then return v-4294967296 end return v end
local function pos()
 local p=emu:read32(gFieldState)
 return {signed(emu:read32(p+0x18)),signed(emu:read32(p+0x1c)),signed(emu:read32(p+0x20))}
end
callbacks:add('frame',function()
 f=f+1
 if testParty then
  if f==120 or (testParty==2 and f==150) then emu:setKeys(516) end
  if f==124 or f==154 then emu:setKeys(0) end
  if f>180 and f<400 then
   local pose=emu:read16(gNativeFriendPose+(testParty-1)*2)
   sawRise=sawRise or pose==3;sawFall=sawFall or pose==4
  end
 end
 if testParty and f==220 then emu:screenshot('@OUTPUT@/rising.png') end
 if testParty and f==260 then emu:screenshot('@OUTPUT@/falling.png') end
 if f==180 then origin=pos();minimum=origin[3];emu:setKeys(2) end
 if f==184 then emu:setKeys(0) end
 if f>180 and f<400 then minimum=math.min(minimum,pos()[3]) end
 if f==400 then
  if testParty then
   check(emu:read16(gNativeParty)==testParty,"jump keeps selected party member")
   check(sawRise,"selected friend uses original jump rise pose")
   check(sawFall,"selected friend uses original falling pose")
   check(emu:read16(gNativeFriendPose+(testParty-1)*2)==0,"selected friend returns to idle after landing")
  end
  local p=pos()
  check(origin[3]-minimum>=45*256,'one B press reaches native full jump height after early release')
  check(emu:read16(gNativeBusy)==0 and math.abs(p[3]-origin[3])<=48,'stationary jump lands through native physics')
  check(math.abs(p[1]-origin[1])<=768 and math.abs(p[2]-origin[2])<=768,'stationary jump preserves field position')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==0,'stationary jump spends one action and no movement')
  emu:screenshot('@OUTPUT@/stationary.png')
 end
 if f==450 then emu:setKeys(8) end
 if f==454 then emu:setKeys(0) end
 if f>=600 and f<1240 then
  local slot=math.floor((f-600)/80)+1;local phase=(f-600)%80
  if phase==0 then emu:setKeys(512+dirs[slot]) end
  if phase==4 or phase==16 or phase==28 then emu:setKeys(0) end
  if phase==12 then emu:setKeys(dirs[slot]) end
  if phase==24 then
   if not chosen and emu:read16(gNativeRouteCost)==2 then chosen=dirs[slot] end
   emu:setKeys(2)
  end
 end
 if f==1300 then
  check(chosen~=nil,'native spawn has a clear two-segment corridor for moving jump')
  moving=pos();movingMinimum=moving[3]
  if chosen then emu:setKeys(chosen+2) end
 end
 if f==1304 then emu:setKeys(0) end
 if f>1300 and f<1570 and chosen then
  local p=pos();movingMinimum=math.min(movingMinimum,p[3])
  maximumTravel=math.max(maximumTravel,math.abs(p[1]-moving[1])+2*math.abs(p[2]-moving[2]))
 end
 if f==1570 then
  if chosen then
   local p=pos()
   check(moving[3]-movingMinimum>=45*256,'moving jump keeps native full-height ascent')
   check(maximumTravel>=20*256,'moving jump from rest clears a useful tactical distance')
   check(maximumTravel<=36*256,'moving jump stays within bounded world-space travel')
   check(emu:read16(gNativeBusy)==0,'moving jump finishes native landing')
   check(math.abs(p[3]-moving[3])<=48,'moving jump lands on the selected corridor floor')
   check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'moving jump spends exactly one move and one action')
   emu:screenshot('@OUTPUT@/moving.png')
  end
  out:close()
 end
end)
