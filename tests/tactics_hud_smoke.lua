local f=0
local callback=0
local out=io.open('@OUTPUT@/hud.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function digit(tile,value,vram)
 local base=vram and (0x06000000+((emu:read16(0x04000008)&12)<<12)) or emu:read32(sHudTiles)
 for i=0,7 do
  local source=emu:read32(gDebugFont0Tiles+(0x40+value)*32+i*4)
  local pixels=(source|(source>>1)|(source>>2)|(source>>3))&0x11111111
  local expected=0x11111111|(pixels<<1)
  if emu:read32(base+tile*32+i*4)~=expected then return false end
 end
 return true
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  check(emu:read16(gGameState+0x32)==80,'initial authoritative Sora HP is 80')
  check(digit(17,0,false) and digit(18,8,false) and digit(19,0,false),'HUD RAM shows active HP 080')
  check(digit(18,8,true),'VBlank uploads the active HP tens digit')
  check(digit(130,8,false) and digit(131,0,false),'party footer RAM shows Sora HP 80')
  emu:screenshot('@OUTPUT@/initial.png')
  callback=emu:read32(gModeVBlankCallback);emu:write32(gModeVBlankCallback,0)
 end
 if f==200 then
  check(emu:read8(sHudPending)==1,'completed HUD waits for its delayed upload')
  -- Explicit delayed-callback/HP fixture; drawing remains native.
  emu:write16(gGameState+0x32,37);emu:write8(gNativePartyHealth+1,14);emu:write8(gNativePartyHealth+2,0)
 end
 if f==220 then
  check(digit(18,8,false) and digit(19,0,false),
    'pending completed HUD stays immutable before upload')
  emu:write32(gModeVBlankCallback,callback)
 end
 if f==260 then
  check(digit(18,3,false) and digit(19,7,false),'active HP change updates native glyph RAM')
  check(digit(18,3,true) and digit(19,7,true),'active HP change reaches VRAM')
  check(digit(134,1,false) and digit(135,4,false),'Donald HP change updates party footer')
  check(digit(138,0,false) and digit(139,0,false),'knocked-out Goofy shows zero HP')
  emu:screenshot('@OUTPUT@/changed.png');emu:write16(gNativeActionLeft,0)
 end
 if f==300 then
  local text='ACT SPENT START TURN'
  local matches=true
  local base=emu:read32(sHudTiles)
  for column=1,#text do
   local ch=text:sub(column,column)
   local glyph=ch==' ' and 0 or string.byte(ch)-string.byte('A')+0x60
   for row=0,7 do
    local source=emu:read32(gDebugFont0Tiles+glyph*32+row*4)
    local pixels=(source|(source>>1)|(source>>2)|(source>>3))&0x11111111
    matches=matches and emu:read32(base+(32+column)*32+row*4)==(0x11111111|(pixels<<1))
   end
  end
  check(matches,'spent action replaces play prompt with Start turn guidance')
  emu:screenshot('@OUTPUT@/spent-action.png');out:close()
 end
end)
