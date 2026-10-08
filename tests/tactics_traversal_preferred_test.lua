-- Exercise the real native-preview inspection block, not a copied policy.
local file=assert(io.open('tests/tactics_traversal_probe.lua'))
local source=file:read('*a');file:close()
local block=assert(source:match(" if phase=='inspect' then(.-)\n if phase=='cancel' then"))
local memory,keys={},nil
local function read(_,a) return memory[a] or 0 end
local env=setmetatable({emu={read16=read,read32=read,setKeys=function(_,v) keys=v end},
 gNativeRouteCost=2,gNativePreview=4,sCursorX=6,sCursorY=8,sRoutePos=1000},{__index=_G})
local prefix=[[local phase,index,best,nextFrame,planned,visits
local f=20;local dirs={16,32};local dx,dy,dz=4096,0,0
local function signed(v) return v end
local function cell(x,y,z) return x..':'..y..':'..z end
local function tick()
 if phase=='inspect' then]]
local suffix=[[
end
return {arm=function(p,v) phase='inspect';index=1;best=nil;planned=p;visits={['4096:0:0']=v} end,
 tick=tick,state=function() return phase,index,best,nextFrame end}]]
local policy=assert(load(prefix..block.."\n"..suffix,'preferred-preview','t',env))()
memory[2]=1;memory[4]=1
local address=1000+40*16;memory[address]=4096
policy.arm(16,0);policy.tick()
local phase,index,best,nextFrame=policy.state()
assert(phase=='confirm' and index==1 and best.dir==16 and keys==0 and nextFrame==24,'retain affordable preferred preview for confirmation')
policy.arm(16,2);policy.tick();assert(policy.state()=='cancel' and keys==2,'retain scan for repeatedly visited destination')
policy.arm(32,0);policy.tick();assert(policy.state()=='cancel' and keys==2,'retain scan for nonpreferred direction')
memory[2]=255;policy.arm(16,0);policy.tick();assert(policy.state()=='cancel' and keys==2,'reject unaffordable native preview')
memory[2]=1;memory[4]=0;policy.arm(16,0);policy.tick();assert(policy.state()=='cancel' and keys==2,'require actual native preview')
print('preferred preview: native validation, resource cost, visit guard and scan fallback passed')
