  local otherWork,otherCollider
  local taskNode=emu:read32(emu:read32(gFieldState)+0x80)
  while taskNode~=0 do
   local t=emu:read32(taskNode);local w=emu:read32(t+4)
   if emu:read32(t)==gTaskDescMapGmk00 and emu:read32(w+4)~=emu:read32(pillar+4) then
    local n=emu:read32(sColliderPoolObstacle+8)
    while n~=0 do
     local c=emu:read32(n)
     if (emu:read16(c+48)&1)~=0 and emu:read32(c+4)==emu:read32(w+4) and emu:read32(c+8)//2==emu:read32(w+8) then otherWork=w;otherCollider=c;break end
     n=emu:read32(n+8)
    end
   end
   if otherWork then break end
   taskNode=emu:read32(taskNode+8)
  end
  check(otherWork~=nil,'second original platform available for disclosed overlap fixture')
  if otherWork then
   emu:write32(otherWork+4,emu:read32(pillar+4));emu:write32(otherWork+8,emu:read32(pillar+8)//2);emu:write32(otherWork+12,emu:read32(pillar+12))
   emu:write32(otherCollider+4,emu:read32(pillar+4));emu:write32(otherCollider+8,emu:read32(pillar+8));emu:write32(otherCollider+12,emu:read32(pillar+12));emu:write32(otherCollider+20,emu:read32(pillar+20)-4096)
  end
