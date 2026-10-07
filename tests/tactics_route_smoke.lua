local f=0
local out=io.open('@OUTPUT@/routes.txt','w')
local directions={16,32,64,128}
local chosen=nil
local target=nil
local multi=nil
local back=nil
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function signed16(v) if v>=32768 then return v-65536 end return v end
callbacks:add('frame',function()
 f=f+1
 if f==170 then
  check(emu:read8(gNativePartyHealth)==80 and emu:read16(gGameState+0x32)==80,'fresh run starts Sora at full health')
 end
 if f>=180 and f<500 then
  local slot=math.floor((f-180)/80)+1
  local phase=(f-180)%80
  if phase==0 then emu:setKeys(512+directions[slot]) end
  if phase==4 then emu:setKeys(0) end
  if phase==24 then
   check(emu:read16(gNativePreview)==1,'L Dpad opens projected route cursor '..slot)
   check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'preview preserves budgets '..slot)
   if not chosen and emu:read16(gNativeRouteCost)==1 then chosen=directions[slot] end
   emu:setKeys(2)
  end
  if phase==28 then emu:setKeys(0) end
  if phase==50 then check(emu:read16(gNativePreview)==0,'B cancels route '..slot) end
 end
 if f==520 then
  check(chosen~=nil,'starting native room has a reachable walking destination')
  if chosen then emu:setKeys(512+chosen) end
 end
 if f==524 then emu:setKeys(0) end
 if f==550 and chosen then
  local x=signed16(emu:read16(sCursorX))
  local y=signed16(emu:read16(sCursorY))
  local address=sRoutePos+((y+4)*9+x+4)*16
  target={emu:read32(address),emu:read32(address+4),emu:read32(address+8)}
  emu:screenshot('@OUTPUT@/preview.png')
  check(emu:read16(gNativeRouteCost)==1,'preview reports exact one-step route cost')
  emu:setKeys(1)
 end
 if f==554 then emu:setKeys(0) end
 if f==570 and chosen then
  check(emu:read16(gNativeMoveLeft)==2,'A charges confirmed route once')
  check(emu:read16(gNativeActionLeft)==1,'route preserves combat action')
 end
 if f==800 then
  if chosen then
   local p=emu:read32(gFieldState)
   local dx=math.abs(emu:read32(p+0x18)-target[1])
   local dy=math.abs(emu:read32(p+0x1c)+emu:read32(p+0x20)-target[2]-target[3])
   check(emu:read16(gNativeBusy)==0,'native controller finishes route')
   check(dx<=768 and dy<=768,'native actor reaches previewed destination')
   check(emu:read16(gNativeMoveLeft)==2,'arrival does not double-charge movement')
   emu:screenshot('@OUTPUT@/arrived.png')
  end
 end
 if f==840 then emu:setKeys(8) end
 if f==844 then emu:setKeys(0) end
 if f>=960 and f<1280 then
  local slot=math.floor((f-960)/80)+1
  local phase=(f-960)%80
  if phase==0 then emu:setKeys(512+directions[slot]) end
  if phase==4 then emu:setKeys(0) end
  if phase==12 then emu:setKeys(directions[slot]) end
  if phase==16 then emu:setKeys(0) end
  if phase==24 then
   if not multi and emu:read16(gNativeRouteCost)==2 then multi=directions[slot] end
   emu:setKeys(2)
  end
  if phase==28 then emu:setKeys(0) end
 end
 if f==1310 then
  check(multi~=nil,'native room provides a two-segment route')
  if multi then emu:setKeys(512+multi) end
 end
 if f==1314 then emu:setKeys(0) end
 if f==1324 and multi then emu:setKeys(multi) end
 if f==1328 then emu:setKeys(0) end
 if f==1350 and multi then
  local x=signed16(emu:read16(sCursorX))
  local y=signed16(emu:read16(sCursorY))
  local address=sRoutePos+((y+4)*9+x+4)*16
  target={emu:read32(address),emu:read32(address+4),emu:read32(address+8)}
  check(emu:read16(gNativeRouteCost)==2,'preview reports two-segment movement cost')
  emu:screenshot('@OUTPUT@/two-step-preview.png');emu:setKeys(1)
 end
 if f==1354 then emu:setKeys(0) end
 if f==1600 then
  if multi then
   local p=emu:read32(gFieldState)
   local dx=math.abs(emu:read32(p+0x18)-target[1])
   local dy=math.abs(emu:read32(p+0x1c)+emu:read32(p+0x20)-target[2]-target[3])
   check(emu:read16(gNativeBusy)==0 and dx<=768 and dy<=768,'native controller follows both route segments')
   check(emu:read16(gNativeMoveLeft)==1,'two-segment route costs two movement points')
  end
 end
 if f==1640 and multi then
  back=multi==16 and 32 or multi==32 and 16 or multi==64 and 128 or 64
  emu:setKeys(512+back)
 end
 if f==1644 then emu:setKeys(0) end
 if f==1654 and back then emu:setKeys(back) end
 if f==1658 then emu:setKeys(0) end
 if f==1680 and back then
  check(emu:read16(gNativeRouteCost)==2,'return preview exposes route beyond remaining budget')
  emu:setKeys(1)
 end
 if f==1684 then emu:setKeys(0) end
 if f==1710 and back then
  check(emu:read16(gNativePreview)==1 and emu:read16(gNativeBusy)==0,'A rejects route exceeding movement budget')
  check(emu:read16(gNativeMoveLeft)==1,'rejected route preserves remaining movement')
  emu:setKeys(2)
 end
 if f==1714 then emu:setKeys(0) end
 if f==1750 then out:close() end
end)
