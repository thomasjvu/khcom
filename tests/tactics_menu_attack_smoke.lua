-- Explicit valid Key hand and enemy approach/HP fixture. Deployment, menu
-- selection, facing, cancellation and the actual strike use native input.
local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(16) end
 if f==200 then emu:setKeys(128) end
 if f==220 then emu:setKeys(8) end
 if f==184 or f==204 or f==224 or f==264 or f==304 or f==344 or f==384 or f==424 or f==464 or f==504 then emu:setKeys(0) end
 if f==260 then
  check(emu:read16(gNativeParty)==1 and emu:read8(gNativeRoster+2)==4,'Rally deployed through input')
  for i=0,11 do emu:write8(gNativeDeck+i,0);emu:write8(gNativeDeck+24+i,6);emu:write8(gNativeDeck+48+i,i<5 and 1 or 0) end
  emu:write8(gNativeDeck+73,0);emu:write8(gNativeDeck+77,0)
  local field=emu:read32(gFieldState);emu:write8(field+0x2c,0)
  for i=0,5 do
   local task=emu:read32(sEnemyTasks+i*4)
   if task~=0 then
    local w=emu:read32(task+4)
    emu:write32(w+8,emu:read32(field+0x18)+(i==0 and 4096 or 65536))
    emu:write32(w+12,emu:read32(field+0x1c));emu:write32(w+16,emu:read32(field+0x20));emu:write32(w+20,emu:read32(field+0x24))
    emu:write16(gNativeEnemyHp+i*2,30)
   end
  end
  emu:setKeys(260)
 end
 if f==300 then emu:setKeys(128) end
 if f==340 then emu:setKeys(1) end
 if f==380 then
  check(emu:read16(gNativeMenu)==4,'Attack opens confirmation instead of spending')
  check(emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+48)==1,'attack selection retains card and action')
  emu:setKeys(16)
 end
 if f==420 then
  check(emu:read8(emu:read32(gFieldState)+0x2c)==64,'D-pad faces Rally toward enemy')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'facing consumes no movement or action')
  emu:screenshot('@OUTPUT@/attack.png');emu:setKeys(2)
 end
 if f==460 then
  check(emu:read16(gNativeMenu)==1,'cancel Attack returns to Commands')
  check(emu:read16(gNativeEnemyHp)==30 and emu:read8(gNativeDeck+48)==1,'cancel neither attacks nor spends card')
  emu:setKeys(1)
 end
 if f==500 then emu:setKeys(1) end
 if f==620 then
  check(emu:read16(gNativeEnemyHp)==22,'confirmed Rally strike deals native Key damage')
  check(emu:read16(gNativeActionLeft)==0 and emu:read8(gNativeDeck+48)==2,'confirmed strike spends one card and action')
  check(emu:read16(gNativeMoveLeft)==3,'confirmed strike preserves movement budget')
  emu:screenshot('@OUTPUT@/struck.png');out:close()
 end
end)
