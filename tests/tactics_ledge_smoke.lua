-- Explicit position fixture; cell geometry is original and is never rewritten.
local out=io.open('@OUTPUT@/ledge.txt','w')
local f=0
local target=nil
local prop=nil
local geometryClear=nil
local enemyFlags={}
local playerCollider=nil
local otherProps={}
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function signed(v) if v>=2147483648 then return v-4294967296 end return v end
local function find()
 local r=emu:read32(gMapRoomState);local cols=emu:read16(r+4);local rows=emu:read16(r+6)
 local cells=emu:read32(sMapCells)
 local function clear(x,y,z)
  for _,dy in ipairs({-1536,0,1536}) do
   local yy=y+dy;local cx=math.floor(x/8192);local cy=math.floor(yy/4096)
   if cx<0 or cy<0 or cx>=cols or cy>=rows then return false end
   local a=cells+(cy*cols+cx)*32
   local upper=signed(emu:read32(a+8));local lower=signed(emu:read32(a+12));local kind=emu:read8(a+2)
   local px=math.floor(x/256)%32;local py=math.floor(yy/256)%16
   local block=emu:read8(emu:read32(a+16)+math.floor(px/8)+math.floor(py/8)*4)
   local mask=(emu:read8(gCellMasks+block*8+py%8)>>(7-px%8))&1
   local ground
   if upper<z then ground=(kind==4 or kind==6) and (mask~=0 and upper or lower) or upper
   else ground=(kind==3 or kind==5) and (mask~=0 and lower or upper) or lower end
   if ground~=z or (not (upper>=z and lower~=1048576) and mask~=0) then return false end
  end
  return true
 end
 geometryClear=clear
 local steps={{4096,0,16},{-4096,0,32},{0,2048,128},{0,-2048,64}}
 for cy=0,rows-1 do for cx=0,cols-1 do
  local a=cells+(cy*cols+cx)*32;local z=signed(emu:read32(a+8))
  if signed(emu:read32(a+12))==1048576 and z~=-1048576 and z~=1048576 then
   for py=2,14,2 do for px=2,30,2 do
    local x=(cx*32+px)*256;local y=(cy*16+py)*256
    if clear(x,y,z) then for _,step in ipairs(steps) do
     local sx=x-step[1];local sy=y-step[2];local good=true
     for i=0,3 do if not clear(sx+step[1]*i/4,sy+step[2]*i/4,z) then good=false end end
     if good then return {x=x,y=y,z=z,sx=sx,sy=sy,key=step[3]} end
    end end
   end end
  end
 end end
