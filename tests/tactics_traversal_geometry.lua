-- A final approach still needs a clear walking route. Proximity alone does
-- not mean a diagonal crosses a wall or prop corner safely.
local function directApproachDirection(x,y,z,tx,ty,tz)
 if math.abs(x-tx)>=8192 or math.abs(y-ty)>=4096 or math.abs(z-tz)>=2048 then return nil end
 local key=0
 if math.abs(tx-x)>512 then key=key|(tx<x and 32 or 16) end
 if math.abs(ty-y)>512 then key=key|(ty<y and 64 or 128) end
 return key~=0 and key or nil
end
-- Return a direction and remaining distance; native previews retain authority.
local function walkingSearch(ox,oy,tx,ty,clear,stride)
 local queue={{x=0,y=0,first=nil}}
 local seen={['0:0']=true}
 local head=1;local nearest=nil;local nearestScore=math.abs(ox-tx)+2*math.abs(oy-ty)
 local steps={{1,0,16},{-1,0,32},{0,-1,64},{0,1,128},{-1,-1,96},{1,-1,80},{-1,1,160},{1,1,144}}
 while head<=#queue and head<=8192 do
  local n=queue[head];head=head+1
  local x=ox+n.x*stride;local y=oy+n.y*(stride/2)
  local score=math.abs(x-tx)+2*math.abs(y-ty)
  if n.first and score<nearestScore then nearestScore=score;nearest=n.first end
  if nearestScore<=2048 then break end
  for _,step in ipairs(steps) do
   local nx=n.x+step[1];local ny=n.y+step[2]
   local key=nx..':'..ny
   if not seen[key] then
    local ex=ox+nx*stride;local ey=oy+ny*(stride/2)
    if clear(ex,ey) and clear((x+ex)/2,(y+ey)/2) then
     seen[key]=true
     queue[#queue+1]={x=nx,y=ny,first=n.first or step[3]}
    end
   end
  end
 end
 return nearest,nearestScore
end
-- Read-only model of original FieldGroundAt/IsFldPosBlocked for the replay.
-- The native route controller still checks and executes every chosen move.
local function walkingDirection(tx,ty)
 local ox,oy,z=pos()
 local r=emu:read32(gMapRoomState)
 local cols=emu:read16(r+4);local rows=emu:read16(r+6)
 local cells=emu:read32(sMapCells)
 local function sample(x,y)
  local cx=math.floor(x/8192);local cy=math.floor(y/4096)
  if cx<0 or cy<0 or cx>=cols or cy>=rows then return nil end
  local a=cells+(cy*cols+cx)*32
  local upper=signed(emu:read32(a+8));local lower=signed(emu:read32(a+12))
  local kind=emu:read8(a+2)
  local px=math.floor(x/256)%32;local py=math.floor(y/256)%16
  local maskTable=emu:read32(a+16)
  local block=emu:read8(maskTable+math.floor(px/8)+math.floor(py/8)*4)
  local mask=(emu:read8(gCellMasks+block*8+py%8)>>(7-px%8))&1
  local ground
  if upper<z then
   if kind==4 or kind==6 then ground=mask~=0 and upper or lower else ground=upper end
  else
   if kind==3 or kind==5 then ground=mask~=0 and lower or upper else ground=lower end
  end
  local blocked=not (upper>=z and lower~=1048576) and mask~=0
  return ground==z and not blocked
 end
 local function clear(x,y)
  local node=emu:read32(sColliderPoolObstacle+8)
  while node~=0 do
   if (emu:read16(node+12)&2)==0 then
    local c=emu:read32(node);local radius=emu:read32(c+16)+1024
    local dx=math.abs(x-signed(emu:read32(c+4)))
    local dy=math.abs((y-z)*2-signed(emu:read32(c+8)))
    local dz=z-signed(emu:read32(c+12))
    if dx<radius and dy<radius and dz<8192 and -dz<emu:read32(c+20) and dx*dx+dy*dy<radius*radius then return false end
   end
   node=emu:read32(node+8)
  end
  return sample(x,y) and sample(x,y-1536) and sample(x,y+1536)
 end
 local direction,distance=walkingSearch(ox,oy,tx,ty,clear,4096)
 -- A coarse lattice can miss a narrow passage. Refine only unresolved goals,
 -- and retain the coarse answer unless the finer route gets closer.
 if distance>2048 then
  local refined,remaining=walkingSearch(ox,oy,tx,ty,clear,2048)
  if refined and remaining<distance then direction=refined end
 end
 return direction
end
