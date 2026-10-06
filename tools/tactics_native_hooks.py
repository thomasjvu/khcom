"""Install bounded Thumb entry hooks without moving any original ROM data."""
import argparse
import struct
import subprocess
from pathlib import Path
HOOKS = {
    'MapGmkFindSpot': 'NativeGmkFindSpot',
    'MapGmk01Open': 'NativeChestOpen',
    'MapEnmStartBattle': 'NativeEnemyContact',
    'GetKeyReleaseTime': 'NativeGetKeyReleaseTime',
    'GetMapFloorDef': 'NativeGetMapFloorDef',
    'GetMapRoomLinks': 'NativeGetMapRoomLinks',
    'GetMapEventDoor': 'NativeGetMapEventDoor',
}
def install(elf, rom):
    symbols = {}
    for line in subprocess.check_output(['arm-none-eabi-nm', '-S', str(elf)], text=True).splitlines():
        fields = line.split()
        if len(fields) >= 3:
            symbols[fields[-1]] = (int(fields[0], 16), int(fields[1],16) if len(fields)==4 else 0)
    for name, expected in {'sKeysHeld': 0x02034000, 'sKeysPressed': 0x02034002,
                           'sKeysRepeat': 0x02034004}.items():
        if symbols[name][0] != expected:
            raise ValueError(f'US input RAM layout changed: {name}')
    data = bytearray(Path(rom).read_bytes())
    for original, replacement in HOOKS.items():
        addr, size = symbols[original]
        target, _ = symbols[replacement]
        if size < 8 or addr % 4 or not 0x08000000 <= addr < 0x09efbfdc:
            raise ValueError(f'unsafe hook site: {original}')
        if not 0x09efbfdc <= target < 0x0a000000:
            raise ValueError(f'unsafe hook target: {replacement}')
        # ldr r3, [pc, #0]; bx r3; .word target|1 (r0/r1 arguments preserved).
        struct.pack_into('<HHI', data, addr-0x08000000, 0x4b00, 0x4718, target|1)
    Path(rom).write_bytes(data)
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('elf'); parser.add_argument('rom')
    args = parser.parse_args(); install(args.elf,args.rom)
