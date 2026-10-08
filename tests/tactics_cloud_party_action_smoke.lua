-- Explicit unlock/position fixture; native assembly/card/attack resolution.
local f=0
local out=assert(io.open('@OUTPUT@/checks.txt','w'))
local function check(v,s)out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush()end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:write8(gNativeRoster,31);emu:setKeys(16)end
 if f==184 or f==204 or f==244 or f==284 or f==344 then emu:setKeys(0)end
 if f==200 or f==240 then emu:setKeys(128)end
 if f==280 then
  check(emu:read8(gNativeRoster+2)==3,'native setup selects unlocked Cloud')
  emu:screenshot('@OUTPUT@/setup.png');emu:setKeys(8)
 end
 if f==310 then
  check(emu:read16(gNativeParty)==1,'native deployment controls Cloud')
  local p=emu:read32(gFieldState)
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then
    local w=emu:read32(t+4)
    for j=0,3 do emu:write32(w+8+j*4,emu:read32(p+0x18+j*4))end
    emu:write32(w+8,emu:read32(p+0x18)+(i==0 and 8192 or 40000));emu:write16(gNativeEnemyHp+i*2,30)
   end
  end
 end
 if f==340 then
  check(emu:read16(gNativeFireDamage)==17,'Cloud Key card previews sword damage at ranged target')
  emu:setKeys(1)
 end
 if f==355 then
  check(emu:read16(gNativeFriendPose)==1,'Cloud slash uses authored action pose')
  check((emu:read16(sFriends+8)&1)==0,'Cloud sword animation plays once')
  emu:screenshot('@OUTPUT@/slash.png')
 end
 if f==650 then
  check(emu:read16(gNativeEnemyHp)==13,'native Cloud slash resolves one sword hit')
  check(emu:read16(gNativeEnemyHp+2)==30,'Cloud slash preserves other enemy HP')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==0,'Cloud slash spends one action without movement')
  check(emu:read16(gNativeFriendPose)==0,'single Cloud slash recovers to idle')
  out:close()
 end
end)
