"""Validate the hack header/capacity without claiming an original-ROM match."""
import sys
from pathlib import Path
from gbafix import NINTENDO_LOGO

def check(path):
    data=Path(path).read_bytes()
    if len(data)!=0x2000000: raise ValueError('expected a 32 MiB GBA ROM')
    if data[4:160]!=NINTENDO_LOGO: raise ValueError('invalid Nintendo boot logo')
    if data[0xAC:0xB0]!=b'KTCE': raise ValueError('wrong tactics product code')
    if data[0xB2]!=0x96 or (sum(data[0xA0:0xBE])+0x19)&255: raise ValueError('invalid GBA header checksum')
    if b'KH TACTICS' not in data: raise ValueError('missing title')
    print('tactics ROM: header and capacity checks passed')

if __name__=='__main__': check(sys.argv[1])
