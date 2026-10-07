local f=0
local reviveHp=0
local cureX=0
local cureZ=0
local out=io.open('@OUTPUT@/party.txt','w')
local function check(v,s) out:write((v and 'PASS ' or 'FAIL ')..s..'\n');out:flush() end
local function hp() return emu:read16(gGameState+0x32) end
callbacks:add('frame',function()
 f=f+1
 if f==180 then emu:setKeys(4) end
 if f==184 then emu:setKeys(0) end
 if f==210 then check(emu:read16(gNativeParty)==1,'Select controls Donald');emu:setKeys(64) end
 if f==214 then emu:setKeys(0) end
 if f==222 then check(emu:read16(gNativeFriendPose)==2,'Donald uses original walking animation during actual travel');emu:screenshot('@OUTPUT@/donald-walk.png') end
 if f==280 then check(emu:read16(gNativeFriendPose)==0,'Donald returns to idle after movement') end
 if f==290 then check(emu:read16(gNativeMoveLeft)==2,'Donald has his own movement budget');emu:setKeys(256) end
 if f==294 then emu:setKeys(0) end
 if f==310 then emu:setKeys(256) end
 if f==314 then emu:setKeys(0) end
 if f==330 then
  check(emu:read8(gNativeDeck+73)==2,'R selects Cure in the real hand')
  emu:write16(gGameState+0x32,40)
 end
 if f==332 then check(emu:read16(gNativeCureHeal)==16,'Cure recovery preview caps Donald healing at missing HP');check(emu:read16(gNativeCureTarget)==1,'Cure preview selects injured Donald before play');emu:screenshot('@OUTPUT@/cure-target.png');emu:setKeys(1) end
 if f==334 then emu:setKeys(0) end
 if f==370 then
  check(hp()==56,'Donald Cure has healing bonus')
  check(emu:read16(gNativeCureHeal)==0,'spent Cure clears recovery preview')
  check(emu:read16(gNativeActionLeft)==0,'Donald card costs his action')
  check(emu:read8(gNativeDeck+48+2)==2,'played Cure enters discard')
  emu:setKeys(4)
 end
 if f==374 then emu:setKeys(0) end
 if f==400 then
  check(emu:read16(gNativeParty)==2,'Select controls Goofy')
  check(hp()==72 and emu:read8(gNativePartyHealth+1)==56,'party health stays independent on selection')
  check(emu:read16(gNativeActionLeft)==1 and emu:read16(gNativeMoveLeft)==3,'Goofy budgets remain independent')
  emu:setKeys(1)
 end
 if f==404 then emu:setKeys(0) end
 if f==430 then
  check(emu:read16(gNativeGuard)==2,'Goofy card activates enhanced guard')
  emu:setKeys(12)
 end
 if f==434 then emu:setKeys(0) end
 if f==470 then
  check(emu:read16(gNativeSaveNotice)==1,'native suspend write verifies SRAM')
  emu:screenshot('@OUTPUT@/party.png')
  emu:write16(gGameState+0x32,50);emu:setKeys(12)
 end
 if f==474 then emu:setKeys(0) end
 if f==500 then
  check(emu:read16(gNativeSaveNotice)==1,'second save writes alternate slot')
  emu:write8(0x0e000400,0) -- Corrupt latest magic; older committed slot survives.
  emu:reset()
 end
 if f==720 then
  check(hp()==72,'reset recovers previous valid save after latest corruption')
  check(emu:read16(gNativeParty)==2,'suspend restores selected Goofy')
  check(emu:read16(gNativeGuard)==2,'suspend restores guard state')
  check(emu:read8(gNativeDeck+50)==2,'suspend restores card discard pile')
  check(emu:read16(gNativeActionLeft)==0,'suspend restores spent party action')
  emu:setKeys(4)
 end
 if f==724 then emu:setKeys(0) end
 if f==750 then
  check(emu:read16(gNativeParty)==0 and emu:read16(gNativeActionLeft)==1,'Sora retains independent action after resume')
  emu:setKeys(768)
 end
 if f==754 then emu:setKeys(0) end
 if f==780 then
  check(emu:read16(gNativeActionLeft)==0,'L R reload spends active action')
  check(emu:read8(gNativeDeck+50)~=2,'reload recycles discarded cards')
  emu:screenshot('@OUTPUT@/resume.png')
 end
 if f==800 then emu:setKeys(8) end
 if f==804 then emu:setKeys(0) end
 if f==920 then
  local p=emu:read32(gFieldState)
  for j=0,3 do emu:write32(sPartyPos+16+j*4,emu:read32(p+0x18+j*4)) end
  emu:write32(sPartyPos+16,emu:read32(sPartyPos+16)+4096)
  emu:write8(gNativePartyHealth+1,0)
  emu:write8(gNativePartyHealth+2,30)
  local slot=0
  for i=0,emu:read8(gNativeDeck+72)-1 do
   if emu:read8(gNativeDeck+48+i)==1 then
    if emu:read8(gNativeDeck+i)==2 then
     emu:write8(gNativeDeck+73,slot)
     reviveHp=8+emu:read8(gNativeDeck+24+i)
     break
    end
    slot=slot+1
   end
  end
 end
 if f==922 then check(emu:read16(gNativeCureTarget)==1,'Cure preview identifies nearby knocked-out Donald');emu:setKeys(258) end
 if f==926 or f==934 or f==942 or f==950 then emu:setKeys(0) end
 if f==930 then check(emu:read16(gNativeCureTarget)==2,'R B chooses nearby Goofy for Cure');emu:setKeys(258) end
 if f==938 then check(emu:read16(gNativeCureHeal)==0,'full-health selected Sora previews zero recovery');check(emu:read16(gNativeCureTarget)==0,'Cure cycling includes the active Sora');emu:setKeys(258) end
 if f==946 then
  check(emu:read16(gNativeCureTarget)==1 and emu:read16(gNativeCureChoice)==1,'Cure cycling explicitly selects knocked-out Donald')
  check(emu:read16(gNativeActionLeft)==1,'Cure target cycling preserves the combat action')
  check(emu:read16(gNativeCureHeal)==reviveHp,'chosen knocked-out Donald previews exact revival HP')
  emu:setKeys(1)
 end
 if f==960 then
  check(emu:read8(gNativePartyHealth+1)==reviveHp and reviveHp>0,'Cure revives nearby knocked-out Donald')
  check(hp()==80,'reviving Donald preserves Sora health')
  emu:setKeys(4)
 end
 if f==964 then emu:setKeys(0) end
 if f==1000 then
  check(emu:read16(gNativeParty)==1 and hp()==reviveHp and reviveHp>0,'revived Donald becomes selectable')
  emu:write16(gGameState+0x32,3)
  local p=emu:read32(gFieldState)
  local n=emu:read32(p+0xbc)
  if n~=0 then
   local e=emu:read32(emu:read32(n)+4)
   for j=0,3 do emu:write32(e+8+j*4,emu:read32(p+0x18+j*4)) end
   n=emu:read32(n+8)
   while n~=0 do
    e=emu:read32(emu:read32(n)+4)
    emu:write32(e+8,emu:read32(p+0x18)+40960)
    emu:write32(e+12,emu:read32(p+0x1c))
    emu:write32(e+16,emu:read32(p+0x20))
    n=emu:read32(n+8)
   end
  end
 end
 if f==1020 then
  check(emu:read16(gNativeThreats+2)==4,'preview warns Donald of incoming enemy damage')
  check(emu:read16(gNativeThreats)==0,'preview keeps untargeted Sora safe')
  emu:setKeys(8)
 end
 if f==1024 then emu:setKeys(0) end
 if f==1100 then
  check(emu:read8(gNativePartyHealth+1)==0,'enemy damage knocks out targeted Donald')
  check(emu:read8(gNativePartyHealth)==80 and emu:read16(gNativeResult)==0,'friend knockout does not damage or defeat Sora')
  check(emu:read16(gNativeParty)==2,'turn completion skips knocked-out member')
  emu:setKeys(64)
 end
 if f==1104 then emu:setKeys(0) end
 if f==1112 then check(emu:read16(gNativeFriendPose+2)==2,'Goofy uses original walking animation during actual travel');emu:screenshot('@OUTPUT@/goofy-walk.png') end
 if f==1180 then
  check(emu:read16(gNativeFriendPose+2)==0,'Goofy returns to idle after movement')
  -- Explicit invalid-choice geometry fixtures after native target/revival checks.
  cureX=emu:read32(sPartyPos);cureZ=emu:read32(sPartyPos+8)
  emu:write16(gNativeCureChoice,0);emu:write32(sPartyPos+8,cureZ+8192)
 end
 if f==1186 then
  check(emu:read16(gNativeCureChoice)==65535 and emu:read16(gNativeCureTarget)~=0,'height-invalid Cure choice falls back to an eligible member')
  emu:write32(sPartyPos+8,cureZ);emu:write32(sPartyPos,cureX+65536);emu:write16(gNativeCureChoice,0)
 end
 if f==1192 then
  check(emu:read16(gNativeCureChoice)==65535 and emu:read16(gNativeCureTarget)~=0,'out-of-range Cure choice falls back safely')
  emu:write32(sPartyPos,cureX);out:close()
 end
end)
