-- Read-only hierarchical terrain planner. Regions are connected standing
-- surfaces, not height numbers: two platforms at one height can be disconnected.
local terrainPlan=nil
local function regionRoute(graph, source, target, x, y)
 if source==target then return nil end
 local queue={target};local distance={[target]=0};local head=1
 while head<=#queue do
  local node=queue[head];head=head+1
  for _,edge in ipairs(graph.edges) do
   if edge.to==node and distance[edge.from]==nil then
    distance[edge.from]=distance[node]+1;queue[#queue+1]=edge.from
   end
  end
 end
 local best=nil
 for _,edge in ipairs(graph.edges) do
  if edge.from==source and distance[edge.to] and distance[source] and distance[edge.to]==distance[source]-1 then
   local score=math.abs(edge.x-x)+2*math.abs(edge.y-y)
   if not best or score<best.score then best={x=edge.x,y=edge.y,z=edge.z,key=edge.key,score=score} end
  end
 end
 return best
end
local function buildTerrainPlan()
 local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
 local cells=emu:read32(sMapCells);local heights={};local walls={}
 for cy=0,rows-1 do for cx=0,cols-1 do
  local a=cells+(cy*cols+cx)*32
  local u=signed(emu:read32(a+8));local l=signed(emu:read32(a+12));local k=emu:read8(a+2)
  if math.abs(u)<1048576 then heights[u]=true end
  if math.abs(l)<1048576 then heights[l]=true end
  if k>=3 and k<=6 and math.abs(u)<1048576 and math.abs(l)<1048576 and l>u then
   walls[#walls+1]={cx=cx,cy=cy,u=u,l=l,k=k,stairs=(emu:read16(a)&32)~=0}
  end
 end end
 local w=cols*4;local h=rows*4;local layers={};local regions=0
 local function sample(x,y,z)
  local cx=math.floor(x/8192);local cy=math.floor(y/4096)
  if cx<0 or cy<0 or cx>=cols or cy>=rows then return false end
  local a=cells+(cy*cols+cx)*32;local u=signed(emu:read32(a+8));local l=signed(emu:read32(a+12));local k=emu:read8(a+2)
  local px=math.floor(x/256)%32;local py=math.floor(y/256)%16
  local m=emu:read32(a+16);local b=emu:read8(m+math.floor(px/8)+math.floor(py/8)*4)
  local mask=(emu:read8(gCellMasks+b*8+py%8)>>(7-px%8))&1
  local ground
  if u<z then ground=(k==4 or k==6) and (mask~=0 and u or l) or u
  else ground=(k==3 or k==5) and (mask~=0 and l or u) or l end
  return ground==z and ((u>=z and l~=1048576) or mask==0)
 end
 for z in pairs(heights) do
  local layer={};layers[z]=layer
  for gy=0,h-1 do for gx=0,w-1 do
   local x=(gx*8+4)*256;local y=(gy*4+2)*256
   if sample(x,y,z) and sample(x,y-1536,z) and sample(x,y+1536,z) then layer[gy*w+gx]=0 end
  end end
  for gy=0,h-1 do for gx=0,w-1 do
   local index=gy*w+gx
   if layer[index]==0 then
    regions=regions+1;layer[index]=regions;local queue={index};local head=1
    while head<=#queue do
     local n=queue[head];head=head+1;local nx=n%w;local ny=math.floor(n/w)
     for _,d in ipairs({{-1,0},{1,0},{0,-1},{0,1}}) do
      local ex=nx+d[1];local ey=ny+d[2];local next=ey*w+ex
      if ex>=0 and ex<w and ey>=0 and ey<h and layer[next]==0 then
       layer[next]=regions;queue[#queue+1]=next
      end
     end
    end
   end
  end end
 end
 local graph={edges={}}
 function graph.region(x,y,z)
  local layer=layers[z];if not layer then return nil end
  local gx=math.floor(x/2048);local gy=math.floor(y/1024);local best=nil;local score=nil
  for dy=-4,4 do for dx=-3,3 do
   local nx=gx+dx;local ny=gy+dy
   if nx>=0 and nx<w and ny>=0 and ny<h then
    local region=layer[ny*w+nx];local distance=math.abs((nx*8+4)*256-x)+2*math.abs((ny*4+2)*256-y)
    if region and (not score or distance<score) then best=region;score=distance end
   end
  end end
  return best
 end
 for _,top in ipairs(walls) do if top.k==3 or top.k==5 then
  local base=nil
  for _,b in ipairs(walls) do
   if b.cx==top.cx and b.cy>top.cy and b.u==top.u and b.l==top.l and b.k==top.k+1 and (not base or b.cy<base.cy) then base=b end
  end
  if base then
   local ux=(top.cx*32+16)*256;local uy=(top.cy*16+(top.stairs and 2 or -4))*256
   local lx=(base.cx*32+16)*256;local ly=(base.cy*16+14)*256
   local upper=graph.region(ux,uy,top.u);local lower=graph.region(lx,ly,top.l)
   if upper and lower and upper~=lower then
    graph.edges[#graph.edges+1]={from=upper,to=lower,x=ux,y=uy,z=top.u,key=top.stairs and 128 or (top.k==5 and 160 or 144)}
    -- Native full-height jumps cover low walls; taller ascent requires stairs
    -- or the game's jump-pad controller and is left to its existing fallback.
    if base.stairs or top.l-top.u<=12288 then
     graph.edges[#graph.edges+1]={from=lower,to=upper,x=lx,y=ly,z=top.l,key=base.stairs and 64 or (base.k==6 and 82 or 98)}
    end
   end
  end
 end end
 -- Original launch pads supply the tall ascent connections omitted by
 -- ordinary jump edges. The native collider/controller still owns launch.
 local node=emu:read32(emu:read32(gFieldState)+0x80)
 while node~=0 do
  local task=emu:read32(node)
  if emu:read32(task)==gTaskDescMapGmkJump then
   local work=emu:read32(task+4)
   local x=signed(emu:read32(work));local z=signed(emu:read32(work+8))
   local y=signed(emu:read32(work+4))+z;local height=emu:read32(work+0xc4)
   local lower=graph.region(x,y,z);local upper=graph.region(x,y-height,z-height)
   if height>0 and lower and upper and lower~=upper then
    graph.edges[#graph.edges+1]={from=lower,to=upper,x=x,y=y,z=z,key=2}
   end
  end
  node=emu:read32(node+8)
 end
 return graph
end
local function terrainGoal(tx,ty,tz)
 if not terrainPlan then terrainPlan=buildTerrainPlan() end
 local x,y=pos();local z=signed(emu:read32(emu:read32(gFieldState)+0x24))
 local source=terrainPlan.region(x,y,z);local target=terrainPlan.region(tx,ty,tz)
 if not source or not target then return nil end
 return regionRoute(terrainPlan,source,target,x,y)
end
