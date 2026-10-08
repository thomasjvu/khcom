-- Read-only driver policy: repeated obstruction must explore alternatives.
local f=assert(io.open('tests/tactics_traversal_probe.lua'));local s=f:read('*a');f:close()
local env=setmetatable({callbacks={add=function()end},io={open=function()return {write=function()end,flush=function()end}end}},{__index=_G})
local choose=assert(load(s..'\nreturn escapeJumpDirection','escape-policy','t',env))()
assert(choose(0,0,0,-16384,8192)==160,'first attempt approaches the connector')
local seen={[160]=true}
for i=2,8 do local key=choose(0,0,0,-16384,8192);assert(not seen[key],'same stuck origin repeated a direction before trying alternatives');seen[key]=true end
assert(choose(0,0,0,-16384,8192)==160,'after exploring each direction return to best approach')
assert(choose(32768,0,0,0,16384)==160,'new origin has independent attempts')
assert(choose(0,0,8192,-16384,8192)==160,'different floor has independent attempts')
print('escape replay: connector preference, eight alternatives, repeat cycle and independent origin/height passed')
