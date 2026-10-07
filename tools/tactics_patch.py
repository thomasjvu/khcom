"""Create/apply BPS patches with source, target and patch CRC32 validation.
Format: https://github.com/Alcaro/Flips/blob/master/bps_spec.md (byuu, public domain).
"""
import argparse
import hashlib
import struct
import zlib
from pathlib import Path
US_SHA1='10729bd884f8fdca7a310b6d606c52e46657aa48'
MAX_ROM=0x2000000

def number(n):
    if n<0:raise ValueError('negative BPS integer')
    out=bytearray()
    while True:
        value=n&127;n>>=7
        if not n:out.append(value|128);return out
        out.append(value);n-=1

def create(source,target,metadata=b''):
    patch=bytearray(b'BPS1')+number(len(source))+number(len(target))+number(len(metadata))+metadata
    start=0
    while start<len(target):
        same=start<len(source) and source[start]==target[start]
        end=start+1
        while end<len(target) and (end<len(source) and source[end]==target[end])==same:end+=1
        patch+=number(((end-start-1)<<2)|(0 if same else 1))
        if not same:patch+=target[start:end]
        start=end
    patch+=struct.pack('<II',zlib.crc32(source),zlib.crc32(target))
    patch+=struct.pack('<I',zlib.crc32(patch))
    return bytes(patch)

def apply(source,patch):
    if len(patch)<16 or patch[:4]!=b'BPS1':raise ValueError('invalid BPS file')
    source_crc,target_crc,patch_crc=struct.unpack('<III',patch[-12:])
    if zlib.crc32(patch[:-4])!=patch_crc:raise ValueError('patch CRC32 mismatch')
    if zlib.crc32(source)!=source_crc:raise ValueError('source CRC32 mismatch')
    pos=4;limit=len(patch)-12
    def read_number():
        nonlocal pos
        value=0;shift=1
        for _ in range(10):
            if pos>=limit:raise ValueError('truncated BPS integer')
            byte=patch[pos];pos+=1;value+=(byte&127)*shift
            if byte&128:return value
            shift<<=7;value+=shift
        raise ValueError('BPS integer too large')
    source_size=read_number();target_size=read_number();metadata_size=read_number()
    if source_size!=len(source):raise ValueError('source size mismatch')
    if target_size>MAX_ROM:raise ValueError('target exceeds GBA ROM capacity')
    pos+=metadata_size
    if pos>limit:raise ValueError('truncated metadata')
    target=bytearray();source_offset=target_offset=0
    while pos<limit:
        command=read_number();action=command&3;length=(command>>2)+1
        if len(target)+length>target_size:raise ValueError('action exceeds target size')
        if action==0:
            offset=len(target)
            if offset+length>len(source):raise ValueError('SourceRead out of range')
            target+=source[offset:offset+length]
        elif action==1:
            if pos+length>limit:raise ValueError('truncated TargetRead')
            target+=patch[pos:pos+length];pos+=length
        elif action==2:
            offset=read_number();source_offset+=(-1 if offset&1 else 1)*(offset>>1)
            if source_offset<0 or source_offset+length>len(source):raise ValueError('SourceCopy out of range')
            target+=source[source_offset:source_offset+length];source_offset+=length
        else:
            offset=read_number();target_offset+=(-1 if offset&1 else 1)*(offset>>1)
            for _ in range(length):
                if target_offset<0 or target_offset>=len(target):raise ValueError('TargetCopy out of range')
                target.append(target[target_offset]);target_offset+=1
    if len(target)!=target_size:raise ValueError('target size mismatch')
    if zlib.crc32(target)!=target_crc:raise ValueError('target CRC32 mismatch')
    return bytes(target)

def main():
    p=argparse.ArgumentParser();p.add_argument('command',choices=['create','apply'])
    p.add_argument('source',type=Path);p.add_argument('input',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
    source=a.source.read_bytes()
    if hashlib.sha1(source).hexdigest()!=US_SHA1:p.error('source is not the supported original US ROM')
    data=a.input.read_bytes()
    if a.command=='create':
        result=create(source,data,b'<kh-tactics version="0.11-surfaces"/>')
        if apply(source,result)!=data:raise ValueError('patch round-trip differs')
    else:result=apply(source,data)
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_bytes(result)
    print(f'{a.output}: {len(result)} bytes; verified')
if __name__=='__main__':main()
