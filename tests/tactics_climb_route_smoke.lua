-- Initial position fixture selects a real generated stair. All subsequent
-- preview, climb, descent, turn, save/reset and selection use native input.
local f=0
local startZ,savedZ,landingX,landingY
local out=io.open('@OUTPUT@/climb-preview.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function player() return emu:read32(emu:read32(emu:read32(field()+0x94))+4) end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(1) end
 if f==184 then emu:setKeys(0) end
 if f==210 then
  local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
  local cells=emu:read32(sMapCells);local found=false
  for y=1,rows-2 do for x=1,cols-2 do
   local c=cells+(y*cols+x)*32
   if not found and (emu:read16(c)&32)~=0 and emu:read8(c+2)==4 then
    local upper=emu:read32(c+8);local lower=emu:read32(c+12)
    if lower<0x100000 and lower-upper>=16384 then
     startZ=lower;local p=field()
     emu:write32(p+0x18,(x*32+16)*256)
     emu:write32(p+0x1c,(y*16+14)*256-startZ)
     emu:write32(p+0x20,startZ);emu:write32(p+0x24,lower);found=true
    end
   end
  end end
  check(found,'generated room has a stair with space for multiple vertical segments')
  for i=0,5 do local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then emu:write32(emu:read32(t+4)+8,131072) end
  end
  emu:setKeys(64)
 end
 if f==340 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeBusy)==0,'original controller attaches and finishes entry segment')
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-4096))<=48,'entry climbs one native sixteen-pixel segment')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'entry spends one movement and preserves action')
  emu:setKeys(0)
 end
 if f==360 then emu:setKeys(576) end
 if f==364 then emu:setKeys(0) end
 if f==380 then emu:setKeys(64) end
 if f==384 then emu:setKeys(0) end
 if f==410 then
  check(emu:read16(gNativePreview)==2 and emu:read16(gNativeRouteCost)==2,'two Up selections preview a two-point climb route')
  check((emu:read16(gNativeClimbReachMask)&5)==5,'both affordable ascent levels have reachable markers')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'multi-segment preview preserves both budgets')
  check(startZ and emu:read32(previewPos()+8)==startZ-12288,'selected marker predicts both native vertical segments')
  emu:screenshot('@OUTPUT@/two-level-preview.png')
 end
 if f==430 then emu:setKeys(1) end
 if f==434 then emu:setKeys(0) end
 if f==650 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeBusy)==0,'native controller finishes both planned climb segments')
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-12288))<=48,'actor reaches the selected two-level height')
  check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'two-level route charges two movement points once')
  savedZ=emu:read32(field()+0x20);emu:screenshot('@OUTPUT@/two-level-arrival.png')
 end
 if f==680 then emu:setKeys(576) end
 if f==684 then emu:setKeys(0) end
 if f==710 then
  check(emu:read16(gNativePreview)==2 and emu:read16(gNativeClimbReachMask)==0,'exhausted movement permits inspection without reachable markers')
  emu:setKeys(1)
 end
 if f==714 then emu:setKeys(0) end
 if f==740 then
  check(emu:read16(gNativeBusy)==0 and emu:read16(gNativeMoveLeft)==0 and emu:read32(field()+0x20)==savedZ,'unaffordable confirmation preserves height and resources')
  emu:setKeys(2)
 end
 if f==744 then emu:setKeys(0) end
 if f==800 then emu:setKeys(12) end
 if f==804 then emu:setKeys(0) end
 if f==830 then check(emu:read16(gNativeSaveNotice)==1,'multi-level attached state suspends successfully');emu:reset() end
 if f==1000 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeMoveLeft)==0 and emu:read32(field()+0x20)==savedZ,'reset restores exact climbed height and exhausted movement')
 end
 if f==1030 then emu:setKeys(8) end
 if f==1034 then emu:setKeys(0) end
 if f==1090 then check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeClimbing)==1,'new turn restores movement while retaining attachment') end
 if f==1100 then emu:setKeys(640) end
 if f==1104 then emu:setKeys(0) end
 if f==1140 or f==1180 then emu:setKeys(128) end
 if f==1144 or f==1184 then emu:setKeys(0) end
 if f==1200 then
  check(emu:read16(gNativePreview)==2 and emu:read16(gNativeRouteCost)==3,'three Down selections preview a three-point descent')
  check((emu:read16(gNativeClimbReachMask)&42)==42,'all three affordable descent levels are marked reachable')
  check(startZ and emu:read32(previewPos()+8)==startZ,'descent marker lands on the supporting floor')
  landingX=emu:read32(previewPos());landingY=emu:read32(previewPos()+4)
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'descent preview spends neither movement nor action')
  emu:screenshot('@OUTPUT@/three-level-descent.png')
 end
 if f==1220 then emu:setKeys(1) end
 if f==1224 then emu:setKeys(0) end
 if f==1500 then
  check(emu:read32(player()+0x94)==0 and emu:read16(gNativeBusy)==0,'original descent exits stairs and returns control on the floor')
  check(startZ and math.abs(emu:read32(field()+0x20)-startZ)<=48,'native landing reaches the previewed floor height')
  check(landingX and math.abs(emu:read32(field()+0x18)-landingX)<=48 and math.abs(emu:read32(field()+0x1c)-landingY)<=48,'floor marker predicts the original controller landing offset')
  check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'three-segment descent charges exactly three movement and preserves action')
  emu:screenshot('@OUTPUT@/descent-arrival.png')
 end
 if f==1530 then emu:setKeys(4) end
 if f==1534 then emu:setKeys(0) end
 if f==1550 then check(emu:read16(gNativeParty)==1,'party selection works after the planned descent');out:close() end
end)
