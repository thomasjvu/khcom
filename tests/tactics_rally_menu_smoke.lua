local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then check((emu:read8(gNativeRoster)&16)~=0,'Rally available at setup');emu:setKeys(16) end
 if f==184 or f==204 or f==224 or f==264 or f==304 or f==344 or f==384 or f==424 or f==464 or f==504 then emu:setKeys(0) end
 if f==200 then emu:setKeys(128) end
 if f==220 then check(emu:read8(gNativeRoster+2)==4,'Rally selected into companion slot');emu:screenshot('@OUTPUT@/setup.png');emu:setKeys(8) end
 if f==260 then check(emu:read16(gNativeParty)==1 and emu:read16(gNativeAssembly)==0,'deploy and control Rally');emu:setKeys(260) end
 if f==300 then check(emu:read16(gNativeMenu)==1,'open tactical commands');check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'opening menu preserves budgets');emu:screenshot('@OUTPUT@/menu.png');emu:setKeys(128) end
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
 if f==650 then emu:setKeys(260) end
 if f==690 then emu:setKeys(64) end
 if f==730 then emu:setKeys(1) end
 if f==770 then check(emu:read16(gNativeSaveNotice)==1,'menu suspend saves Rally');emu:reset() end
 if f==1100 then
  check(emu:read8(gNativeRoster+2)==4 and emu:read16(gNativeParty)==1,'reset preserves deployed active Rally')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'reset preserves Rally budgets')
  emu:screenshot('@OUTPUT@/resumed.png');out:close()
 end
end)
