-- Read-only controller-policy mock; execute the real replay callback.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local memory,keys,log={},nil,{}
local function read(_,a) assert(a~=nil);return memory[a] or 0 end
local env=setmetatable({emu={read8=read,read16=read,read32=read,setKeys=function(_,v) keys=v end},
 callbacks={add=function() end},io={open=function() return {
 write=function(_,s) log[#log+1]=s end,flush=function() end} end}}, {__index=_G})
local names={'gMapFloorState','gNativeFloor','gNativeResult','gCurrentMode','gCurrentModeUpdate','gNativeSeed','gNativeRoster'}
for i,name in ipairs(names) do env[name]=i*256 end
local seed=0xfffffff0
local nextSeed=(seed+0x9e3779b9)&0xffffffff
env.initialRosterMask=23;env.rosterBytes=39
env.sNativeMode=0x9000;env.NativeUpdate=0xa000
local policy=assert(load(source..[[
local failure=nil
finish=function(ok,why) failure=why;done=true end
return {
 arm=function(seed) f=200;done=false;failure=nil;retryFrame=212;retrySeed=seed end,
 tick=replayFrame,
 state=function() return retryFrame,failure end
}
]],'retry-policy','t',env))()
local function terminal()
 memory[env.gNativeFloor]=3;memory[env.gNativeResult]=2
 memory[env.gCurrentMode]=env.sNativeMode;memory[env.gCurrentModeUpdate]=env.NativeUpdate|1
 memory[env.gNativeSeed]=seed;memory[env.gNativeRoster]=15
end
local function restarted()
 memory[env.gNativeFloor]=0;memory[env.gNativeResult]=0
 memory[env.gNativeSeed]=nextSeed;memory[env.gNativeRoster]=23
end
terminal();policy.arm(seed)
for _=1,11 do policy.tick();assert(keys==4,'hold Select until its release frame') end
policy.tick();assert(keys==0 and policy.state()==212,'release Select while waiting for real initialization')
restarted();policy.tick();assert(policy.state()==nil,'accept wrapped seed advancement and reset roster')
assert(table.concat(log):find('RETRY VERIFIED',1,true),'record verified native retry')
terminal();policy.arm(seed);restarted();memory[env.gNativeSeed]=seed
for _=1,12 do policy.tick() end
local _,failure=policy.state();assert(failure and failure:find('advance seed',1,true),'reject unchanged seed')
terminal();policy.arm(seed);restarted();memory[env.gNativeRoster]=15
for _=1,12 do policy.tick() end
_,failure=policy.state();assert(failure and failure:find('reset the recruited roster',1,true),'reject stale recruitment')
terminal();policy.arm(seed);restarted();memory[env.gCurrentMode]=0
for _=1,12 do policy.tick() end
assert(policy.state()==212,'wait while native mode is still initializing')
memory[env.gCurrentMode]=env.sNativeMode;policy.tick();assert(policy.state()==nil,'continue after actual native-mode readiness')
terminal();policy.arm(seed)
for _=1,193 do policy.tick() end
_,failure=policy.state();assert(failure and failure:find('did not initialize',1,true),'bound retry initialization wait')
print('retry replay policy: held/released input, wrapped seed, roster reset, native-mode readiness and timeout passed')
