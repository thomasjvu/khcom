#ifndef FIELD_JUMP_H
#define FIELD_JUMP_H
/* Collision-free motion step, seeded after the ground controller commits B.
 * Terrain/ledge/collider resolution must be applied by a native adapter. */
typedef struct FieldJumpMotion {
    int x, y, z, originX, originY, speed, velocity, timer, phase;
} FieldJumpMotion;
void FieldJumpMotionInit(FieldJumpMotion* motion, int x, int y, int z,
                         int originX, int originY, int speed);
void FieldJumpMotionStep(FieldJumpMotion* motion, int sine, int cosine,
                         int moving, int ground);
#endif
