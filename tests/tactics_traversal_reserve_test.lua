-- Test the real replay's read-only reserve selection policy.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local memory={}
local function read(_,address) assert(address~=nil);return memory[address] or 0 end
local env=setmetatable({gNativeRoster=256,gNativePartyHealth=512,heroCount=6,rosterHpOffset=21,
 emu={read8=read,read16=read,read32=read},callbacks={add=function() end},
 io={open=function() return {write=function() end,flush=function() end} end}}, {__index=_G})
local choose=assert(load(source..'\nreturn reserveReplacement','reserve-policy','t',env))()
memory[256]=31;memory[257]=0;memory[258]=3;memory[259]=2
memory[512]=70;memory[513]=0;memory[514]=0
memory[278]=56;memory[281]=64
local slot,hero=choose();assert(slot==1 and hero==4,'replace KO Cloud with healthiest unlocked reserve Rally')
memory[256]=15;slot,hero=choose();assert(slot==1 and hero==1,'exclude locked Rally')
memory[278]=0;assert(choose()==nil,'do not revive KO reserves')
memory[256]=31;memory[258]=4;memory[513]=64
slot,hero=choose();assert(slot==nil,'do not redeploy a healthy hero into another slot')
memory[278]=56;slot,hero=choose();assert(slot==2 and hero==1,'replace second KO with available Donald')
memory[514]=72;assert(choose()==nil,'healthy party needs no replacement')
print('reserve replay policy: hero HP, unlocked reserves, KO slots and duplicate exclusion passed')
