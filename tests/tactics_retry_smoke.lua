-- Explicit terminal/progression fixture; actual retry uses native Select input.
local f=0
local seed
local out=io.open('@OUTPUT@/retry.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  seed=emu:read32(gNativeSeed)
  check(emu:read16(gNativeAssembly)==1,'fresh run enters native assembly')
  emu:setKeys(1)
 end
 if f==184 then emu:setKeys(0) end
 if f==220 then
  check(emu:read16(gNativeAssembly)==0,'native A begins the first run')
  emu:write16(gNativeResult,2);emu:write16(gNativeFloor,3)
  emu:write8(gNativeRoster,15);emu:write8(gNativeRoster+4,8);emu:write8(gNativeRoster+8,7)
  emu:write16(gNativeKills,4);emu:write16(gNativeChests,7)
  emu:write16(gGameState+0x32,20);emu:write8(gNativePartyHealth,20)
 end
 if f==240 then emu:setKeys(4) end
 if f==252 then emu:setKeys(0) end
 if f==400 then
  check(emu:read16(gNativeResult)==0 and emu:read16(gNativeFloor)==0,'native retry returns to live first world')
  check(emu:read32(gNativeSeed)==((seed+0x9e3779b9)&0xffffffff),'native retry advances the procedural seed exactly once')
  check(emu:read16(gNativeKills)==0 and emu:read16(gNativeChests)==0,'new run resets kill and chest progression')
  check(emu:read8(gNativeRoster)==7 and emu:read8(gNativeRoster+1)==0 and emu:read8(gNativeRoster+2)==1 and emu:read8(gNativeRoster+3)==2,'new run resets recruitment and restores starter deployment')
  local clean=true
  for i=4,11 do clean=clean and emu:read8(gNativeRoster+i)==0 end
  check(clean,'new run clears personal powers and sleight ownership')
  check(emu:read8(gNativePartyHealth)==80 and emu:read8(gNativePartyHealth+1)==56 and emu:read8(gNativePartyHealth+2)==72,'new run restores all starter health pools')
  check(emu:read16(gNativeParty)==0 and emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'new run resets active Sora turn resources')
  local fresh=emu:read8(gNativeDeck+72)==12 and emu:read8(gNativeDeck+77)==0
  for i=0,11 do fresh=fresh and emu:read8(gNativeDeck+48+i)==(i<5 and 1 or 0) end
  check(fresh,'new run reconstructs the starter deck and clears card exhaustion')
  check(emu:read8(gMapFloorState+6)==0 and emu:read16(gNativeAssembly)==1,'new procedural entry room opens party assembly')
  emu:screenshot('@OUTPUT@/new-seed-assembly.png');out:close()
 end
end)
