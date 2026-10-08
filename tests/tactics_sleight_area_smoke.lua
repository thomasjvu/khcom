-- Explicit two-enemy boundary fixtures; stock and resolution use native input.
local f=0
local out=io.open('@OUTPUT@/sleight-area.txt','w')
local enemies={}
local x,y,z=0,0,0
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function place(slot,dx,dz)
 local w=enemies[slot];if not w then return end
 emu:write32(w+8,x+dx*256);emu:write32(w+12,y);emu:write32(w+16,z+dz*256);emu:write32(w+20,z)
end
callbacks:add('frame',function()
 f=f+1
 if f==140 then emu:setKeys(1) end
 if f==144 then emu:setKeys(0) end
 if f==180 or f==220 or f==260 then emu:setKeys(513) end
 if f==184 or f==224 or f==264 then emu:setKeys(0) end
 if f==300 then
  local p=emu:read32(gFieldState);x=emu:read32(p+0x18);y=emu:read32(p+0x1c);z=emu:read32(p+0x20)
  for i=0,5 do
   local t=emu:read32(sEnemyTasks+i*4)
   if t~=0 then enemies[i]=emu:read32(t+4);emu:write16(gNativeEnemyHp+i*2,40);place(i,256,0) end
  end
  check(enemies[0]~=nil and enemies[1]~=nil,'two original enemies available for area fixture')
  -- Three Fire values of six produce 32 enhanced damage without Donald bonus.
  for i=0,2 do emu:write8(gNativeDeck+i,1);emu:write8(gNativeDeck+24+i,6) end
  place(0,144,24);place(1,145,0)
 end
 if f==320 then
  check(emu:read16(gNativeSleightDamage)==32,'Fire sleight includes exact 144-pixel range and 24-pixel height boundary')
  check(emu:read16(gNativeSleightDamage+2)==0,'Fire sleight excludes enemy beyond range')
  place(1,64,25)
 end
 if f==340 then
  check(emu:read16(gNativeSleightDamage+2)==0,'Fire sleight excludes enemy above height limit')
  for i=0,2 do emu:write8(gNativeDeck+i,0) end
  place(0,64,24);place(1,65,0)
 end
 if f==360 then
  check(emu:read16(gNativeSleightDamage)==32,'melee sleight includes exact 64-pixel range boundary')
  check(emu:read16(gNativeSleightDamage+2)==0,'melee sleight excludes enemy beyond its shorter area')
  for i=0,2 do emu:write8(gNativeDeck+i,1) end
  place(0,32,0);place(1,96,0)
 end
 if f==380 then
  check(emu:read16(gNativeSleightDamage)==32 and emu:read16(gNativeSleightDamage+2)==32,'Fire previews damage on both eligible enemies')
  check(emu:read16(gNativeActionLeft)==1 and emu:read8(gNativeDeck+77)==3,'area inspection preserves stocked cards and action')
  emu:screenshot('@OUTPUT@/multiple.png');emu:setKeys(1)
 end
 if f==384 then emu:setKeys(0) end
 if f==420 then
  check(emu:read16(gNativeEnemyHp)==8 and emu:read16(gNativeEnemyHp+2)==8,'native Fire sleight applies previewed damage to both enemies')
  check(emu:read16(gNativeActionLeft)==0 and emu:read8(gNativeDeck+77)==0,'area sleight consumes exactly one action and clears stock')
  check(emu:read16(gNativeSleightDamage)==0 and emu:read16(gNativeSleightDamage+2)==0,'resolved sleight clears area preview')
  out:close()
 end
end)
