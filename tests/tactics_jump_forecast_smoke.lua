-- Input-only live terrain prediction; unresolved means no guaranteed marker.
local f=0
local output=assert(io.open('@OUTPUT@/checks.txt','w'))
local resolved,x,y,z=0,0,0,0
local function check(ok,message) output:write((ok and 'PASS ' or 'FAIL ')..message..'\n');output:flush() end
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
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==240 then emu:setKeys(4) end
 if f==280 or f==440 then emu:setKeys(1) end
 if f==320 then emu:setKeys(256) end
 if f==360 then emu:setKeys(@DIRECTION@) end
 if f==184 or f==244 or f==284 or f==324 or f==364 or f==444 then emu:setKeys(0) end
 if f==400 then
  resolved=emu:read16(gNativeJumpPrediction)
  x=emu:read32(gNativeJumpLanding);y=emu:read32(gNativeJumpLanding+4);z=emu:read32(gNativeJumpLanding+8)
  check(emu:read16(gNativeMenu)==6 and emu:read16(gNativeDirection)==@DIRECTION@,'native menu retains chosen direction')
  check(resolved>=1 and resolved<=4,'terrain reports landing, attachment or explicit unresolved route')
  check(hudText(4,0,resolved==1 and 'LANDING DIAMOND ON MAP' or resolved==3 and 'ATTACH TO STAIRS' or resolved==4 and 'CATCH LEDGE' or 'LANDING UNRESOLVED'),'native Jump HUD labels projected landing status')
  emu:screenshot('@OUTPUT@/landing-preview.png')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'prediction preserves movement and action')
  output:write('PREDICTION '..resolved..' '..x..','..y..','..z..'\n');output:flush()
 end
 if f==800 then
  local field=emu:read32(gFieldState)
  if resolved==1 then
   check(emu:read16(gNativeBusy)==0 and emu:read32(field+0x18)==x and emu:read32(field+0x1c)==y and emu:read32(field+0x20)==z,'actual native landing matches resolved forecast exactly')
  else check(resolved>=2 and resolved<=4,'attachment or unresolved terrain withheld guaranteed landing') end
  output:write('ACTUAL '..emu:read32(field+0x18)..','..emu:read32(field+0x1c)..','..emu:read32(field+0x20)..'\n');output:close()
 end
end)
