local f=0
local phase=0
local nextFrame=180
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if phase==4 or f<nextFrame then return end
 emu:setKeys(0)
 if f>2000 then check(false,'bounded chest fixture timeout');out:close();phase=4;return end
 if emu:read16(gNativeAssembly)~=0 or emu:read16(gNativeProgressReward)~=0 then
  emu:setKeys(1);nextFrame=f+4;return
 end
 if emu:read16(gNativeBusy)~=0 then nextFrame=f+4;return end
 if phase==0 then
  emu:setKeys(320);phase=1;nextFrame=f+4
 elseif phase==1 then phase=2;nextFrame=f+40
 elseif phase==2 then
  local p=emu:read32(gFieldState)
  check(emu:read8(p+0x2c)==0,'recorded chest approach faces north after R Up')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'facing at chest preserves budgets')
  emu:screenshot('@OUTPUT@/chest-facing.png');emu:setKeys(1);phase=3;nextFrame=f+4
 elseif phase==3 then
  if emu:read16(gNativeChests)==0 then nextFrame=f+4;return end
  check(emu:read16(gNativeChests)==1 and emu:read16(gNativeReward)~=0,'native melee opens recorded chest and offers reward')
  check(emu:read16(gNativeActionLeft)==0,'opening chest spends one native attack action')
  emu:screenshot('@OUTPUT@/opened.png');out:close();phase=4
 end
end)
