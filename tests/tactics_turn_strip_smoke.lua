-- Disclosed budget/HP/phase fixtures; verify actual rendered native HUD pixels.
local f=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
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
 if f==180 then emu:setKeys(8)end
 if f==184 then emu:setKeys(0)end
 if f==240 then
  check(hudText(1,0,'XSOR DON GOO THEN ENEMIES'),'ready heroes and controlled marker rendered')
  check(emu:read16(gWin0V)==18 and emu:read16(gWin1V)==37024,'idle turn strip keeps narrow windows')
  emu:write16(gNativeMoveLeft,0);emu:write16(gNativeActionLeft,0)
 end
 if f==280 then
  check(hudText(1,0,'DON GOO THEN ENEMIES'),'exhausted controlled hero omitted')
  emu:write8(gNativePartyHealth+1,0)
 end
 if f==320 then
  check(hudText(1,0,'GOO THEN ENEMIES'),'knocked out companion omitted')
  emu:write16(sPartyMove+4,0);emu:write16(sPartyAction+4,0)
 end
 if f==360 then
  check(hudText(1,0,'END TURN FOR ENEMIES'),'fully exhausted party prompts enemy phase')
  emu:write16(gNativeEnemyFrames,120)
 end
 if f==400 then
  check(hudText(1,0,'ENEMIES THEN SOR GOO'),'enemy phase precedes next living party')
  emu:screenshot('@OUTPUT@/enemy-phase.png')
  out:close()
 end
end)
