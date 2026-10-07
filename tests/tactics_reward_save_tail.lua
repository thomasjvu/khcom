 if f==550 then emu:setKeys(12) end
 if f==554 then emu:setKeys(0) end
 if f==600 then
  check(emu:read16(gNativeSaveNotice)==1,'confirmed chest reward can be suspended')
  emu:reset()
 end
 if f==900 then
  check(emu:read8(gMapFloorState+6)==9,'suspend restores the reward room')
  check(emu:read8(gNativeDeck+72)==13,'suspend preserves exactly one chosen reward card')
  check(emu:read8(gNativeDeck+12)==(rewardSeed%4+1)%4 and emu:read8(gNativeDeck+36)==5+((rewardSeed>>8)+1)%5,'suspend preserves chosen card kind and value')
  check(emu:read16(gNativeChests)==1 and (emu:read16(gMapFloorState+0x1c+9*16)&16)~=0,'suspend keeps the chest opened')
  check(emu:read16(gNativeReward)==0,'confirmed reward does not reopen after reset')
  check(emu:read16(gGameState+0x32)==52,'suspend preserves confirmed party healing')
  emu:screenshot('@OUTPUT@/reward-resumed.png')
 end
 if f==930 then
  -- Explicit capacity fixture: isolate full-deck reward confirmation.
  for i=13,23 do
   emu:write8(gNativeDeck+i,i%4);emu:write8(gNativeDeck+24+i,5);emu:write8(gNativeDeck+48+i,0)
  end
  emu:write8(gNativeDeck+72,24);emu:write16(gGameState+0x32,40);emu:write16(gNativeReward,1)
 end
 if f==934 then emu:setKeys(32) end
 if f==938 then emu:setKeys(0) end
 if f==940 then
  check(emu:read16(gNativeRewardChoice)==2,'Left wraps reward selection from first choice to third')
  emu:setKeys(16)
 end
 if f==944 then emu:setKeys(0) end
 if f==948 then
  check(emu:read16(gNativeRewardChoice)==0,'Right wraps reward selection back to first without field movement')
  check(emu:read16(gNativeReward)==1 and emu:read8(gNativeDeck+72)==24,'browsing full-deck reward does not grant cards')
  emu:setKeys(2)
 end
 if f==952 then emu:setKeys(0) end
 if f==958 then
  check(emu:read16(gNativeReward)==1,'B cannot silently abandon an opened chest reward')
 end
 if f==960 then
  emu:screenshot('@OUTPUT@/reward-full-deck.png');emu:setKeys(1)
 end
 if f==964 then emu:setKeys(0) end
 if f==970 then
  check(emu:read8(gNativeDeck+72)==24,'full-deck confirmation never exceeds card capacity')
  check(emu:read16(gGameState+0x32)==52,'full-deck confirmation still grants party healing')
  check(emu:read16(gNativeReward)==0,'full-deck confirmation dismisses the reward safely')
  emu:setKeys(1)
 end
 if f==974 then emu:setKeys(0) end
 if f==1010 then
  check(emu:read16(gGameState+0x32)==52,'a later A press cannot grant chest healing a second time')
  check(emu:read8(gNativeDeck+72)==24 and emu:read16(gNativeChests)==1,'a later A press cannot duplicate the opened chest reward')
  out:close()
 end
end)
