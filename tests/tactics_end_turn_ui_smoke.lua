-- Input-only End Turn forecast/menu regression.
local frame=0
local initialTurn=nil
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(ok,name) out:write((ok and 'PASS ' or 'FAIL ')..name..'\n');out:flush() end
local function hudText(row,column,text)
 local control=emu:read16(0x04000008)
 local screen=0x06000000+((control&0x1f00)<<3)
 local tiles=0x06000000+((control&0x000c)<<12)
 for index=1,#text do
  local c=text:byte(index)
  local glyph=c>=48 and c<=57 and c-48+1 or c>=65 and c<=90 and c-65+11 or 0
  local tile=emu:read16(screen+(row*32+column+index-1)*2)&0x3ff
  for y=0,6 do
   local bits=emu:read8(sUiGlyphs+glyph*7+y)
   local pixels=emu:read32(tiles+tile*32+y*4)
   for x=0,4 do
    local expected=(bits&(1<<(4-x)))~=0 and 3 or 1
    if ((pixels>>((x+1)*4))&15)~=expected then return false end
   end
  end
 end
 return true
end
callbacks:add('frame',function()
 frame=frame+1
 if frame==180 then emu:setKeys(8) end
 if frame==220 then emu:setKeys(4) end
 if frame==260 or frame==300 or frame==340 or frame==380 then emu:setKeys(128) end
 if frame==420 or frame==540 or frame==580 then emu:setKeys(1) end
 if frame==184 or (frame>=224 and frame<=584 and frame%40==24) then emu:setKeys(0) end
 if frame==460 then
  check(emu:read16(gNativeMenu)==7,'End Turn opens confirmation')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'viewing forecast preserves active resources')
  for slot=0,2 do
   local hp=emu:read8(gNativePartyHealth+slot)
   local damage=math.min(99,emu:read16(gNativeThreats+slot*2))
   local text=string.format('HP%02d M3 A1 HIT%02d',hp,damage)
   check(hudText(slot+2,8,text),'rendered hero '..slot..' forecast matches native threat and health')
  end
  check(hudText(5,0,'HIT IS EXPECTED DAMAGE'),'forecast legend visible')
  initialTurn=emu:read16(gNativeTurn)
  emu:screenshot('@OUTPUT@/forecast.png');emu:setKeys(2)
 end
 if frame==464 then emu:setKeys(0) end
 if frame==500 then
  check(emu:read16(gNativeMenu)==1 and emu:read16(gNativeMenuChoice)==4,'cancel returns to End Turn command')
  check(emu:read16(gNativeTurn)==initialTurn,'cancel does not advance enemy phase')
 end
 if frame==620 then
  check(emu:read16(gNativeMenu)==0 and emu:read16(gNativeTurn)==initialTurn+1,'confirmation advances native turn once')
  out:close()
 end
end)
