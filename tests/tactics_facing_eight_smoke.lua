local f=0
local out=io.open('@OUTPUT@/checks.txt','w')
local start=nil
local cases={{64,0},{128,128},{16,64},{32,192},{80,45},{96,211},{144,83},{160,173}}
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(8) end
 if f==184 then emu:setKeys(0) end
 if f==240 then local p=emu:read32(gFieldState);start={emu:read32(p+0x18),emu:read32(p+0x1c),emu:read32(p+0x20)} end
 for i,c in ipairs(cases) do
  local frame=260+(i-1)*80
  if f==frame then emu:setKeys(256+c[1]) end
  if f==frame+4 then emu:setKeys(0) end
  if f==frame+40 then
   local p=emu:read32(gFieldState)
   check(emu:read8(p+0x2c)==c[2],'native facing direction '..c[2]..' persists after release')
   if i==1 then emu:screenshot('@OUTPUT@/north.png') end
  end
 end
 if f==880 then
  local p=emu:read32(gFieldState)
  check(emu:read32(p+0x18)==start[1] and emu:read32(p+0x1c)==start[2] and emu:read32(p+0x20)==start[3] and emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==1 and emu:read16(gNativeBusy)==0,'turn-in-place preserves position movement action and idle state');out:close()
 end
end)
