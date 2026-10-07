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
  return sample(x,y) and sample(x,y-1536) and sample(x,y+1536)
 end
 local queue={{x=0,y=0,first=nil}}
 local seen={['0:0']=true}
 local head=1;local nearest=nil;local nearestScore=math.abs(ox-tx)+2*math.abs(oy-ty)
 local steps={{1,0,16},{-1,0,32},{0,-1,64},{0,1,128},{-1,-1,96},{1,-1,80},{-1,1,160},{1,1,144}}
 while head<=#queue and head<=8192 do
  local n=queue[head];head=head+1
  local x=ox+n.x*4096;local y=oy+n.y*2048
  local score=math.abs(x-tx)+2*math.abs(y-ty)
  if n.first and score<nearestScore then nearestScore=score;nearest=n.first end
  if nearestScore<=2048 then break end
  for _,step in ipairs(steps) do
   local nx=n.x+step[1];local ny=n.y+step[2]
   local key=nx..':'..ny
   if not seen[key] then
    seen[key]=true
    local ex=ox+nx*4096;local ey=oy+ny*2048
    if clear(ex,ey) and clear((x+ex)/2,(y+ey)/2) then
     queue[#queue+1]={x=nx,y=ny,first=n.first or step[3]}
    end
   end
  end
 end
 return nearest
end
