-- Explicit saved-room approach; actual jump and ledge climb use native input.
local f,nextFrame,commands=0,0,0
local phase,index,best,terrainPlan='scan',1,nil,nil
local seenLedge=false
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function pos()
 local p=emu:read32(gFieldState)
 return emu:read32(p+0x18),emu:read32(p+0x1c)+emu:read32(p+0x20),emu:read32(p+0x20)
end
-- @BUSY@
callbacks:add('frame',function()
 f=f+1
 if f>=180 and f<220 and f%8==4 and
    (emu:read16(gNativeAssembly)~=0 or emu:read16(gNativeProgressReward)~=0) then emu:setKeys(1) end
 if f>=184 and f<=216 and f%8==0 or f==224 then emu:setKeys(0) end
 if f==220 then emu:setKeys(98) end
 if f>=260 and f<600 and f%4==0 then
  local p=emu:read32(gFieldState);local task=emu:read32(emu:read32(p+0x94));local state=emu:read32(emu:read32(task+4)+0x94)
  if state==8 or state==9 then seenLedge=true end
  emu:setKeys(0);busyInput()
 end
 if f==600 then
  local p=emu:read32(gFieldState);local task=emu:read32(emu:read32(p+0x94));local state=emu:read32(emu:read32(task+4)+0x94)
  check(seenLedge,'native jump catches the recorded ledge')
  check(commands==1,'real replay helper issues one held climb sequence')
  check(state==0 and emu:read16(gNativeBusy)==0,'original ledge controller settles grounded and idle')
  check(emu:read32(p+0x20)==0,'climb reaches the upper standing surface')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'ledge climb preserves committed jump costs')
  emu:screenshot('@OUTPUT@/climbed.png');out:close()
 end
end)
