-- Test the input-only driver's party-turn policy against a read-only memory mock.
local file=assert(io.open('tests/tactics_traversal_probe.lua'));local source=file:read('*a');file:close()
local memory={};local function read(_,a) assert(a~=nil);return memory[a] or 0 end
local env=setmetatable({emu={read8=read,read16=read,read32=read},callbacks={add=function() end},io={open=function() return {write=function() end,flush=function() end} end}}, {__index=_G})
local names={'gFieldState','gNativeEnemyFrames','gNativeParty','gNativeGuard','gNativeThreats','sPartyAction','gNativeDeck','gNativePartyHealth','gNativeActionLeft','gNativeRoster'}
for i,name in ipairs(names) do env[name]=i*256 end
memory[env.gFieldState]=4096;memory[4096+0x94]=8192;memory[8192]=12288;memory[12288+4]=16384;memory[16384+0x94]=0
memory[env.gNativeDeck+72]=1;memory[env.gNativeDeck]=3;memory[env.gNativeDeck+48]=1
memory[env.gNativeRoster+1]=0;memory[env.gNativeRoster+2]=1;memory[env.gNativeRoster+3]=2
memory[env.gNativeThreats]=10;memory[env.gNativePartyHealth+2]=72;memory[env.sPartyAction+4]=1;memory[env.gNativeActionLeft]=1
local turn=assert(load(source..'\nreturn turnKey','party-policy','t',env))()
assert(turn()==516,'select Donald while cycling to Goofy')
memory[env.gNativeParty]=1;assert(turn()==516,'select Goofy')
memory[env.gNativeParty]=2;assert(turn()==1,'play an available Guard in hand slot zero')
memory[env.gNativeGuard]=2;assert(turn()==8,'end turn after Goofy guard commits')
memory[env.gNativeEnemyFrames]=24;assert(turn()==0,'wait for the actual enemy phase')
memory[env.gNativeEnemyFrames]=0;assert(turn()==516,'cycle back to Sora after enemy phase')
memory[env.gNativeParty]=0;assert(turn()==0,'resume Sora navigation')
memory[env.gNativeGuard]=0;memory[env.gNativePartyHealth+2]=0;assert(turn()==8,'skip unavailable Goofy')
-- Consume the return-to-Sora state before considering a new turn.
assert(turn()==0)
memory[env.gNativePartyHealth+2]=72;memory[16384+0x94]=6;assert(turn()==8,'retain native attached-stair turn input')
memory[16384+0x94]=0;assert(turn()==0)
memory[env.gNativeRoster+2]=2;memory[env.gNativeRoster+3]=4
memory[env.gNativePartyHealth+1]=72;memory[env.sPartyAction+2]=1
assert(turn()==516,'select actual Goofy after swapping into slot one')
memory[env.gNativeParty]=1;assert(turn()==1,'swapped Goofy casts Guard')
memory[env.gNativeGuard]=2;assert(turn()==8);memory[env.gNativeParty]=0;assert(turn()==0)
memory[env.gNativeGuard]=0;memory[env.gNativeRoster+2]=3
assert(turn()==8,'do not mistake Rally or Cloud for Goofy')
print('party replay policy: Goofy Guard, phase wait, Sora return, unavailable member and attached stairs passed')
