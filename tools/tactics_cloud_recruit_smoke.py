"""Generate explicit Cloud sword presentation and recruitment fixtures."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('elf');p.add_argument('output');p.add_argument('--power',action='store_true');a=p.parse_args()
names={}
for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines():
    parts=line.split()
    if len(parts)==3:names[parts[2]]=int(parts[0],16)
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
keys=('gFieldState','gMapRoomState','gMapFloorState','gNativeFloor',
 'gNativeCloudReady','gNativeCloudPose','sCloudTiles','sCloudPalette',
 'gNativeRoster','gNativeAssembly','gNativeProgressReward','sRecruitTiles','sRecruitPalette','gNativeEnemyHp','gNativeEnemyCharge','gNativeThreats','gNativePartyHealth',
 'sEnemyTasks','sPartyPos','gNativeSaveNotice','gNativeGuard','gNativeDeck',
 'sValueTiles','sValuePalette','sCardTiles','sCardPalettes','gWin0V','gWin1V','sUiGlyphs')
header=''.join(f'local {k}=0x{names[k]:08x}\n' for k in keys)
script=header+Path('tests/tactics_cloud_recruit_smoke.lua').read_text().replace('@OUTPUT@',str(out))
if a.power:
 script=script.replace('emu:setKeys(64)','emu:setKeys(0)').replace('emu:read8(gNativeRoster)==31','emu:read8(gNativeRoster)==23 and emu:read8(gNativeRoster+4)==1')
 script=script.replace('recruit choice unlocks Cloud roster card','power choice upgrades Sora while leaving Cloud locked').replace('Cloud unlock persists without duplicate recruit reward','boss power choice persists without unlocking Cloud')
rom=Path(a.elf).resolve().with_suffix('.gba')
if not rom.is_file():p.error('matching built ROM required beside ELF')
(out/'boss.lua').write_text(script)
(out/'fixture-metadata.json').write_text(json.dumps({
 'rom_sha256':hashlib.sha256(rom.read_bytes()).hexdigest(),
 'elf_sha256':hashlib.sha256(Path(a.elf).read_bytes()).hexdigest(),
 'driver_sha256':hashlib.sha256(script.encode()).hexdigest(),
 'driver_logic_sha256':hashlib.sha256(script.replace(str(out),'@OUTPUT@').encode()).hexdigest(),
 'explicit_memory_fixtures':True,
 'reward_choice':'power' if a.power else 'recruit',
 'expected_checks':19,
 'result':'not yet observed; inspect cloud.txt'
},indent=2)+'\n')
