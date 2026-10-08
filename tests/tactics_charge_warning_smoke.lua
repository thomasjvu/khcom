-- Disclosed charge/phase flags; inspect native VRAM and window geometry.
local f=0
local assemblyHeld=false
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s)out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush()end
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
 if f>280 then
  if emu:read16(gNativeAssembly)~=0 then emu:setKeys(1);assemblyHeld=true
  elseif assemblyHeld then emu:setKeys(0);assemblyHeld=false end
 end
 if f==180 then emu:setKeys(8)end
 if f==184 then emu:setKeys(0)end
 if f==240 then emu:write8(gNativeEnemyCharge,1)end
 if f==280 then
  check(hudText(2,0,'GUARDIAN CHARGED'),'charged enemy warning is visible')
  check(hudText(1,0,'XSOR DON GOO THEN ENEMIES'),'warning preserves ready-party phase strip')
  check(emu:read16(gWin0V)==26 and emu:read16(gWin1V)==37024,'warning uses only one extra row')
  emu:screenshot('@OUTPUT@/charged.png')
  local r=emu:read32(gMapRoomState);local p=emu:read32(gFieldState)
  emu:write8(r+15,7);emu:write8(r+16,1);emu:write32(p+0x70,emu:read32(p+0x70)|16)
 end
 if f==500 then emu:write8(gNativeEnemyCharge,1)end
 if f==540 then
  check(emu:read16(gNativeBossReady)==1 and hudText(2,0,'ARMOR SLAM CHARGED'),'actual armor encounter names its charged attack')
  emu:write8(gNativeEnemyCharge,0)
 end
 if f==580 then
  check(emu:read16(gWin0V)==18,'cleared warning restores compact idle header')
  check(hudText(1,0,'XSOR DON GOO THEN ENEMIES'),'cleared charge preserves turn strip')
  emu:screenshot('@OUTPUT@/idle.png');out:close()
 end
end)
