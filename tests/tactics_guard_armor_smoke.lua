local f=0
local assemblyHeld=false
local breakTurn=0
local effectWait=0
local out=io.open('@OUTPUT@/boss.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function place(distance,height)
 -- Explicit range/height fixture. Other enemies are placed outside reach.
 local p=emu:read32(gFieldState)
 local x=emu:read32(p+0x18);local y=emu:read32(p+0x1c)
 local z=emu:read32(p+0x20);local ground=emu:read32(p+0x24)
 for i=0,2 do
  emu:write32(sPartyPos+i*16,x);emu:write32(sPartyPos+i*16+4,y)
  emu:write32(sPartyPos+i*16+8,z);emu:write32(sPartyPos+i*16+12,ground)
 end
 for i=0,5 do
  local task=emu:read32(sEnemyTasks+i*4)
  if task~=0 then
   local e=emu:read32(task+4)
   emu:write32(e+8,x+(i==0 and distance*256 or 200*256))
   emu:write32(e+12,y+(i==0 and -height*256 or 0))
   emu:write32(e+16,z+(i==0 and height*256 or 0))
   emu:write32(e+20,ground)
  end
 end
 emu:write8(gNativeEnemyCharge,1)
end
callbacks:add('frame',function()
 f=f+1
 -- These encounter fixtures still deploy every room through native input.
 if emu:read16(gNativeAssembly)~=0 then emu:setKeys(1);assemblyHeld=true
 elseif assemblyHeld then emu:setKeys(0);assemblyHeld=false end
 if f==180 then
  local ready=emu:read32(sValueTiles)~=0 and emu:read32(sValuePalette)~=0
  for i=0,3 do ready=ready and emu:read32(sCardTiles+i*4)~=0 and emu:read32(sCardPalettes+i*4)~=0 end
  check(ready,'entry room allocates all original card art and value digits')
  local r=emu:read32(gMapRoomState);local p=emu:read32(gFieldState)
  emu:write8(r+15,7);emu:write8(r+16,1)
  emu:write32(p+0x70,emu:read32(p+0x70)|16)
 end
 if f==320 then
  check(emu:read8(gMapFloorState+6)==7,'native field enters Traverse Town boss room')
  check(emu:read16(gNativeBossReady)==1,'Guard Armor replaces the field elite presentation')
  out:write('ASSET failure '..emu:read16(gNativeBossAllocation)..'\n')
  local ready=emu:read32(sArmorPalette)~=0
  out:write('ASSET palette '..string.format('%08x',emu:read32(sArmorPalette))..'\n')
  for i=0,6 do
   local tile=emu:read32(sArmorTiles+i*4)
   out:write('ASSET part '..i..' '..string.format('%08x',tile)..'\n')
   ready=ready and tile~=0
  end
  check(ready,'all seven original Guard Armor components and palette allocate')
  local cards=emu:read32(sValueTiles)~=0 and emu:read32(sValuePalette)~=0
  for i=0,3 do cards=cards and emu:read32(sCardTiles+i*4)~=0 and emu:read32(sCardPalettes+i*4)~=0 end
  check(cards,'boss room retains all four card assets and value digits')
  emu:screenshot('@OUTPUT@/guard-armor.png')
  place(80,24)
 end
 if f==340 then
  check(emu:read16(gNativeBossPose)==1,'charged armor displays original crouch and raised-hand windup')
  check(emu:read16(gNativeThreats)==10,'slam preview includes exact 80-pixel range and 24-pixel height')
  emu:screenshot('@OUTPUT@/slam-preview.png');emu:setKeys(8)
 end
 if f==344 then emu:setKeys(0) end
 if f==350 then
  check(emu:read16(gNativeBossPose)==2,'resolved slam displays impact pose during enemy presentation')
  emu:screenshot('@OUTPUT@/slam-impact.png')
 end
 if f==450 then
  check(emu:read16(gNativeBossPose)==0,'impact presentation returns to original idle pose')
  check(emu:read8(gNativePartyHealth)==70,'Guard Armor slam resolves previewed Sora damage')
  check(emu:read8(gNativePartyHealth+1)==46 and emu:read8(gNativePartyHealth+2)==62,'slam damages each nearby party member once')
  check(emu:read8(gNativeEnemyCharge)==0,'slam consumes charge')
  place(81,24)
 end
 if f==470 then
  check(emu:read16(gNativeThreats)==0,'slam preview excludes 81-pixel range')
  emu:setKeys(8)
 end
 if f==474 then emu:setKeys(0) end
 if f==580 then
  check(emu:read8(gNativePartyHealth)==70,'out-of-range slam leaves Sora HP unchanged')
  place(80,25)
 end
 if f==600 then
  check(emu:read16(gNativeThreats)==0,'slam preview excludes 25-pixel height')
  emu:setKeys(8)
 end
 if f==604 then emu:setKeys(0) end
 if f==710 then
  check(emu:read8(gNativePartyHealth)==70,'out-of-height slam leaves Sora HP unchanged')
  place(64,0);emu:setKeys(12)
 end
 if f==714 then emu:setKeys(0) end
 if f==760 then
  check(emu:read16(gNativeSaveNotice)==1,'boss charge can be suspended')
  emu:reset()
 end
 if f==1100 then
  check(emu:read8(gMapFloorState+6)==7 and emu:read16(gNativeBossReady)==1,'suspend reconstructs original boss presentation')
  check(emu:read8(gNativeEnemyCharge)==1,'suspend preserves boss windup')
  check(emu:read16(gNativeBossPose)==1,'suspend reconstructs charged attack pose from saved windup')
  check(emu:read8(gNativePartyHealth)==70,'suspend preserves damage already resolved')
  emu:screenshot('@OUTPUT@/guard-armor-resume.png')
 end
 if f==1120 then
  emu:write16(gNativeEnemyHp,27);place(64,0);emu:setKeys(256)
 end
 if f==1124 then emu:setKeys(0) end
 if f==1140 then breakTurn=emu:read16(gNativeTurn);emu:setKeys(1) end
 if f==1144 then emu:setKeys(0) end
 if f==1150 then
  check(emu:read16(gNativeBossBreaks)==1 and emu:read16(gNativeBossEffects)==1,'native Fire breaking armor starts original spark effect')
  emu:screenshot('@OUTPUT@/armor-break-spark.png')
 end
 if f==1180 and emu:read16(gNativeBossEffects)~=0 and effectWait<180 then
  effectWait=effectWait+1;f=f-1;return
 end
 if f==1180 then
  check(emu:read16(gNativeBossEffects)==0,'original break spark expires while enemy AI remains frozen')
  check(emu:read16(gNativeTurn)==breakTurn,'visual spark updates do not advance authoritative enemy decisions')
  check(emu:read16(gNativeBossBreaks)==1,'break spark is emitted once per threshold crossing')
  check(emu:read16(gNativeEnemyHp)==15,'native Fire crosses the first armor-break threshold')
  check(emu:read16(gNativeBossPhase)==1,'card damage changes the rendered armor phase')
  emu:write16(gNativeEnemyHp,26);place(64,0)
 end
 if f==1200 then
  check(emu:read16(gNativeThreats)==8,'one-hand phase previews eight damage at 64 pixels')
  emu:screenshot('@OUTPUT@/one-hand.png');emu:setKeys(8)
 end
 if f==1204 then emu:setKeys(0) end
 if f==1310 then
  check(emu:read8(gNativePartyHealth)==62,'one-hand strike resolves eight damage')
  place(65,0)
 end
 if f==1330 then
  check(emu:read16(gNativeThreats)==0,'one-hand strike excludes 65 pixels')
  emu:write16(gNativeEnemyHp,13);place(48,0)
 end
 if f==1350 then
  check(emu:read16(gNativeBossPhase)==2,'13 HP breaks both original hand components')
  check(emu:read16(gNativeThreats)==6,'body phase previews six damage at 48 pixels')
  emu:screenshot('@OUTPUT@/body-phase.png');emu:setKeys(8)
 end
 if f==1354 then emu:setKeys(0) end
 if f==1460 then
  check(emu:read8(gNativePartyHealth)==56,'body strike resolves six damage')
  place(49,0)
 end
 if f==1480 then
  check(emu:read16(gNativeThreats)==0,'body strike excludes 49 pixels')
  place(48,0);emu:setKeys(12)
 end
 if f==1484 then emu:setKeys(0) end
 if f==1530 then
  check(emu:read16(gNativeSaveNotice)==1,'broken armor phase suspends')
  emu:reset()
 end
 if f==1850 then
  check(emu:read16(gNativeEnemyHp)==13 and emu:read16(gNativeBossPhase)==2,'resume derives broken armor phase from persisted HP')
  check(emu:read16(gNativeThreats)==6 and emu:read8(gNativePartyHealth)==56,'resume preserves body strike preview and resolved party damage')
  emu:screenshot('@OUTPUT@/body-phase-resume.png');out:close()
 end
end)
