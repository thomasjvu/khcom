"""Generate small bitmap actors only from the user's extracted game sprites."""
from pathlib import Path
import struct
import yaml
from sprite_sheet import read_png
ROOT = Path(__file__).resolve().parents[1]

def palette(data):
    pos=8
    while pos<len(data):
        size,kind=struct.unpack_from('>I4s',data,pos)
        if kind==b'PLTE':
            rgb=data[pos+8:pos+8+size]
            return [(rgb[i]>>3)|((rgb[i+1]>>3)<<5)|((rgb[i+2]>>3)<<10) for i in range(0,len(rgb),3)]
        pos+=12+size
    raise ValueError('sprite has no palette')

def generate(version,out):
    colors=[0]*256
    actors=[]
    sources=[('sora','sprites_sora','sor1ff00'),('shadow','sprites_emy','emy_00_l_00'),
             ('mage','sprites_emy','emy_01_l_00'),('knight','sprites_emy','emy_25_00'),
             ('boss','sprites_emy','emy_37_00')]
    for n,(name,group,sheet) in enumerate(sources):
        manifest=yaml.safe_load((ROOT/f'config/assets/{group}.yaml').read_text())
        entry=next(e for e in manifest['entries'] if e['name']==sheet)
        data=(ROOT/f'assets/{version}/{group}/{sheet}.png').read_bytes()
        w,h,pixels=read_png(data);cw,ch=entry['cell']
        # Crop first frame to nonzero indices (each palette bank's index 0 is transparent).
        occupied=[(x,y) for y in range(ch) for x in range(cw) if pixels[y*w+x]%16]
        x0=min(x for x,y in occupied);x1=max(x for x,y in occupied)+1
        y0=min(y for x,y in occupied);y1=max(y for x,y in occupied)+1
        pw,ph=x1-x0,y1-y0
        scale=min(20/pw,23/ph);sw=max(1,round(pw*scale));sh=max(1,round(ph*scale))
        pal=palette(data);base=32+n*32
        if len(pal)>32: raise ValueError('actor palette exceeds allocation')
        colors[base:base+len(pal)]=pal
        output=[]
        for y in range(24):
            for x in range(24):
                ox=x-(24-sw)//2;oy=y-(24-sh)
                index=pixels[(y0+min(ph-1,int(oy/scale)))*w+x0+min(pw-1,int(ox/scale))] if 0<=ox<sw and 0<=oy<sh else 0
                output.append(base+index if index%16 else 0)
        actors.append(f'static const unsigned char s{name.title()}Pixels[576] = {{'+','.join(map(str,output))+'};')
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text('/* Generated from local ROM assets. Do not distribute. */\n'+
        'static const unsigned short sActorPalette[256] = {'+','.join(map(str,colors))+'};\n'+'\n'.join(actors)+'\n')

if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('version');p.add_argument('output',type=Path);a=p.parse_args()
    generate(a.version,a.output)
