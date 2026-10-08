local file=assert(io.open('tests/tactics_traversal_regions.lua'));local source=file:read('*a');file:close()
local route=assert(load(source..'\nreturn regionRoute'))()
local function edge(a,b,x,key) return {from=a,to=b,x=x,y=0,z=0,key=key} end
-- Distinct regions may have equal heights. Route through an intermediate
-- region instead of treating that equality as connectivity.
local graph={edges={edge(1,2,100,98),edge(2,1,100,144),edge(2,3,500,144),edge(3,2,500,98)}}
assert(route(graph,1,3,0,0).key==98)
assert(route(graph,2,3,100,0).key==144)
assert(route(graph,1,1,0,0)==nil)
assert(route(graph,1,4,0,0)==nil)
-- Direction matters: dropping from a tall ledge does not imply a possible jump.
graph={edges={edge(1,2,100,144)}}
assert(route(graph,1,2,0,0).key==144)
assert(route(graph,2,1,0,0)==nil)
-- Prefer fewer height transitions; among equally short paths, approach the
-- closest actual connector, rather than assuming one connector per height.
graph={edges={edge(1,2,900,98),edge(1,2,100,82),edge(2,3,400,144),edge(1,4,0,64),edge(4,5,0,64),edge(5,3,0,128)}}
assert(route(graph,1,3,0,0).key==82)
assert(route(graph,1,3,1000,0).key==98)
print('terrain region graph: disconnected equal-height regions, cycles, directed drops and connector choice passed')
