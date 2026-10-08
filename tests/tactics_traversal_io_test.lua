-- Reproduce a denied diagnostic snapshot without changing emulator memory.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local callback,keys=nil,nil
local log={}
local opened=0
local env=setmetatable({
 callbacks={add=function(_,_,fn)callback=fn end},
 emu={setKeys=function(_,v)keys=v end},
 io={open=function()
  opened=opened+1
  if opened==1 then return {write=function(_,v)log[#log+1]=v end,flush=function()end} end
  return nil,'Too many open files'
 end}
},{__index=_G})
assert(load(source..[[
replayFrame=function() navigationSnapshot('navigation-periodic-turns.json') end
]],'io-reproduction','t',env))()
callback()
assert(keys==0,'release native inputs when diagnostic IO fails')
local message=table.concat(log)
assert(message:find('ERROR frame=0',1,true),'record explicit driver error')
assert(message:find('navigation-periodic-turns.json',1,true),'retain failing artifact path')
assert(message:find('Too many open files',1,true),'retain filesystem reason')
assert(not message:find('nil value',1,true),'avoid hiding IO failure behind nil indexing')
callback();assert(opened==2,'terminal driver does not continue gameplay after failure')
print('snapshot IO failure: explicit path/reason, released input and terminal driver passed')
