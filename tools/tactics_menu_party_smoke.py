"""Generate an input-only native Party command regression."""
import argparse,subprocess,hashlib,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeAssembly','gNativeRoster','gNativeParty','gNativeMenu','gNativeMenuChoice','gNativeMoveLeft','gNativeActionLeft','gNativePreview','gNativeSaveNotice','gNativePartyHealth','gGameState','gNativeDeck','gNativeCureTarget','gNativeCureHeal','gFieldState','sEnemyTasks','gNativeEnemyHp','gNativeFireTarget','gNativeFireDamage','gNativeBusy','gNativeDirection','gNativeTurn','gNativeEnemyFrames','gWin0V','sUiGlyphs')
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
s=''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_menu_party_smoke.lua').read_text().replace('@OUTPUT@',str(out))

assert 'emu:write' not in s
(out/'test.lua').write_text(s)
(out/'metadata.json').write_text(json.dumps(dict(input_only=True,explicit_memory_fixture=False,rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(s.encode()).hexdigest(),expected_checks=17,result='not yet observed'),indent=2)+'\n')
