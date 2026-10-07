-- Card policy uses read-only native state and controller commands.
local file=assert(io.open('tests/tactics_traversal_probe.lua'));local source=file:read('*a');file:close()
local memory={};local function read(_,address) assert(address);return memory[address] or 0 end
local env=setmetatable({gNativeDeck=256,emu={read8=read},callbacks={add=function() end},io={open=function() return {write=function() end,flush=function() end} end}}, {__index=_G})
local choose=assert(load(source..'\nreturn chestCardKey','chest-policy','t',env))()
memory[256+72]=12
assert(choose()==768,'empty hand requests native reload')
memory[256+48]=1;memory[256]=1
memory[256+49]=1;memory[257]=0
assert(choose()==256,'cycle to Keyblade instead of casting Fire at a chest')
memory[256+73]=1;assert(choose()==1,'play selected Keyblade')
memory[256+49]=2;memory[256+73]=0
assert(choose()==1,'consume remaining non-Key cards so draw/reload can recover an attack')
memory[256+48]=2;assert(choose()==768,'depleted hand requests reload')
print('chest replay card policy: Keyblade selection, depletion and native reload passed')
