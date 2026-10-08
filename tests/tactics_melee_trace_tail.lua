-- Read-only attack-box trace for diagnosing exact native strike reach.
local traceFrame=0
local trace=assert(io.open('@OUTPUT@/attack-trace.csv','w'))
trace:write('frame,busy,angle,active,attackX,attackY,attackZ,actorX,actorY,actorZ,enemyX,enemyY,enemyZ,enemyGround,enemyFlags,enemyHp\n')
callbacks:add('frame',function()
 traceFrame=traceFrame+1
 if ((traceFrame>=440 and traceFrame<=560) or (traceFrame>=920 and traceFrame<=1040)) and traceFrame%4==0 then
  local field=emu:read32(gFieldState);local room=emu:read32(gMapRoomState);local task=emu:read32(sEnemyTasks)
  local work=task~=0 and emu:read32(task+4) or 0
  trace:write(traceFrame..','..emu:read16(gNativeBusy)..','..emu:read8(field+0x2c)..','..emu:read8(room+0x20))
  for _,a in ipairs({room+0x24,room+0x28,room+0x2c,field+0x18,field+0x1c,field+0x20,work+8,work+12,work+16,work+20}) do trace:write(','..emu:read32(a)) end
  trace:write(','..emu:read16(work+4)..','..emu:read16(gNativeEnemyHp)..'\n');trace:flush()
 end
 if traceFrame==1100 then trace:close() end
end)
