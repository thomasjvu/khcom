-- Input-only direct party selection and independent resource persistence.
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
 if f==140 then emu:setKeys(16) end
 if f==160 then emu:setKeys(128) end
 if f==180 then emu:setKeys(8) end
 if f==200 or f==220 then emu:setKeys(516) end
 if f==144 or f==164 or f==204 or f==224 then emu:setKeys(0) end
 if f==240 then check(emu:read8(gNativeRoster+2)==4 and emu:read16(gNativeParty)==0,'Rally deployed beside Sora and Goofy through setup input') end
 if f==240 or f==680 or f==840 then emu:setKeys(4) end
 if f==280 or f==320 or f==360 or f==560 or f==920 then emu:setKeys(64) end
 if f==400 or f==520 or f==600 or f==720 or f==800 or f==880 or f==960 then emu:setKeys(1) end
 if f==440 or f==760 then emu:setKeys(128) end
 if f==480 then emu:setKeys(2) end
 if f==640 then emu:setKeys(16) end
 if f==184 or (f>=244 and f<=964 and f%40==4) then emu:setKeys(0) end
 if f==440 then
  check(emu:read16(gNativeMenu)==8,'Party opens deployed hero selector')
  check(hudText(2,9,'HP80 MOVE3 ACT1') and hudText(3,9,'HP64 MOVE3 ACT1'),'Party explicitly labels movement for leader and Rally')
  check(hudText(4,9,'HP72 MOVE3 ACT1'),'Party explicitly labels Goofy movement without clipping')
  check(emu:read16(gNativeParty)==0 and emu:read16(gNativeMoveLeft)==3,'opening Party preserves active Sora and movement')
 end
 if f==480 then check(emu:read16(gNativeMenuChoice)==1 and emu:read16(gNativeParty)==0,'highlighting Rally does not switch control') end
 if f==520 then check(emu:read16(gNativeMenu)==1 and emu:read16(gNativeMenuChoice)==3,'cancel returns to Party command') end
 if f==600 then check(emu:read16(gNativeMenuChoice)==2,'selector reaches Goofy directly from Sora') end
 if f==640 then check(emu:read16(gNativeParty)==2 and emu:read16(gNativeMenu)==0,'confirm takes control of chosen Goofy') end
 if f==650 then
  check(emu:read16(gNativeBusy)~=0,'Goofy movement is in native progress')
  -- NativeHud must retain the compact window throughout label construction.
  check(emu:read16(gWin0V)==18,'normal movement retains compact two-row HUD')
  emu:screenshot('@OUTPUT@/compact-moving.png')
 end
 if f==680 then check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'Goofy movement spends only his movement') end
 if f==840 then check(emu:read16(gNativeParty)==0 and emu:read16(gNativeMoveLeft)==3,'selecting Sora restores his untouched budget') end
 if f==1000 then
  check(emu:read16(gNativeParty)==2 and emu:read16(gNativeMoveLeft)==2,'returning to Goofy preserves his spent movement')
  check(emu:read16(gNativeActionLeft)==1,'party selection never spends or renews action');emu:screenshot('@OUTPUT@/selected.png')
 end
 if f==600 then emu:screenshot('@OUTPUT@/party.png') end
 if f==1040 then emu:setKeys(4) end
 if f==1080 or f==1160 then emu:setKeys(1) end
 if f==1120 then emu:setKeys(64) end
 if f==1044 or f==1084 or f==1124 or f==1164 then emu:setKeys(0) end
 if f==1200 then
  check(emu:read16(gNativeParty)==1 and emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'Party selects Rally with her independent untouched budgets');out:close()
 end
end)
