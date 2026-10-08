-- Position fixture at an original generated stair. Commands use native input.
local f=0
local out=io.open('@OUTPUT@/jump-stairs.txt','w')
local startZ=nil
local attachedZ=nil
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function field() return emu:read32(gFieldState) end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6);local cells=emu:read32(sMapCells)
  for y=1,rows-2 do for x=1,cols-2 do
   local c=cells+(y*cols+x)*32
   if not startZ and (emu:read16(c)&32)~=0 and emu:read8(c+2)==4 then
    local upper=emu:read32(c+8);local lower=emu:read32(c+12)
    if lower<1048576 and lower-upper>=16384 then
     startZ=lower;local p=field()
     emu:write32(p+0x18,(x*32+16)*256);emu:write32(p+0x1c,(y*16+14)*256-lower)
     emu:write32(p+0x20,lower);emu:write32(p+0x24,lower)
    end
   end
  end end
  check(startZ~=nil,'fixture finds an original tall stair')
  for i=0,5 do local t=emu:read32(sEnemyTasks+i*4);if t~=0 then emu:write32(emu:read32(t+4)+8,131072) end end
  emu:setKeys(66)
 end
 if f==184 then emu:setKeys(0) end
 if f==500 then
  check(emu:read16(gNativeClimbing)==1,'moving jump hands off to native stair attachment')
  check(emu:read16(gNativeBusy)==0,'jump-to-stair handoff releases command gate')
  check((emu:read32(field()+0x70)&0x800000)==0,'attached stair clears stale jumping flag')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'jump attachment costs one move and one action')
  attachedZ=emu:read32(field()+0x20);emu:screenshot('@OUTPUT@/attached.png');emu:setKeys(8)
 end
 if f==504 then emu:setKeys(0) end
 if f==620 then
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'turn renews resources after jump attachment')
  emu:setKeys(64)
 end
 if f==624 then emu:setKeys(0) end
 if f==780 then
  check(attachedZ and math.abs(emu:read32(field()+0x20)-(math.floor(attachedZ/4096)*4096-4096))<=48,'next command climbs exactly one original stair level')
  check(emu:read16(gNativeBusy)==0 and emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'subsequent climb retains normal movement-only cost')
  out:close()
 end
end)
