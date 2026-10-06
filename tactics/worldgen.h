#ifndef TACTICS_WORLDGEN_H
#define TACTICS_WORLDGEN_H
/* Byte-only packed records are shared by host tests and agbcc. */
#define TAC_WORLD_ROOMS 12
#define TAC_WORLD_NONE 255
#define TAC_WORLD_EXIT 253
struct TacWorldRoom {
    unsigned int seed;
    unsigned char layout, width, heightMin, heightMax, depthMin, depthMax;
    unsigned char chest, enemies;
} __attribute__((packed));
struct TacWorld {
    unsigned char links[TAC_WORLD_ROOMS][4];
    struct TacWorldRoom rooms[TAC_WORLD_ROOMS];
} __attribute__((packed));
void TacWorldGenerate(struct TacWorld* world, unsigned int seed, unsigned char floor);
#endif
