-- Explicit cards/power/enemy-position fixtures; menus and strikes use input.
local f=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function setup(value,power)
 local selected=emu:read8(gNativeDeck+73);local slot=0
 for i=0,emu:read8(gNativeDeck+72)-1 do
  if emu:read8(gNativeDeck+48+i)==1 then
   if slot==selected then emu:write8(gNativeDeck+i,0);emu:write8(gNativeDeck+24+i,value);break end
   slot=slot+1
  end
 end
 local hero=emu:read8(gNativeRoster+1+emu:read16(gNativeParty))
 emu:write8(gNativeRoster+4+hero,power)
 local field=emu:read32(gFieldState)
 for i=0,5 do
  local task=emu:read32(sEnemyTasks+i*4)
  if task~=0 then
   local work=emu:read32(task+4)
   emu:write32(work+8,emu:read32(field+0x18)+(i==0 and 0 or 65536))
   emu:write32(work+12,emu:read32(field+0x1c)-5120)
   emu:write32(work+16,emu:read32(field+0x20));emu:write32(work+20,emu:read32(field+0x24))
  end
 end
end
callbacks:add('frame',function()
 f=f+1
 if testRally and f==100 then emu:setKeys(256) end
 if testRally and f==120 then emu:setKeys(128) end
 if testRally and (f==104 or f==124) then emu:setKeys(0) end
 if f==180 or f==600 or f==1080 then emu:setKeys(8) end
 if f==240 or f==760 or f==1240 then emu:setKeys(4) end
 if f==280 then emu:setKeys(128) end
 if f==420 or f==910 or f==1390 then emu:setKeys(64) end
 if f==424 or f==914 or f==1394 then emu:setKeys(0) end
 if f==320 or f==440 or f==840 or f==920 or f==1320 or f==1420 then emu:setKeys(1) end
 if f==184 or f==604 or f==1084 or f==244 or f==764 or f==1244 or f==284 or f==804 or f==1284 or f==324 or f==444 or f==844 or f==924 or f==1324 or f==1424 then emu:setKeys(0) end
 if f==380 then setup(5,0);emu:write16(gNativeEnemyHp,40) end
 if f==400 then
  if testRally then check(emu:read16(gNativeParty)==1 and emu:read8(gNativeRoster+2)==4,'native assembly deploys and controls Rally for sword attacks') end
  check(emu:read16(gNativeMenu)==4,'native sword confirmation menu opens')
  check(hudText(3,0,'FACE ENEMY OR CHEST'),'sword confirmation names chest targeting')
  check(hudText(4,0,'ON HIT 07'),'value five previews seven on-hit damage')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'inspection preserves resources')
  emu:screenshot('@OUTPUT@/normal.png')
 end
 if f==560 then
  check(emu:read16(gNativeEnemyHp)==33,'native sword hit deals previewed seven damage')
  check(emu:read16(gNativeActionLeft)==0,'native sword spends one action')
 end
 if f==720 then setup(0,8) end
 if f==880 then setup(0,8) end
 if f==900 then
  check(hudText(4,0,'ON HIT 11'),'zero card previews eleven with eight personal power')
  emu:screenshot('@OUTPUT@/zero-upgraded.png')
 end
 if f==1040 then out:write('HP '..emu:read16(gNativeEnemyHp)..'\n');check(emu:read16(gNativeEnemyHp)==22,'zero-card native hit includes personal power exactly') end
 if f==1220 or f==1340 then setup(1,8) end
 if f==1360 then
  check(hudText(4,0,'CARD BREAK NO DAMAGE'),'low card warns that damage will break')
  emu:screenshot('@OUTPUT@/break.png')
 end
 if f==1540 then
  out:write('BREAK HP '..emu:read16(gNativeEnemyHp)..' count '..emu:read16(gNativeBreaks)..'\n')
  check(emu:read16(gNativeEnemyHp)==22,'broken native strike leaves enemy HP unchanged')
  check(emu:read16(gNativeBreaks)==1,'broken strike registers exactly one card break')
  check(emu:read16(gNativeActionLeft)==0,'broken strike still spends one action')
  out:close()
 end
end)
