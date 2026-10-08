-- Explicit saved approach; jump and ledge-menu operations use native input.
local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function state()
 local p=emu:read32(gFieldState);local task=emu:read32(emu:read32(p+0x94))
 return emu:read32(emu:read32(task+4)+0x94)
end
local phase='wait'
local nextFrame=260
local done=false
callbacks:add('frame',function()
 f=f+1
 if done then return end
 if f>=180 and f<220 and f%8==4 and
    (emu:read16(gNativeAssembly)~=0 or emu:read16(gNativeProgressReward)~=0) then emu:setKeys(1) end
 if f>=184 and f<=216 and f%8==0 or f==224 then emu:setKeys(0) end
 if f==220 then emu:setKeys(98) end
 if f<nextFrame then return end
 emu:setKeys(0)
 if f>2000 then out:write('TIMEOUT state='..state()..' busy='..emu:read16(gNativeBusy)..' z='..emu:read32(emu:read32(gFieldState)+0x20)..'\n');check(false,'bounded ledge menu scenario completes');emu:screenshot('@OUTPUT@/timeout.png');out:close();done=true;return end
 if phase=='wait' then
  if state()~=9 then nextFrame=f+4;return end
  check(true,'native jump reaches hanging ledge state');emu:setKeys(4);phase='open'
 elseif phase=='open' then
  check(emu:read16(gNativeMenu)==9 and state()==9,'Select opens ledge commands without climbing')
  emu:setKeys(128);phase='drop'
 elseif phase=='drop' then
  check(emu:read16(gNativeMenuChoice)==1 and state()==9,'menu Down highlights Drop without dropping')
  emu:screenshot('@OUTPUT@/ledge-menu.png');emu:setKeys(2);phase='cancel'
 elseif phase=='cancel' then
  check(emu:read16(gNativeMenu)==0 and state()==9,'B cancels ledge menu without dropping')
  emu:setKeys(4);phase='reopen'
 elseif phase=='reopen' then
  check(emu:read16(gNativeMenu)==9 and emu:read16(gNativeMenuChoice)==0,'reopening defaults to Climb')
  if dropTest then emu:setKeys(128);phase='readyDrop' else emu:setKeys(1);phase='climb' end
 elseif phase=='readyDrop' then
  emu:setKeys(1);phase='climb';nextFrame=f+4;return
 elseif phase=='climb' then
  check(emu:read16(gNativeMenu)==0 and emu:read16(gNativeBusy)==2,'confirm begins original ledge action')
  phase='settle'
 elseif phase=='settle' then
  if state()~=0 or emu:read16(gNativeBusy)~=0 then nextFrame=f+4;return end
  check(emu:read32(emu:read32(gFieldState)+0x20)==(dropTest and 16384 or 0),'menu-confirmed action settles on expected surface')
  check(emu:read16(gNativeMoveLeft)==2 and emu:read16(gNativeActionLeft)==0,'ledge menu preserves already-paid jump costs')
  out:close();done=true;emu:screenshot('@OUTPUT@/climbed.png')
 end
 nextFrame=f+20
end)
