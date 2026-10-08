"""Generate explicit Jafar field presentation and spell fixtures."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('elf');p.add_argument('output');p.add_argument('--recruit',action='store_true');p.add_argument('--power',action='store_true');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    parts=line.split()
    if len(parts)==3:names[parts[2]]=int(parts[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gNativeRoster','gNativeProgressReward','sRecruitTiles','sRecruitPalette','sRecruitCard','sUiGlyphs','gNativeAssembly','gFieldState','gMapRoomState','gMapFloorState','gNativeFloor',
 'gNativeJafarReady','gNativeJafarPose','sJafarTiles','sJafarPalette',
 'gNativeEnemyHp','gNativeEnemyCharge','gNativeThreats','gNativePartyHealth',
 'sEnemyTasks','sPartyPos','gNativeSaveNotice','gNativeGuard','gNativeDeck',
 'sValueTiles','sValuePalette','sCardTiles','sCardPalettes')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
logic=Path('tests/tactics_jafar_smoke.lua').read_text()
if a.recruit or a.power:
 logic=logic[:logic.index(' if f==1850')]+Path('tests/tactics_aladdin_recruit_tail.lua').read_text()
 if a.power:
  logic=logic.replace('f==1825 then emu:setKeys(64)','f==1825 then emu:setKeys(0)')
  logic=logic.replace("hudText(5,2,'RECRUIT ALADDIN')","hudText(5,2,'RECRUIT ALADDIN')")
  logic=logic.replace('emu:read8(gNativeRoster)==55','emu:read8(gNativeRoster)==23 and emu:read8(gNativeRoster+4)==1')
  logic=logic.replace('Aladdin unlock','power reward without Aladdin unlock')
(out/'boss.lua').write_text(header+logic.replace('@OUTPUT@',str(out)))

rom=Path(a.elf).resolve().with_suffix('.gba')
script=(out/'boss.lua').read_bytes()
(out/'fixture-metadata.json').write_text(json.dumps({
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script).hexdigest(),
 'explicit_memory_fixtures':True,
 'expected_checks':32 if a.recruit or a.power else 25,
 'result':'not observed; inspect boss.txt'
},indent=2)+'\n')
