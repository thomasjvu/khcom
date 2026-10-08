local file=assert(io.open('tests/tactics_traversal_geometry.lua'))
local source=file:read('*a');file:close()
local helper=source:sub(1,assert(source:find('-- Return a direction',1,true))-1)
local direction=assert(load(helper..'\nreturn directApproachDirection'))()
assert(direction(40612,92784,8192,45056,95744,8192)==144,'recorded approach must identify its desired diagonal')
assert(direction(0,0,0,4096,0,0)==16,'horizontal alignment must not add downward input')
assert(direction(0,0,0,0,-2048,0)==64,'vertical alignment must not add rightward input')
assert(direction(0,0,0,0,0,0)==nil,'at-target input must not invent movement')
assert(direction(0,0,0,8192,0,0)==nil,'distant target must use normal route planning')
assert(direction(0,0,0,0,0,2048)==nil,'height transition must use terrain planning')
file=assert(io.open('tests/tactics_traversal_probe.lua'));source=file:read('*a');file:close()
local block=assert(source:match(" if not ex and not approach and phase=='scan'.-\n end\n local stair"))
block=block:gsub('\n local stair$','')
local pressed=nil;local planned=16
local env=setmetatable({x0=40612,y0=92784,z0=8192,dx=45056,dy=95744,dz=8192,
 phase='scan',f=20,approachDebugFrame=0,commands=0,index=2,
 directApproachDirection=direction,walkingDirection=function()return planned end,
 pressNative=function(k)pressed=k end},{__index=_G})
local tick=assert(load('return function()\n'..block..'\nend','approach-input','t',env))()
tick();assert(pressed==nil and env.phase=='scan' and env.commands==0,'blocked diagonal must fall through to ordinary planning')
planned=144;tick();assert(pressed==144 and env.phase=='release' and env.commands==1,'clear direct approach must use acknowledged input')
env.phase='scan';planned=nil;pressed=nil;tick()
assert(pressed==nil and env.phase=='scan' and env.commands==1,'missing walking route must not bypass collision checks')
print('approach: recorded diagonal, axis alignment, bounds, height and planner agreement passed')
