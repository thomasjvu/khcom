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
/* Apply the original wall response after terrain rejects a motion step.
 * Stair/ledge attachment must be handled by the caller instead. */
void FieldJumpMotionWall(FieldJumpMotion* motion, int previousX, int previousY,
                         int ground);
#endif