end
callbacks:add('frame',function()
 f=f+1
 if not propsOnly and f>450 then return end
 if f==200 then
  target=find();if propsOnly then target=nil;return end;check(target~=nil,'generated room has a walkable upper ledge above void')
  if target then
   local p=emu:read32(gFieldState)
   emu:write32(p+0x18,target.sx);emu:write32(p+0x1c,target.sy-target.z)
   emu:write32(p+0x20,target.z);emu:write32(p+0x24,target.z)
  end
 end
 if f==240 and target then emu:setKeys(512+target.key) end
 if f==244 then emu:setKeys(0) end
 if f==270 and target then
  check(emu:read16(gNativePreview)==1,'upper ledge route preview opens')
  check(emu:read16(gNativeRouteCost)==1,'upper ledge above void is a legal one-point route')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'ledge preview preserves budgets')
  emu:screenshot('@OUTPUT@/preview.png');emu:setKeys(1)
 end
 if f==274 then emu:setKeys(0) end
 if f==450 and not propsOnly then
  if target then
   local p=emu:read32(gFieldState);local x=signed(emu:read32(p+0x18));local z=signed(emu:read32(p+0x20));local y=signed(emu:read32(p+0x1c))+z
   check(emu:read16(gNativeBusy)==0,'upper ledge native movement completes')
   check(math.abs(x-target.x)<=768 and math.abs(y-target.y)<=768 and z==target.z,'actor reaches upper ledge without falling into void')
   check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'upper ledge costs one move and no combat action')
   emu:screenshot('@OUTPUT@/arrived.png')
  end
  out:close()
 end
 if f==800 and propsOnly then emu:setKeys(8) end
 if f==804 then emu:setKeys(0) end
 if f==900 and propsOnly and geometryClear then
  local node=emu:read32(sColliderPoolObstacle+8)
  local steps={{8192,0,32},{-8192,0,16},{0,4096,64},{0,-4096,128}}
  while node~=0 and not prop do
   local c=emu:read32(node);local radius=emu:read32(c+16)
   local x=signed(emu:read32(c+4));local y=signed(emu:read32(c+8))/2;local z=signed(emu:read32(c+12))
   if (emu:read16(node+12)&2)==0 and radius>=2048 and radius<=3072 and emu:read32(c+20)>=2048 then
    for _,step in ipairs(steps) do
     local good=true
     for i=0,8 do
      local xx=x+step[1]*i/8;local yy=y+step[2]*i/8
      if not geometryClear(xx,yy,z)  then good=false end
     end
     if good then prop={node=node,flags=emu:read16(node+12),x=x,y=y,z=z,sx=x+step[1],sy=y+step[2],key=step[3],height=emu:read32(c+20)};break end
    end
   end
   node=emu:read32(node+8)
  end
  check(prop~=nil,'generated room supplies a solid prop route fixture')
  if prop then
   local n=emu:read32(sColliderPoolObstacle+8)
   while n~=0 do local c=emu:read32(n);if n~=prop.node then otherProps[#otherProps+1]={node=n,height=emu:read32(c+20),flags=emu:read16(n+12)} end;n=emu:read32(n+8) end
   local p=emu:read32(gFieldState);local task=emu:read32(emu:read32(p+0x94));local w=emu:read32(task+4)
   playerCollider={work=w,flags=emu:read16(w+0x5c)}
   emu:write32(w+0x94,0);emu:write16(w+0x98,0);emu:write32(w+0xa0,0);emu:write32(task+0x20,task_fld_sora_1|1)
   emu:write32(p+0x28,0);emu:write8(w+0x64,0);emu:write16(w+0x66,0);emu:write32(w+0x70,0);emu:write32(w+0x74,0)
   for i=0,5 do local t=emu:read32(sEnemyTasks+i*4);if t~=0 then local a=emu:read32(t+4)+0x6c;enemyFlags[#enemyFlags+1]={a,emu:read16(a)};emu:write16(a,emu:read16(a)|2) end end
   emu:write32(p+0x18,prop.sx);emu:write32(p+0x1c,prop.sy-prop.z);emu:write32(p+0x20,prop.z);emu:write32(p+0x24,prop.z)
  end
 end
 if f==920 and prop then emu:setKeys(512+prop.key) end
 if f==924 or f==938 or f==964 or f==994 or f==1064 or f==1078 or f==1104 then emu:setKeys(0) end
 if f==934 and prop then emu:setKeys(prop.key) end
 if f==960 and prop then
  out:write('PREVIEW cost='..emu:read16(gNativeRouteCost)..' move='..emu:read16(gNativeMoveLeft)..' action='..emu:read16(gNativeActionLeft)..'\n');out:flush()
  emu:screenshot('@OUTPUT@/prop-preview.png')
  check(emu:read16(gNativeRouteCost)==65535,'route rejects destination inside solid original prop')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'blocked prop preview preserves resources')
  emu:setKeys(1)
 end
 if f==990 and prop then
  check(emu:read16(gNativePreview)==1 and emu:read16(gNativeBusy)==0,'A cannot commit a route through solid prop')
  check(emu:read16(gNativeMoveLeft)==3,'rejected prop route spends no movement')
 end
 if f>=900 and f<=1300 and playerCollider then
  -- Isolate route geometry from contact pushes during explicit placement.
  for _,r in ipairs(otherProps) do emu:write16(r.node+12,r.flags|2);emu:write32(emu:read32(r.node)+20,0) end
  local w=playerCollider.work
  emu:write16(w+0x5c,playerCollider.flags|2);emu:write8(w+0x64,0);emu:write16(w+0x66,0)
 end
 if f>=1040 and f<=1300 and prop then
  -- Native prop tasks may re-enable their collider each update. Hold the
  -- explicit removal fixture through route construction and execution.
  emu:write16(prop.node+12,prop.flags|2)
  emu:write32(emu:read32(prop.node)+20,0)
 end
 if f==1100 and prop then
  check(emu:read16(gNativeRouteCost)==2,'open preview updates when the solid prop is removed')
  emu:setKeys(1)
 end
 if f==1300 then
  if prop then
   local p=emu:read32(gFieldState);local x=signed(emu:read32(p+0x18));local z=signed(emu:read32(p+0x20));local y=signed(emu:read32(p+0x1c))+z
   check(emu:read16(gNativeBusy)==0 and math.abs(x-prop.x)<=768 and math.abs(y-prop.y)<=768,'native actor crosses after prop collider is disabled')
   check(emu:read16(gNativeMoveLeft)==1 and emu:read16(gNativeActionLeft)==1,'unblocked prop route charges exact two-point cost')
   emu:write16(prop.node+12,prop.flags)
   emu:write32(emu:read32(prop.node)+20,prop.height)
   for _,r in ipairs(otherProps) do emu:write16(r.node+12,r.flags);emu:write32(emu:read32(r.node)+20,r.height) end
   emu:write16(playerCollider.work+0x5c,playerCollider.flags)
   for _,record in ipairs(enemyFlags) do emu:write16(record[1],record[2]) end
  end
  out:close()
 end
end)
