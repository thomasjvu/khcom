local f=0
local out=io.open('@OUTPUT@/diagonals.txt','w')
local dirs={80,96,144,160}
local chosen=nil
local target=nil
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function signed(v) if v>=2147483648 then return v-4294967296 end return v end
callbacks:add('frame',function()
 f=f+1
 if f>=180 and f<500 then
  local slot=math.floor((f-180)/80)+1;local phase=(f-180)%80
  if phase==0 then emu:setKeys(512+dirs[slot]) end
  if phase==4 then emu:setKeys(0) end
  if phase==24 then
   check(emu:read16(gNativePreview)==1,'diagonal cursor opens '..slot)
   check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1,'diagonal preview preserves budgets '..slot)
   if not chosen and emu:read16(gNativeRouteCost)==1 then chosen=dirs[slot] end
   emu:setKeys(2)
  end
  if phase==28 then emu:setKeys(0) end
  if phase==50 then check(emu:read16(gNativePreview)==0,'diagonal preview cancels '..slot) end
 end
 if f==520 then check(chosen~=nil,'spawn has a legal projected diagonal');if chosen then emu:setKeys(512+chosen) end end
 if f==524 then emu:setKeys(0) end
 if f==550 and chosen then
  local x=emu:read16(sCursorX);if x>=32768 then x=x-65536 end
  local y=emu:read16(sCursorY);if y>=32768 then y=y-65536 end
  local a=sRoutePos+((y+4)*9+x+4)*16
  target={signed(emu:read32(a)),signed(emu:read32(a+4))+signed(emu:read32(a+8))}
  check(x~=0 and y~=0,'confirmed target changes both projected coordinates')
  check(emu:read16(gNativeRouteCost)==1,'diagonal costs one movement segment')
  emu:screenshot('@OUTPUT@/preview.png');emu:setKeys(1)
 end
 if f==554 then emu:setKeys(0) end
 if f==800 then
  if chosen then
   local p=emu:read32(gFieldState)
   local x=signed(emu:read32(p+0x18));local y=signed(emu:read32(p+0x1c))+signed(emu:read32(p+0x20))
   check(emu:read16(gNativeBusy)==0,'diagonal native controller completes')
   check(math.abs(x-target[1])<=768 and math.abs(y-target[2])<=768,'actor reaches diagonal preview')
   check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==1,'diagonal arrival charges one move and no action')
   emu:screenshot('@OUTPUT@/arrived.png')
  end
  out:close()
 end
end)
