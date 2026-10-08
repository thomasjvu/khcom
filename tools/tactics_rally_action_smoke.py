"""Generate input-only Rally dedicated action and airborne regression."""
import argparse, hashlib, json, subprocess
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('elf');parser.add_argument('output');args=parser.parse_args()
symbols={words[2]:int(words[0],16) for line in subprocess.check_output(['arm-none-eabi-nm',args.elf],text=True).splitlines() if len(words:=line.split())==3}
output=Path(args.output).resolve();output.mkdir(parents=True,exist_ok=True)
keys=('gNativeRoster','gNativeParty','gNativeMoveLeft','gNativeActionLeft','gNativeFriendPose','gNativeBusy','gNativeEnemyFrames','gFrameCounter','sRawKeys','gFieldState','sFriends')
script=''.join(f'local {key}=0x{symbols[key]:x}\n' for key in keys)+Path('tests/tactics_rally_action_smoke.lua').read_text().replace('@OUTPUT@',str(output))
(output/'test.lua').write_text(script)
(output/'metadata.json').write_text(json.dumps(dict(explicit_memory_fixture=False,input_only=True,expected_checks=13,rom_sha256=hashlib.sha256(Path(args.elf).with_suffix('.gba').read_bytes()).hexdigest(),driver_sha256=hashlib.sha256(script.encode()).hexdigest(),result='not yet observed'),indent=2)+'\n')
