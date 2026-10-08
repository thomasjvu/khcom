-- Explicit position/encounter fixture; actions resolve through native input.
local f=0
local hurt=false
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s)out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush()end
local function stateGfx()local a=emu:read32(sFriends+20);return emu:read16(a+emu:read16(sFriends+14)*4)end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(16)end
 if f==184 or f==204 or f==244 or f==284 or f==324 or f==364 or f==724 then emu:setKeys(0)end
 if f==200 then emu:setKeys(128)end
 if f==240 then emu:setKeys(8)end
 if f==280 or f==320 then emu:setKeys(256)end
 if f==360 then emu:setKeys(1)end
 if f==370 then
  check(emu:read16(gNativeParty)==1 and emu:read8(gNativeRoster+2)==4,'Rally deployed and controlled')
  check(emu:read16(gNativeFriendPose)==6,'native card play selects casting pose')
  check(emu:read32(sFriends+4)==sRallyStateFrames and stateGfx()==4,'rear cast uses dedicated original art')
  check((emu:read16(sFriends+8)&1)==0,'cast holds once without looping')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==0,'cast spends action and preserves movement')
  emu:screenshot('@OUTPUT@/casting.png')
 end
 if f==700 then check(emu:read16(gNativeFriendPose)==0,'casting recovery returns to idle')end
 if f==720 then
  local p=emu:read32(gFieldState)
  for slot=0,2,2 do emu:write32(sPartyPos+slot*16,4000000)end
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then
    local w=emu:read32(t+4)
    if i==0 then
     for j=0,3 do emu:write32(w+8+j*4,emu:read32(p+0x18+j*4))end
     emu:write8(gNativeEnemyKind,0)
    else emu:write32(w+8,4000000)end
   end
  end
  emu:setKeys(8)
 end
 if f>720 and f<820 and emu:read16(gNativeFriendPose)==5 then
  if not hurt then emu:screenshot('@OUTPUT@/hurt.png')end
  hurt=emu:read32(sFriends+4)==sRallyStateFrames and stateGfx()==3
 end
 if f==820 then
  check(emu:read8(gNativePartyHealth+1)<64 and emu:read8(gNativePartyHealth+1)>0,'native enemy turn damages living Rally')
  check(hurt,'actual resolved damage selects dedicated rear recoil art')
 end
 if f==840 then
  local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
  local cells=emu:read32(sMapCells);local found=false
  for y=1,rows-2 do for x=1,cols-2 do
   local c=cells+(y*cols+x)*32
   if not found and (emu:read16(c)&32)~=0 and emu:read8(c+2)==4 then
    local upper=emu:read32(c+8);local lower=emu:read32(c+12)
    if lower<0x100000 and lower-upper>=16384 then
     local p=emu:read32(gFieldState)
     emu:write32(p+0x18,(x*32+16)*256);emu:write32(p+0x1c,(y*16+14)*256-lower)
     emu:write32(p+0x20,lower);emu:write32(p+0x24,lower);found=true
    end
   end
  end end
  check(found,'generated room supplies real multi-level stair')
  for i=0,5 do local t=emu:read32(sEnemyTasks+i*4);if t~=0 then emu:write32(emu:read32(t+4)+8,4000000)end end
  emu:setKeys(64)
 end
 if f==970 then
  check(emu:read16(gNativeClimbing)==1,'native movement attaches Rally to staircase')
  check(emu:read16(gNativeFriendPose)==7 and emu:read32(sFriends+4)==sRallyStateFrames and stateGfx()==5,'attached native climb uses dedicated rear pose')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'climb pose preserves native segment costs')
  emu:screenshot('@OUTPUT@/climbing.png');emu:setKeys(0);out:close()
 end
end)
