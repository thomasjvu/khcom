-- Read-only test of deliberate descent targeting across generated rooms.
local file=assert(io.open('tests/tactics_traversal_probe.lua'));local source=file:read('*a');file:close()
local block=assert(source:match(' local approach=nil\n if composedDescent.-\n end\n local ex,ey,ez=encounterGoal'))
block=block:gsub('\n local ex,ey,ez=encounterGoal$','')
local stair=nil
local env=setmetatable({composedDescent=true,composedRoutes=0,world=2,room=6,dx=1,dy=2,dz=3,composedStairApproach=function()return stair end},{__index=_G})
local tick=assert(load('return function()\n'..block..'\nreturn approach\nend','composed-goal','t',env))()
assert(tick()==nil and env.dx==1,'room without suitable stairs keeps normal door goal')
stair={x=10,y=20,z=30};assert(tick()==stair and env.dx==10 and env.dy==20 and env.dz==30,'later-world stairs can satisfy required native descent')
env.composedRoutes=1;env.dx=1;assert(tick()==nil and env.dx==1,'completed descent stops further detours')
env.composedRoutes=0;env.composedDescent=false;assert(tick()==nil and env.dx==1,'unrequested descent preserves ordinary traversal')
print('composed descent policy: later rooms/worlds, absent stairs, completed and unrequested coverage passed')
