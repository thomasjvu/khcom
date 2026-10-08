"""Resume a recorded native format-12 SRAM fixture and rewrite it natively."""
import argparse, hashlib, json, subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('elf');p.add_argument('output');a=p.parse_args()
out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
oracle=Path('build/tactics/native_legacy_oracle')
subprocess.run(['cc','-std=c89','-pedantic','-Wall','-Wextra','-Werror','-I','tactics','tactics/field_deck.c','tactics/field_roster.c','tactics/field_save.c','tests/tactics_native_legacy_oracle.c','-o',str(oracle)],check=True)
fixture=Path('tests/fixtures/phase-ui-format12.sav')
expected=subprocess.check_output([str(oracle),str(fixture)],text=True)
names={w[2]:int(w[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',a.elf],text=True).splitlines() if len(w:=line.split())==3}
keys=('gNativeRoster','gNativeDeck','gNativePartyHealth','gNativeParty','gNativeSaveNotice','sSaveSlot')
script=expected+''.join(f'local {k}=0x{names[k]:x}\n' for k in keys)+Path('tests/tactics_native_legacy_smoke.lua').read_text().replace('@OUTPUT@',str(out))
(out/'test.lua').write_text(script)
(out/'fresh.sav').write_bytes(fixture.read_bytes()+b'\xff'*(32768-fixture.stat().st_size))
(out/'metadata.json').write_text(json.dumps(dict(rom_sha256=hashlib.sha256(Path(a.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),legacy_fixture_sha256=hashlib.sha256(fixture.read_bytes()).hexdigest(),expected_checks=7,fixture='Recorded native format-12 SRAM; current host decoder supplies semantic oracle; native save/reset inputs thereafter.',result='not yet observed'),indent=2)+'\n')
