local f=0
local attached=false
local completed=false
local commands=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local trace=assert(io.open('@OUTPUT@/motion.csv','w'))
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 local field=emu:read32(gFieldState)
 if field~=0 and f>100 then
  local task=emu:read32(emu:read32(field+0x94))
  local work=emu:read32(task+4)
  local state=emu:read32(work+0x94)
  local z=emu:read32(field+0x20)
  if f%4==0 then trace:write(f..','..state..','..z..','..emu:read32(field+0x24)..','..emu:read16(gNativeBusy)..','..emu:read16(gNativeMoveLeft)..'\n');trace:flush() end
  if state==6 then attached=true end
  if attached and state==0 then completed=true end
  if f%24==0 and not completed and emu:read16(gNativeBusy)==0 and emu:read16(gNativeEnemyFrames)==0 then
   if emu:read16(gNativeMoveLeft)==0 then emu:setKeys(8)
   elseif state==6 then
    local bottom=emu:read32(field+0x24)
    emu:setKeys(20480>=bottom and 128 or 64);commands=commands+1
   end
  end
  if f%24==4 then emu:setKeys(0) end
  if f==1200 then
   check(attached,'saved position enters original native stair controller')
   check(completed and state==0,'native input reaches connected stair endpoint without reversing bands')
   check(commands>0 and z==emu:read32(field+0x24),'endpoint settles exactly on original supporting floor')
   out:close();trace:close();emu:screenshot('@OUTPUT@/landed.png')
  end
 end
end)
