local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
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
 if f==180 then check((emu:read8(gNativeRoster)&16)~=0,'Rally available at setup');emu:setKeys(16) end
 if f==184 or f==204 or f==224 or f==264 or f==304 or f==344 or f==384 or f==424 or f==464 or f==504 then emu:setKeys(0) end
 if f==200 then emu:setKeys(128) end
 if f==220 then
  check(emu:read16(gWin0V)==56 and emu:read16(gWin1V)==37024,'setup windows leave central field visible')
  check(hudText(0,0,'PARTY SETUP A DEPLOY'),'setup heading rendered with single stroke font')
  check(hudText(2,1,'RALLY') and hudText(2,9,'HP64 M3 A1 P0'),'deployed Rally health and budgets visible')
  check(hudText(5,0,'RESERVE HP DON56'),'reserve Donald health visible after swap')
  local tiles=emu:read32(sAssemblyTiles);local palette=emu:read32(sAssemblyPalette)
  local tileMatch=tiles~=0;local paletteMatch=palette~=0
  if tiles~=0 then
   local address=0x06010000+emu:read16(tiles+6)*32
   for offset=0,511 do if emu:read8(address+offset)~=emu:read8(sRallyCardTiles+offset) then tileMatch=false end end
  end
  if palette~=0 then
   local address=0x05000200+emu:read16(palette+6)*32
   for offset=0,15 do if emu:read16(address+offset*2)~=emu:read16(sRallyCardPalette+offset*2) then paletteMatch=false end end
  end
  check(tileMatch,'Rally original card tiles uploaded to allocated OBJ bank')
  check(paletteMatch,'Rally original card palette uploaded to allocated bank')
  check(emu:read8(gNativeRoster+2)==4,'Rally selected into companion slot');emu:screenshot('@OUTPUT@/setup.png');emu:setKeys(8) end
 if f==260 then check(emu:read32(sAssemblyTiles)==0 and emu:read32(sAssemblyPalette)==0,'deployment releases Rally setup card resources');check(emu:read16(gNativeParty)==1 and emu:read16(gNativeAssembly)==0,'deploy and control Rally');emu:setKeys(4) end
 if f==300 then check(emu:read16(gWin0V)==40 and emu:read16(gWin1V)==37024,'commands leave 104 pixels of field unobscured');check(hudText(1,2,'MOVE') and hudText(1,17,'PARTY') and hudText(3,17,'SUSPEND'),'commands use compact two column layout');check(hudText(4,0,'SOR XRAL GOO THEN ENEMIES'),'commands retain party phase order');check(emu:read16(gNativeMenu)==1,'open tactical commands');check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'opening menu preserves budgets');emu:screenshot('@OUTPUT@/menu.png');emu:setKeys(128) end
 if f==340 then check(emu:read16(gNativeMenuChoice)==1,'navigate attack without moving');emu:setKeys(128) end
 if f==380 then check(emu:read16(gNativeMenuChoice)==2,'navigate skills');emu:setKeys(1) end
 if f==420 then check(emu:read16(gNativeMenu)==2,'skills submenu opens');emu:screenshot('@OUTPUT@/skills.png');emu:setKeys(2) end
 if f==460 then check(emu:read16(gNativeMenu)==1,'back returns to commands');emu:setKeys(64) end
 if f==500 then emu:setKeys(64) end
 if f==540 then emu:setKeys(1) end
 if f==544 then emu:setKeys(0) end
 if f==580 then check(emu:read16(gNativePreview)==1,'Move opens reachable-tile preview');check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'menu and preview navigation spend no resources');emu:screenshot('@OUTPUT@/reach.png') end
 if f==610 then emu:setKeys(2) end
 if f==614 or f==654 or f==694 or f==734 then emu:setKeys(0) end
 if f==650 then emu:setKeys(4) end
 if f==690 then emu:setKeys(64) end
 if f==730 then emu:setKeys(1) end
 if f==770 then check(emu:read16(gNativeSaveNotice)==1,'menu suspend saves Rally');emu:reset() end
 if f==1100 then
  check(emu:read8(gNativeRoster+2)==4 and emu:read16(gNativeParty)==1,'reset preserves deployed active Rally')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'reset preserves Rally budgets')
  emu:screenshot('@OUTPUT@/resumed.png')
 end
 if f==1140 then emu:setKeys(516) end
 if f==1144 or f==1184 then emu:setKeys(0) end
 if f==1180 then check(emu:read16(gNativeParty)==2 and emu:read16(gNativeMenu)==0,'L Select switches to Goofy without opening Commands');emu:setKeys(516) end
 if f==1220 then check(emu:read16(gNativeParty)==0 and emu:read16(gNativeMenu)==0,'L Select cycles to Sora');out:close() end
end)
