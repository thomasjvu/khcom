-- Construct an old format-8 ceiling position, then save/reset through input.
local f=0
local slot=nil
local original={}
local out=io.open('@OUTPUT@/legacy-spawn.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 and emu:read8(gNativeEnemyKind+i)==1 then
    slot=i;local w=emu:read32(t+4)
    for j=0,3 do original[j]=emu:read32(w+8+j*4) end
    emu:write32(w+16,0xffff6000);emu:write16(gNativeEnemyHp+i*2,4)
   end
  end
  check(slot~=nil,'legacy fixture finds the seeded ranged actor')
  emu:setKeys(12)
 end
 if f==184 then emu:setKeys(0) end
 if f==260 then check(emu:read16(gNativeSaveNotice)==1,'legacy ceiling encounter writes a verified format-8 suspend');emu:reset() end
 if f==520 then
  local t=slot and emu:read32(sEnemyTasks+slot*4) or 0
  check(t~=0,'legacy ranged actor survives native suspend/reset')
  if t~=0 then
   local w=emu:read32(t+4)
   check(emu:read32(w+16)==emu:read32(w+20),'resume repairs only the old fixed ceiling entrance to its saved floor')
   check(emu:read32(w+8)==original[0] and emu:read32(w+12)==original[1] and emu:read32(w+20)==original[3],'legacy repair preserves horizontal position and standing surface')
   check(emu:read16(gNativeEnemyHp+slot*2)==4,'legacy repair preserves partial enemy damage')
  end
  out:close()
 end
end)
