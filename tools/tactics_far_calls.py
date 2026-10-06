"""Route the appended Thumb code's calls through nearby tail-call veneers."""
import re
import sys
from pathlib import Path
TARGETS=('__divsi3','__udivsi3','__modsi3','__umodsi3','memcpy',
         'm4aSongNumStart','m4aSoundInit','m4aSoundVSyncOn','m4aSoundVSync','m4aSoundMain','InitSystem','ResetGameState','SetupSoraNewGame','EnterFloorWorld','ModeRequest','EnableVBlankIntr','UpdateKeyState','ModeUpdate','ApplyIntrCallbacks','VBlankIntrWait','Mode_MapFld_0','MapEnmCheckAttacked','GetMapFloorRoom','TaskKill','MapEnmUpdateAnim','GetKeysPressed','UpdateMapField','ColliderSetDisabled','MapEnmUpdateSpawner','ColliderUpdateAll','DrawMapField','CreateMapRoom','SetCurrentMapRoom','DebugTextInit','DebugTextLoadPalette','DebugTextPrint','DebugTextDraw','DebugTextClear','DebugTextFree','Mode_MapFld_2','MapEnmSetAnim','SeedRandom','MapEnmSetupArgs','TaskCreate','MapCellIsFreeOfType','FldPosPlaceAtCell','_call_via_r3','_call_via_r1','MapReserveArea')

def rewrite(path):
    p=Path(path);s=p.read_text()
    for target in TARGETS:
        s=re.sub(r'(?m)^(\s*bl\s+)'+re.escape(target)+r'\s*$',r'\1TacFar_'+target,s)
    p.write_text(s)

if __name__=='__main__':rewrite(sys.argv[1])
