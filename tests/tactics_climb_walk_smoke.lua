-- Approach position fixture; subsequent route, turn resources, save/reset
-- and party selection use controller input without emulated memory writes.
local f=0
local lowerZ,attachedZ,chosen,target,saved
local out=io.open('@OUTPUT@/climb-preview.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function player() return emu:read32(emu:read32(emu:read32(field()+0x94))+4) end
local function pos(address) return {emu:read32(address),emu:read32(address+4),emu:read32(address+8)} end
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
     lowerZ=lower;local p=field()
     emu:write32(p+0x18,(x*32+16)*256);emu:write32(p+0x1c,(y*16+14)*256-lower)
     emu:write32(p+0x20,lower);emu:write32(p+0x24,lower);found=true
    end
   end
  end end
  check(found,'generated room provides a real stair and supporting floor')
  for i=0,5 do local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then emu:write32(emu:read32(t+4)+8,131072) end
  end
  emu:setKeys(64)
 end
 if f==340 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeBusy)==0,'native stair entry completes before composed route')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'entry leaves two movement points and an action')
  attachedZ=emu:read32(field()+0x20);emu:setKeys(0)
 end
 if f==360 then emu:setKeys(640) end
 if f==364 then emu:setKeys(0) end
 if f==380 then emu:setKeys(256) end
 if f==384 then emu:setKeys(0) end
 if f==410 then
  check(emu:read16(gNativePreview)==3,'R switches from heights to the landing-floor cursor')
  check(emu:read16(gNativeRouteCost)==1 and emu:read8(gNativeReachCost+24)==1,'landing costs one descent segment with no duplicate walking charge')
  local count=0;local bounded=true
  for i=0,48 do local cost=emu:read8(gNativeReachCost+i)
   if cost~=255 then count=count+1;bounded=bounded and cost>0 and cost<=2 end
  end
  check(count==emu:read16(gNativeReachCount) and count>0 and count<=9 and bounded,'floor reachable markers include the cost of descent')
  local directions={{16,25},{32,23},{64,17},{128,31},{80,18},{96,16},{144,32},{160,30}}
  for _,d in ipairs(directions) do if not chosen and emu:read8(gNativeReachCost+d[2])==2 then chosen=d[1] end end
  check(chosen~=nil,'generated floor has a reachable walking step after descent')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1 and emu:read32(field()+0x20)==attachedZ,'floor inspection preserves attached position and both budgets')
  emu:screenshot('@OUTPUT@/floor-reach.png')
 end
 if f==420 then emu:setKeys(256) end
 if f==424 then emu:setKeys(0) end
 if f==430 then
  check(emu:read16(gNativePreview)==2 and emu:read16(gNativeRouteCost)==0,'R returns to the vertical origin cursor')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1 and emu:read32(field()+0x20)==attachedZ,'cursor-mode changes preserve attachment and resources')
  emu:setKeys(256)
 end
 if f==434 then emu:setKeys(0) end
 if f==440 and chosen then emu:setKeys(chosen) end
 if f==444 then emu:setKeys(0) end
 if f==470 then
  check(emu:read16(gNativeRouteCost)==2,'selected floor destination previews total descent plus walking cost')
  check(emu:read8(sPlayerEdge)==1 and emu:read8(sPlayerEdge+1)==0,'planned native path contains a climb edge followed by a walking edge')
  target=pos(sClimbPreviewPos);emu:screenshot('@OUTPUT@/descent-walk-preview.png');emu:setKeys(2)
 end
 if f==474 then emu:setKeys(0) end
 if f==510 then
  check(emu:read16(gNativePreview)==0 and emu:read16(gNativeClimbing)==1,'B cancels composed preview while retaining stair attachment')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1 and emu:read32(field()+0x20)==attachedZ,'cancelling the composed route preserves height and resources')
 end
 if f==520 then emu:setKeys(640) end
 if f==524 then emu:setKeys(0) end
 if f==540 then emu:setKeys(256) end
 if f==544 then emu:setKeys(0) end
 if f==560 and chosen then emu:setKeys(chosen) end
 if f==564 then emu:setKeys(0) end
 if f==590 then emu:setKeys(1) end
 if f==594 then emu:setKeys(0) end
 if f==610 then check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'one confirmation reserves both segments without spending action') end
 if f==820 then
  local p=pos(field()+0x18)
  check(emu:read32(player()+0x94)==0 and emu:read16(gNativeBusy)==0,'original controller descends and continues walking before returning control')
  check(target and math.abs(p[1]-target[1])<=512 and math.abs(p[2]+p[3]-target[2]-target[3])<=512,'actor reaches the previewed floor walking destination')
  check(lowerZ and p[3]==lowerZ,'walking continues on the supporting floor height')
  check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'descent-to-walking handoff neither renews nor double-charges resources')
  saved=p;emu:screenshot('@OUTPUT@/descent-walk-arrival.png')
 end
 if f==850 then emu:setKeys(12) end
 if f==854 then emu:setKeys(0) end
 if f==880 then check(emu:read16(gNativeSaveNotice)==1,'completed composed route suspends successfully');emu:reset() end
 if f==1050 then
  local p=pos(field()+0x18)
  check(saved and p[1]==saved[1] and p[2]==saved[2] and p[3]==saved[3],'reset restores exact position after composed route')
  check(emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1 and emu:read16(gNativeParty)==0,'reset preserves active hero and spent movement with action available')
 end
 if f==1080 then emu:setKeys(4) end
 if f==1084 then emu:setKeys(0) end
 if f==1110 then
  check(emu:read16(gNativeParty)==1,'Donald remains selectable after composed-route resume')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'Donald retains his independent unspent turn resources')
 end
 if f==1120 then emu:setKeys(4) end
 if f==1124 then emu:setKeys(0) end
 if f==1150 then
  check(emu:read16(gNativeParty)==2,'Goofy remains selectable after composed-route resume')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'Goofy retains his independent unspent turn resources')
 end
 if f==1160 then emu:setKeys(4) end
 if f==1164 then emu:setKeys(0) end
 if f==1190 then
  check(emu:read16(gNativeParty)==0 and emu:read16(gNativeMoveLeft)==0 and emu:read16(gNativeActionLeft)==1,'returning to Sora preserves the composed route cost')
  out:close()
 end
end)
