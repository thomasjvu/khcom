 if f==760 then emu:setKeys(12)end
 if f==764 then emu:setKeys(0)end
 if f==800 then
  check(emu:read16(gNativeSaveNotice)==1,'native Aladdin party suspend succeeds')
 end
 if f==820 then emu:reset()end
 if f==1150 then
  check(emu:read8(gNativeRoster+2)==5 and emu:read16(gNativeParty)==1,'reboot restores deployed and controlled Aladdin')
  check(emu:read8(gNativePartyHealth+1)==60 and emu:read8(gNativeRoster+26)==60,'reboot preserves Aladdin independent health')
  check(emu:read16(gNativeMoveLeft)==3 and emu:read16(gNativeActionLeft)==0,'reboot preserves recovered movement and spent action')
  check(emu:read8(gNativeDeck+48)==2 and emu:read16(gNativeEnemyHp)==16,'reboot preserves spent Key and resolved sword damage')
  emu:screenshot('@OUTPUT@/resume.png')
  out:close()
 end
end)
