"""Analyze captured native terrain/props; diagnostic evidence, not execution proof."""
import argparse
from collections import deque
import json
from pathlib import Path


def analyze(data, step):
    ox, oy, z = data['position']
    tx, ty, tz = data['goal']
    def sample(x, y):
        cx, cy = x // 8192, y // 4096
        if not (0 <= cx < data['cols'] and 0 <= cy < data['rows']):
            return False
        c = data['cells'][cy * data['cols'] + cx]
        u, l, k = c['upper'], c['lower'], c['kind']
        mask = (c['mask'][(y // 256) % 16] >> (31 - (x // 256) % 32)) & 1
        if u < z:
            ground = (u if mask else l) if k in (4, 6) else u
        else:
            ground = (l if mask else u) if k in (3, 5) else l
        return ground == z and ((u >= z and l != 1048576) or mask == 0)
    def clear(x, y):
        if not all(sample(x, y + offset) for offset in (-1536, 0, 1536)):
            return False
        for px, py, pz, radius, height in data['props']:
            dx, dy = abs(x - px) >> 4, abs((y-z)*2-py) >> 4
            r = (radius + 1024) >> 4
            if z-pz < 8192 and pz-z < height and dx*dx+dy*dy < r*r:
                return False
        return True
    queue = deque([(0, 0)])
    parents = {(0, 0): None}
    nearest = (0, 0)
    best = abs(ox-tx)+2*abs(oy-ty)
    reached = None
    while queue:
        node = queue.popleft()
        x, y = ox + node[0]*step*256, oy + node[1]*step*128
        score = abs(x-tx)+2*abs(y-ty)
        if score < best:
            best, nearest = score, node
        if tz == z and score <= 2048:
            reached = node
            break
        for dx, dy in ((1,0),(-1,0),(0,1),(0,-1),(1,1),(1,-1),(-1,1),(-1,-1)):
            nxt = node[0]+dx, node[1]+dy
            if nxt in parents:
                continue
            ex, ey = ox+nxt[0]*step*256, oy+nxt[1]*step*128
            if all(clear(x+(ex-x)*i//4, y+(ey-y)*i//4) for i in (1,2,3,4)):
                parents[nxt] = node
                queue.append(nxt)
    path = []
    node = reached or nearest
    while node is not None:
        path.append([ox+node[0]*step*256, oy+node[1]*step*128, z])
        node = parents[node]
    return dict(horizontal_step_pixels=step, projected_depth_step_pixels=step/2,
                reachable_samples=len(parents), reached_goal_tolerance=reached is not None,
                closest_weighted_distance_pixels=best/256, path=list(reversed(path)),
                scope='same-height terrain and circular props, quarter-edge sweeps; excludes actors, jumps and native execution')

if __name__ == '__main__':
    p=argparse.ArgumentParser();p.add_argument('snapshot',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args();data=json.loads(a.snapshot.read_text())
    results=[analyze(data,step) for step in (16,8,4)]
    prop_exclusions=[]
    for index in range(len(data['props'])):
        candidate=dict(data)
        candidate['props']=[prop for i,prop in enumerate(data['props']) if i!=index]
        prop_exclusions.append(dict(excluded_prop=index,analysis=analyze(candidate,4)))
    a.output.write_text(json.dumps(dict(seed=data['seed'],floor=data['floor'],room=data['room'],analyses=results,diagnostic_prop_exclusions=prop_exclusions),indent=2)+'\n')
    for r in results:
        print(f"{r['horizontal_step_pixels']}px: reached={r['reached_goal_tolerance']} closest={r['closest_weighted_distance_pixels']:.2f}px samples={r['reachable_samples']}")
