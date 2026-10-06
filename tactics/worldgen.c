#include "worldgen.h"
static unsigned int Next(unsigned int* state) {
    *state = *state * 1664525U + 1013904223U;
    return *state;
}
static void Link(struct TacWorld* world, unsigned char a, unsigned char side,
                 unsigned char b) {
    static const unsigned char opposite[4] = {1,0,3,2};
    world->links[a][side] = b;
    world->links[b][opposite[side]] = a;
}
void TacWorldGenerate(struct TacWorld* world, unsigned int seed, unsigned char floor) {
    unsigned int rng = seed ^ (0x9e3779b9U * (floor + 1));
    unsigned char i, j;
    struct TacWorldRoom* room;
    for (i = 0; i < TAC_WORLD_ROOMS; i++) {
        for (j = 0; j < 4; j++) world->links[i][j] = TAC_WORLD_NONE;
        room = &world->rooms[i];
        room->seed = Next(&rng);
        room->layout = (unsigned char)(Next(&rng) % 3 + 1);
        room->width = (unsigned char)(Next(&rng) % 7 + 12);
        room->heightMin = 2;
        room->heightMax = (unsigned char)(Next(&rng) % 3 + 4);
        room->depthMin = 4;
        room->depthMax = (unsigned char)(Next(&rng) % 4 + 8);
        room->chest = (i == 3 || i == 9 || i == 11);
        if (room->chest) {
            /* Reward props need a broad, reachable floor rather than narrow ledges. */
            room->layout = 0;
            room->width = 16;
            room->depthMin = 8;
            room->depthMax = 12;
        }
        room->enemies = room->chest ? 0 : (unsigned char)(2 + (floor != 0));
    }
    for (i = 0; i < 7; i++) Link(world, i, 1, i+1);
    /* Seed selects which side the optional loops occupy. */
    j = (unsigned char)((Next(&rng) & 1) ? 2 : 3);
    Link(world, 1, j, 8);
    Link(world, 8, 1, 9);
    Link(world, 9, j, 4);
    Link(world, 5, j, 10);
    Link(world, 10, 1, 11);
    /* The far door in room 7 advances to the next world. */
    world->links[7][1] = TAC_WORLD_EXIT;
}
