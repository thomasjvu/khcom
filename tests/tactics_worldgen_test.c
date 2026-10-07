#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "worldgen.h"
int main(void) {
    struct TacWorld world, replay;
    unsigned int seed;
    unsigned char floor, i, side, neighbor, reached[12], pass;
    static const unsigned char opposite[4] = {1,0,3,2};
    for (seed=0; seed<1000; seed++) for (floor=0; floor<3; floor++) {
        memset(&world,0,sizeof(world)); memset(&replay,0,sizeof(replay));
        TacWorldGenerate(&world,seed,floor); TacWorldGenerate(&replay,seed,floor);
        assert(!memcmp(&world,&replay,sizeof(world)));
        memset(reached,0,sizeof(reached)); reached[0]=1;
        for (pass=0;pass<12;pass++) for (i=0;i<12;i++) {
            assert(world.rooms[i].width>=12 && world.rooms[i].width<=18);
            assert(world.rooms[i].heightMin<=world.rooms[i].heightMax);
            assert(world.rooms[i].depthMin<=world.rooms[i].depthMax);
            for(side=0;side<4;side++) {
                neighbor=world.links[i][side];
                if(neighbor==255)continue;
                if(neighbor==TAC_WORLD_EXIT) { assert(i==7 && side==1); continue; }
                assert(neighbor<12);
                assert(world.links[neighbor][opposite[side]]==i);
                if(reached[i])reached[neighbor]=1;
            }
        }
        for(i=0;i<12;i++)assert(reached[i]);
        assert(world.rooms[9].chest && world.rooms[11].chest);
        assert(world.rooms[7].enemies==(floor<=1 ? 1 : 3));
        assert(world.links[7][1]==TAC_WORLD_EXIT);
        assert(world.links[0][0]==TAC_WORLD_NONE);
    }
    puts("worldgen: 3000 seeded graphs passed connectivity, reciprocal doors and bounds");
    return 0;
}
