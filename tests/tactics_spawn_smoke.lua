-- Explicit position fixture; Fire is selected and played through native input.
local f=0
local out=io.open('@OUTPUT@/spawn.txt','w')
local slot=nil
local target=nil
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local aligned=true;local count=0
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then
    local w=emu:read32(t+4);count=count+1
    aligned=aligned and emu:read32(w+16)==emu:read32(w+20)
    if emu:read8(gNativeEnemyKind+i)==1 then slot=i;target=w
    else emu:write32(w+8,0);emu:write32(w+12,0);emu:write32(w+16,0) end
   end
  end
  check(count==2,'native entry encounter contains both seeded actors')
  check(aligned,'all tactical actors spawn on their assigned floor instead of above the ceiling')
  check(target~=nil,'seeded encounter includes an original Red Nocturne')
  if target then
   local p=emu:read32(gFieldState)
   emu:write32(p+0x18,emu:read32(target+8));emu:write32(p+0x1c,emu:read32(target+12)+2048)
   emu:write32(p+0x20,emu:read32(target+20));emu:write32(p+0x24,emu:read32(target+20));emu:write8(p+0x2c,0)
  end
 end
 if f==200 then emu:setKeys(256) end
 if f==204 then emu:setKeys(0) end
 if f==240 then emu:setKeys(1) end
 if f==244 then emu:setKeys(0) end
 if f==400 then
  check(slot and emu:read32(sEnemyTasks+slot*4)==0,'a real Fire card defeats the fresh ranged actor from its floor')
  check(emu:read16(gNativeKills)==1,'Fire awards exactly one native kill')
  check(emu:read16(gNativeActionLeft)==0,'Fire targeting consumes the normal combat action')
  emu:screenshot('@OUTPUT@/fire.png');out:close()
 end
end)
