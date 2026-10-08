-- Read-only policy mock: the driver must target the recipient it wants to heal.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local memory={}
local function read(_,a) assert(a~=nil);return memory[a] or 0 end
local env=setmetatable({emu={read8=read,read16=read,read32=read},
 callbacks={add=function() end},io={open=function() return {write=function() end,flush=function() end} end}}, {__index=_G})
local names={'gFieldState','sEnemyTasks','gNativeActionLeft','gNativeDeck','gNativePartyHealth','gNativeCureTarget','gNativeThreats','sPartyPos'}
for i,name in ipairs(names) do env[name]=i*256 end
memory[env.gFieldState]=4096
memory[env.sEnemyTasks]=8192;memory[8192+4]=12288
memory[12288+8]=4096 -- Enemy is 16 pixels from the zero-position actor.
memory[env.gNativeActionLeft]=1;memory[env.gNativePartyHealth]=40
memory[env.gNativeDeck+72]=1;memory[env.gNativeDeck]=2;memory[env.gNativeDeck+48]=1
local combat=assert(load(source..'\nreturn combatInput','cure-policy','t',env))()
memory[env.gNativeCureTarget]=1
assert(combat()==258,'cycle Cure off Donald when Sora needs recovery')
memory[env.gNativeCureTarget]=2
assert(combat()==258,'continue cycling Cure off Goofy')
memory[env.gNativeCureTarget]=0
assert(combat()==1,'play Cure only after Sora is selected')
memory[env.gNativeDeck+73]=1
assert(combat()==256,'select the Cure card before cycling its target')
memory[env.gNativeActionLeft]=0
assert(combat()==nil,'spent action cannot attempt Cure targeting')
memory[env.gNativeActionLeft]=1;memory[env.gNativePartyHealth]=12;memory[env.gNativeThreats]=14
memory[env.gNativeDeck+72]=2;memory[env.gNativeDeck]=1;memory[env.gNativeDeck+1]=3
memory[env.gNativeDeck+49]=1;memory[env.gNativeDeck+73]=0
assert(combat()==256,'select Guard instead of Fire near lethal incoming damage')
memory[env.gNativeDeck+73]=1
assert(combat()==1,'commit selected defensive Guard with native A input')
memory[env.gNativeDeck]=2;memory[env.gNativeCureTarget]=0
assert(combat()==256,'available Cure has priority over defensive Guard')
memory[env.gNativeDeck]=1;memory[env.gNativePartyHealth]=80;memory[env.gNativeDeck+73]=0
assert(combat()==1,'healthy Sora retains Fire preference')
memory[env.sEnemyTasks]=0;memory[env.gNativePartyHealth]=40;memory[env.gNativeDeck]=2;memory[env.gNativeCureTarget]=0
assert(combat()==1,'heal injured Sora even between distant encounters')
memory[env.gNativeDeck]=1;assert(combat()==nil,'do not waste offensive cards without nearby enemies')
print('Combat replay policy: explicit Sora Cure, defensive Guard, and action/card gates passed')

-- Healthy leader can rescue a nearby companion using actual native range.
memory[env.gNativePartyHealth]=80;memory[env.gNativePartyHealth+1]=10;memory[env.gNativePartyHealth+4]=56
memory[env.gNativeDeck]=2;memory[env.gNativeCureTarget]=0
memory[env.sPartyPos+16]=2048
assert(combat()==258,'cycle Cure toward injured nearby companion')
memory[env.gNativeCureTarget]=1;assert(combat()==1,'heal injured Donald while Sora is healthy')
memory[env.gNativePartyHealth+2]=0;memory[env.gNativePartyHealth+5]=72
memory[env.gNativeCureTarget]=2;assert(combat()==1,'revive nearby knocked-out Goofy')
memory[env.sPartyPos+32+8]=8192
assert(combat()==258,'exclude recipient beyond native height reach')
memory[env.sPartyPos+16]=65536;memory[env.gNativeCureTarget]=1
assert(combat()==nil,'no attempted Cure at out-of-range companions without enemies')
print('Combat replay recovery: nearby injured/KO companions, native range and height gates passed')
