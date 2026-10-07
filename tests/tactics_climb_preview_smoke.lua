local f=0
local startZ=nil
local savedZ=nil
local out=io.open('@OUTPUT@/climb-preview.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
local function player() return emu:read32(emu:read32(emu:read32(field()+0x94))+4) end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
  local cells=emu:read32(sMapCells);local found=false
  for y=1,rows-2 do for x=1,cols-2 do
   local c=cells+(y*cols+x)*32
   if not found and (emu:read16(c)&32)~=0 and emu:read8(c+2)==4 then
    local upper=emu:read32(c+8);local lower=emu:read32(c+12)
    if lower<0x100000 and lower-upper>=16384 then
     startZ=lower
     local p=field()
     emu:write32(p+0x18,(x*32+16)*256)
     emu:write32(p+0x1c,(y*16+14)*256-startZ)
     emu:write32(p+0x20,startZ);emu:write32(p+0x24,lower)
     found=true
    end
   end
  end end
  check(found,'generated room contains a native multi-level stair')
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then emu:write32(emu:read32(t+4)+8,131072) end
  end
  emu:setKeys(64)
 end
 if f==340 then
  check(emu:read16(gNativeClimbing)==1,'movement attaches to original stair controller')
  check(emu:read16(gNativeBusy)==0,'one climb segment completes and returns control')
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-4096))<=48,'held Up climbs exactly one sixteen-pixel level')
  check(emu:read16(gNativeMoveLeft)==2,'stair entry and segment cost one movement point')
  check(emu:read16(gNativeActionLeft)==1,'climbing preserves the combat action')
  emu:screenshot('@OUTPUT@/attached.png');emu:setKeys(0)
 end
 if f==360 then emu:setKeys(4) end
 if f==364 then emu:setKeys(0) end
 if f==390 then check(emu:read16(gNativeParty)==0,'party switch is rejected while attached to stairs');emu:setKeys(576) end
 if f==394 then emu:setKeys(0) end
 if f==410 then
  check(emu:read16(gNativePreview)==2 and emu:read16(gNativeRouteCost)==1,'attached Up opens a one-point vertical preview')
  check(emu:read16(gNativeMoveLeft)==2,'preview does not spend movement')
  emu:screenshot('@OUTPUT@/preview.png');emu:setKeys(2)
 end
 if f==414 then emu:setKeys(0) end
 if f==430 then
  check(emu:read16(gNativePreview)==0 and emu:read16(gNativeMoveLeft)==2,'B cancels vertical preview without dropping or spending movement')
  check(emu:read16(gNativeClimbing)==1,'preview cancellation retains native attachment');emu:setKeys(576)
 end
 if f==434 then emu:setKeys(0) end
 if f==450 then emu:setKeys(1) end
 if f==454 then emu:setKeys(0) end
 if f==530 then
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-8192))<=48,'second Up command advances a second vertical level')
  check(emu:read16(gNativeMoveLeft)==1,'second climb step spends another movement point')
  emu:setKeys(128)
 end
 if f==534 then emu:setKeys(0) end
 if f==610 then
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-4096))<=48,'Down descends one native stair level')
  check(emu:read16(gNativeMoveLeft)==0,'third climb step exhausts movement budget')
  emu:setKeys(64)
 end
 if f==614 then emu:setKeys(0) end
 if f==660 then savedZ=emu:read32(field()+0x20);emu:setKeys(12) end
 if f==664 then emu:setKeys(0) end
 if f==690 then
  check(startZ and math.abs(emu:read32(field()+0x20)-(startZ-4096))<=48,'exhausted movement prevents another climb step')
  check(emu:read16(gNativeSaveNotice)==1,'attached stair state saves successfully')
  emu:reset()
 end
 if f==860 then
  check(emu:read16(gNativeClimbing)==1 and emu:read16(gNativeMoveLeft)==0,'reset restores stair attachment and exhausted movement')
  check(savedZ and emu:read32(field()+0x20)==savedZ,'reset preserves the exact vertical climb position')
  emu:setKeys(8)
 end
 if f==864 then emu:setKeys(0) end
 if f==970 then
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeClimbing)==1,'ending turn restores movement without losing stair attachment')
  emu:setKeys(2)
 end
 if f==974 then emu:setKeys(0) end
 if f==1280 then
  check(emu:read32(player()+0x94)==0 and emu:read16(gNativeBusy)==0,'B drops from stairs and finishes native landing')
  check(emu:read16(gNativeActionLeft)==0,'dropping consumes the action')
  emu:screenshot('@OUTPUT@/landed.png');emu:setKeys(4)
 end
 if f==1284 then emu:setKeys(0) end
 if f==1330 then
  check(emu:read16(gNativeParty)==1,'party switching works again after landing')
  out:close()
 end
end)
