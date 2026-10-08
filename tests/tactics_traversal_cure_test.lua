-- Read-only policy mock: the driver must target the recipient it wants to heal.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local memory={}
local function read(_,a) assert(a~=nil);return memory[a] or 0 end
local env=setmetatable({emu={read8=read,read16=read,read32=read},
 callbacks={add=function() end},io={open=function() return {write=function() end,flush=function() end} end}}, {__index=_G})
local names={'gFieldState','sEnemyTasks','gNativeActionLeft','gNativeDeck','gNativePartyHealth','gNativeCureTarget','gNativeThreats','sPartyPos','gNativeParty','gNativeEnemyHp','gNativeFloor','gNativeRoster','gNativeFireTarget','gNativeFireDamage'}
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

-- A finishing Fire must be both eligible and confirmed by native forecast.
memory[env.sEnemyTasks]=8192;memory[env.gNativeEnemyHp]=12
memory[env.gNativePartyHealth]=40;memory[env.gNativeDeck+72]=2
memory[env.gNativeDeck]=2;memory[env.gNativeDeck+1]=1
memory[env.gNativeDeck+24+1]=6;memory[env.gNativeDeck+48]=1;memory[env.gNativeDeck+49]=1
memory[env.gNativeDeck+73]=0;memory[env.gNativeFireTarget]=0;memory[env.gNativeFireDamage]=12
assert(combat()==256,'select finishing Fire before healing against last enemy')
memory[env.gNativeDeck+73]=1;assert(combat()==1,'confirmed finishing Fire removes last enemy before defense')
memory[env.gNativeFireDamage]=11;assert(combat()==256,'unconfirmed damage falls back to selecting Cure')
memory[env.gNativeFireDamage]=12;memory[env.gNativeDeck+25]=1
assert(combat()==256,'card broken by native floor threshold cannot be a finishing attack')
memory[env.gNativeDeck+25]=6;memory[12288+8]=32768
assert(combat()==256,'exact 128 pixel Fire boundary is excluded')
memory[12288+8]=4096;memory[12288+16]=8192
assert(combat()==256,'out of height Fire falls back to Cure')
memory[12288+16]=0;memory[env.sEnemyTasks+4]=8192;memory[env.gNativeEnemyHp+2]=12
assert(combat()==1,'confirmed finishing hit remains valid within an enemy group')
memory[env.gNativeEnemyHp+2]=20;memory[env.gNativeFireTarget]=1
assert(combat()==258,'cycle native Fire target toward the weaker eligible enemy')
memory[env.gNativeFireTarget]=0
assert(combat()==1,'play only after the native preview confirms a finishing target')
print('finishing Fire policy: native forecast, group targets, range, height and break gates passed')
