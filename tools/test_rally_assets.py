"""Validate Rally's native OBJ encoding, bounds and dedicated action bindings."""
import re, hashlib, struct
from pathlib import Path
from PIL import Image
header=Path('tactics/character-assets/rally/rally_data.h').read_text()
def array(name):
 body=re.search(r'\b'+name+r'(?:\[.*?\]).*?=\s*\{(.*?)\};',header,re.S).group(1)
 return [int(token,0) for token in body.replace('\n','').split(',') if token.strip()]
palette=array('sRallyPalette');tiles=array('sRallyTiles')
assert len(palette)==16 and len(tiles)==30*1024
# Golden hashes of the approved pre-action palette and all original20 tiles.
assert hashlib.sha256(struct.pack('<16H',*palette)).hexdigest()=='32b9f49dbdb10839d48289f3089fe8aad9a0b0fe41af11eccc0a69c9299dbec8'
assert hashlib.sha256(bytes(tiles[:20480])).hexdigest()=='fbf1404e6ec2e2ba5612fe2edb6ee7b7333a79609a0084299d4bf325a74fa613'
atlas=Image.open('tactics/character-assets/rally/rally-native.png')
assert atlas.mode=='P' and atlas.size==(160,384) and atlas.info['transparency']==0
assert max(atlas.getdata())<16
for frame in range(30):
 assert array('sRallyFrame'+str(frame))==[1,0x80c8,0xc1f0,frame*32,0]
 x0=(frame%5)*32;y0=(frame//5)*64;encoded=[]
 for ty in range(8):
  for tx in range(4):
   for y in range(8):
    for x in range(0,8,2):
     encoded.append(atlas.getpixel((x0+tx*8+x,y0+ty*8+y))|(atlas.getpixel((x0+tx*8+x+1,y0+ty*8+y))<<4))
 assert encoded==tiles[frame*1024:(frame+1)*1024],f'frame {frame} tile layout'
 bounds=atlas.crop((x0,y0,x0+32,y0+64)).getbbox()
 assert bounds and bounds[3]==56,f'frame {frame} foot baseline'
for bank in range(4):
 action=20+(bank%2)*5
 for name,expected in [('Pose',[action,action+1,action+2]),('Rise',[action+3]),('Fall',[action+4])]:
  values=array(f'sRally{name}{bank}')
  assert values[:3]==[0,0,len(expected)] and values[3::2]==expected
  assert all(duration>0 for duration in values[4::2])
assert len({bytes(tiles[i*1024:(i+1)*1024]) for i in range(20,30)})==10
print('Rally assets: 30 encoded frames, palette, fixed feet and 10 dedicated action/air poses passed')
