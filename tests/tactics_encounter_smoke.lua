local f=0
local out=io.open('@OUTPUT@/encounters.txt','w')
local initial={}
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function transition(room)
 local r=emu:read32(gMapRoomState)
 local p=emu:read32(gFieldState)
 emu:write8(r+15,room);emu:write8(r+16,1)
 emu:write32(p+0x70,emu:read32(p+0x70)|16)
end
local function enemy()
 local t=emu:read32(sEnemyTasks)
 return t~=0 and emu:read32(t+4) or 0
end
callbacks:add('frame',function()
 f=f+1
 if f==180 then
  local e=enemy();check(e~=0,'first generated encounter exists')
  for j=0,3 do initial[j]=emu:read32(e+8+j*4) end
  check(initial[0]%256==0 and initial[1]%256==0 and initial[2]%256==0,'enemy positions use exact whole-pixel fixed point')
  emu:write16(gNativeEnemyHp,4)
  transition(1)
 end
 if f==320 then
  check(emu:read8(gMapFloorState+6)==1,'entered adjacent generated room')
  emu:write16(gNativeEnemyHp,3)
  transition(0)
 end
 if f==460 then
  check(emu:read8(gMapFloorState+6)==0,'backtracked into original room')
  check(emu:read16(gNativeEnemyHp)==4,'partial enemy damage survives backtracking')
  local e=enemy();local same=e~=0
  for j=0,3 do same=same and emu:read32(e+8+j*4)==initial[j] end
  check(same,'exact enemy position survives backtracking')
  emu:setKeys(12)
 end
 if f==464 then emu:setKeys(0) end
 if f==520 then
  check(emu:read16(gNativeSaveNotice)==1,'all visited encounters fit verified native save')
  emu:reset()
 end
 if f==760 then
  check(emu:read8(gMapFloorState+6)==0 and emu:read16(gNativeEnemyHp)==4,'resume restores damaged current-room encounter')
  transition(1)
 end
 if f==900 then
  check(emu:read8(gMapFloorState+6)==1,'return to another saved room after reboot')
  check(emu:read16(gNativeEnemyHp)==3,'resume preserves partial damage in other visited rooms')
  emu:screenshot('@OUTPUT@/backtracking.png')
  out:close()
 end
end)
