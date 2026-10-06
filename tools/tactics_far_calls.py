"""Route the appended Thumb code's calls through nearby tail-call veneers."""
import re
import sys
from pathlib import Path
TARGETS=('__divsi3','__udivsi3','__modsi3','__umodsi3','memcpy',
         'm4aSongNumStart','m4aSoundInit','m4aSoundVSyncOn','m4aSoundVSync','m4aSoundMain')

def rewrite(path):
    p=Path(path);s=p.read_text()
    for target in TARGETS:
        s=re.sub(r'(?m)^(\s*bl\s+)'+re.escape(target)+r'\s*$',r'\1TacFar_'+target,s)
    p.write_text(s)

if __name__=='__main__':rewrite(sys.argv[1])
