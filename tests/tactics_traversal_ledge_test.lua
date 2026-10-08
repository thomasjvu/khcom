-- Execute the real replay busy controller with read-only actor-state fixtures.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local helper=assert(source:match('(local ledgeActive=false.-)local function replayFrame'))
local memory,keys,log={},0,{}
local function read(_,a) return memory[a] or 0 end
local env=setmetatable({emu={read16=read,read32=read,setKeys=function(_,v) keys=v end},gNativeBusy=100,gFieldState=200},{__index=_G})
local policy=assert(load([[local f,nextFrame,commands=100,0,0
local best,index,phase,terrainPlan=true,8,'scan',true
local out={write=function(_,s) log[#log+1]=s end,flush=function() end}
local function pos() return 1,2,3 end
]]..helper..[[
return {tick=busyInput,state=function() return nextFrame,commands,phase,index,best,terrainPlan end}
]],'ledge-policy','t',setmetatable({log=log},{__index=env})))()
memory[200]=1000;memory[1000+0x94]=2000;memory[2000]=3000;memory[3000+4]=4000
memory[100]=2;memory[4000+0x94]=8
assert(policy.tick() and keys==64,'held Up must reach native ledge catch')
local nextFrame,commands=policy.state();assert(nextFrame==104 and commands==1,'one ledge input sequence is counted')
memory[4000+0x94]=9;assert(policy.tick() and keys==64,'continue held Up at ledge hang')
assert(select(2,policy.state())==1,'do not count repeated held input as new commands')
memory[4000+0x94]=10;keys=0;assert(policy.tick() and keys==0 and policy.state()==108,'wait for original climb animation')
memory[100]=0;assert(policy.tick(),'settling resets replay planning')
local _,_,phase,index,best,terrainPlan=policy.state();assert(phase=='release' and index==1 and best==nil and terrainPlan==nil,'discard stale route after height change')
assert(not policy.tick(),'ordinary idle resumes traversal')
memory[100]=3;keys=0;assert(policy.tick() and keys==0,'other busy routes receive no ledge input')
assert(table.concat(log):find('LEDGE SETTLED',1,true),'record native settlement evidence')
print('ledge replay: held climb input, native animation wait, route invalidation and idle continuation passed')
