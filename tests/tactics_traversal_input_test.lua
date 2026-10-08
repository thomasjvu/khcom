local file=assert(io.open('tests/tactics_traversal_input.lua'))
local source=file:read('*a');file:close()
local memory={[1]=0,[2]=10};local keys
local env=setmetatable({f=0,sRawKeys=1,gFrameCounter=2,emu={
 read16=function(_,a)return memory[a] end,read32=function(_,a)return memory[a] end,
 setKeys=function(_,k)keys=k end}},{__index=_G})
local press,wait=assert(load(source..'\nreturn pressNative,waitNativeInput','native-input','t',env))()
press(1);assert(keys==1 and wait(),'press must wait for native sampling')
env.f=40;assert(wait() and keys==1,'video frames alone must not release unconsumed input')
memory[1]=1;assert(wait(),'sampled press must wait for update completion')
memory[2]=11;assert(not wait(),'completed native update must acknowledge press')
press(1);assert(keys==0,'repeated key must first release the native edge')
assert(wait());memory[1]=0;assert(wait());memory[2]=12
assert(wait() and keys==1,'completed release must deliver queued press')
memory[1]=1;assert(wait());memory[2]=13;assert(not wait(),'queued press must complete its own update')
press(2);env.f=401;assert(not pcall(wait),'unresponsive native input must fail within a bounded wait')
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local driver=file:read('*a');file:close()
assert(driver:find('pressNative(12);suspendStage=1',1,true),'save request must wait for completed native encoding and SRAM verification')
print('native input: delayed sampling, update completion, repeated edges and bounded timeout passed')
