local f=0
local out=io.open('@OUTPUT@/assembly.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function hudLine(row,text)
 -- Read the uploaded display, not a staging buffer midway through redraw.
 local control=emu:read16(0x04000008)
 local screen=0x06000000+((control&0x1f00)<<3)
 local tiles=0x06000000+((control&0x000c)<<12)
 for col=0,#text-1 do
  local c=text:sub(col+1,col+1)
  local glyph=c==' ' and 0 or (c>='0' and c<='9' and c:byte()-48+0x40 or c:byte()-65+0x60)
  local tile=emu:read16(screen+row*64+col*2)&0x3ff
  for y=0,7 do
   local pixels=emu:read32(gDebugFont0Tiles+glyph*32+y*4)
   pixels=(pixels&(pixels>>1)&(pixels>>2)&(pixels>>3))&0x11111111
   if emu:read32(tiles+tile*32+y*4)~=(0x11111111|(pixels<<1)) then
    out:write('HUD MISMATCH frame='..f..' row='..row..' column='..col..' char='..c..' tile='..tile..' scanline='..y..'\n');out:flush();return false
   end
  end
 end
 return true
end
local origin
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  check(emu:read16(gNativeAssembly)==1,'fresh round enters setup')
  check(emu:read32(sAssemblyTiles)~=0 and emu:read32(sAssemblyPalette)~=0,'original Donald character card allocates within palette budget')
  check(emu:read32(sCardTiles+4)==0 and emu:read32(sCardPalettes+4)==0,'setup borrows Fire artwork budget')
  local p=emu:read32(gFieldState);origin=emu:read32(p+0x18)
  check(hudLine(0,'MOVE 3 ACT 1 HP 080'),'setup renders highlighted Sora resources')
  emu:screenshot('@OUTPUT@/setup.png');emu:setKeys(64)
 end
 if f==184 then emu:setKeys(0) end
 if f==210 then
  check(emu:read32(emu:read32(gFieldState)+0x18)==origin,'setup does not move selected hero')
  check(emu:read16(gNativeGuard)==0 and emu:read16(gNativeActionLeft)==1,'setup preserves combat action and Guard')
  -- Explicit distinct budget fixture to expose selected-versus-active mistakes.
  emu:write16(gNativeMoveLeft,1);emu:write16(gNativeActionLeft,0)
  emu:write16(sPartyMove+2,2);emu:write16(sPartyAction+2,1)
  emu:setKeys(256)
 end
 if f==214 then emu:setKeys(0) end
 if f==230 then
  check(emu:read16(sAssemblyChoice)==1,'setup highlights Donald before deployment')
  check(hudLine(0,'MOVE 2 ACT 1 HP 056'),'highlighted Donald shows his own health instead of Sora health')
  check(hudLine(18,'DONALD MAGIC CURE BONUS'),'setup explains Donald role and healing specialty')
  -- Explicit owned-upgrade fixture; selection, drawing and deployment use input.
  emu:write8(gNativeRoster+5,3);emu:write8(gNativeRoster+9,5)
 end
 if f==238 then
  check(hudLine(2,'POWER 3 KEY 4 FIR 0 CUR 4'),'setup renders selected hero power and only owned matching sleight bonuses')
  emu:screenshot('@OUTPUT@/donald-loadout.png')
 end
 if f==240 then emu:setKeys(256) end
 if f==244 then emu:setKeys(0) end
 if f==280 then
  check(hudLine(0,'MOVE 3 ACT 1 HP 072'),'highlighted Goofy renders his own resources')
  check(hudLine(18,'GOOFY SPIN SHIELD GUARD'),'setup explains Goofy attack and protection role')
  check(hudLine(2,'POWER 0 KEY 0 FIR 0 CUR 0'),'Donald upgrades do not leak into Goofy loadout')
  -- Explicit recruitment fixture; subsequent roster swap uses native Up input.
  emu:write8(gNativeRoster,15);emu:setKeys(64)
 end
 if f==284 then emu:setKeys(0) end
 if f==330 then
  check(emu:read8(gNativeRoster+3)==3,'assembly cycles selected companion slot to unlocked Cloud')
  check(hudLine(0,'MOVE 3 ACT 1 HP 072'),'highlighted Cloud renders his own resources')
  check(hudLine(18,'CLOUD SWORD TARGET RANGE 64'),'setup explains Cloud targeted sword range')
  check(hudLine(2,'POWER 0 KEY 0 FIR 0 CUR 0'),'Cloud displays his own loadout after replacement')
  emu:screenshot('@OUTPUT@/cloud-loadout.png');emu:setKeys(256)
 end
 if f==334 then emu:setKeys(0) end
 if f==360 then
  check(hudLine(0,'MOVE 1 ACT 0 HP 080'),'returning to highlighted Sora restores his resource display')
  check(hudLine(2,'POWER 0 KEY 0 FIR 0 CUR 0'),'Sora displays his own loadout after companion cycling')
  emu:setKeys(256)
 end
 if f==364 then emu:setKeys(0) end
 if f==390 then
  check(hudLine(2,'POWER 3 KEY 4 FIR 0 CUR 4'),'returning to Donald restores his owned upgrades')
  emu:setKeys(1)
 end
 if f==394 then emu:setKeys(0) end
 if f==430 then
  check(emu:read16(gNativeAssembly)==0 and emu:read16(gNativeParty)==1,'confirm starts round controlling Donald')
  check(emu:read8(gNativeRoster+14)==1,'roster enters battle phase')
  check(emu:read32(sAssemblyTiles)==0 and emu:read32(sAssemblyPalette)==0,'confirmation releases setup artwork')
  check(emu:read32(sCardTiles+4)~=0 and emu:read32(sCardPalettes+4)~=0,'confirmation reconstructs Fire artwork')
  check(emu:read16(gNativeActionLeft)==1,'setup confirmation does not also attack')
  check(emu:read8(gNativeRoster+5)==3 and emu:read8(gNativeRoster+9)==5,'deploying preserves highlighted hero upgrades')
  emu:screenshot('@OUTPUT@/deployed.png');out:close()
 end
end)
