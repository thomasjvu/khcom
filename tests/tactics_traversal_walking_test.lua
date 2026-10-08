local f=assert(io.open('tests/tactics_traversal_geometry.lua'));local s=f:read('*a');f:close()
local search=assert(load(s:sub(1,assert(s:find('-- Read%-only model'))-1)..'\nreturn walkingSearch'))()
local function gap(x,y)
 if x<0 or x>8192 or y<0 or y>4096 then return false end
 return math.abs(x-4096)>=512 or math.abs(y-1024)<512
end
local coarse,cd=search(0,0,8192,0,gap,4096)
local fine,fd=search(0,0,8192,0,gap,2048)
assert(cd>2048,'coarse lattice unexpectedly crossed narrow gap')
assert(fine and fd<=2048,'refined lattice missed reachable narrow gap')
local direction,distance=search(0,0,8192,0,function(x,y) return x>=0 and x<=8192 and y==0 end,4096)
assert(direction==16 and distance==0,'coarse open route changed')
local blocked,remaining=search(0,0,8192,0,function() return false end,2048)
assert(blocked==nil and remaining==8192,'blocked search invented a move')
print('walking planner: narrow gap refinement, open route and blocked route passed')
