#ifndef FIELD_JUMP_H
#define FIELD_JUMP_H
/* Collision-free motion step, seeded after the ground controller commits B.
 * Terrain/ledge/collider resolution must be applied by a native adapter. */
typedef struct FieldJumpMotion {
    int x, y, z, originX, originY, speed, velocity, timer, phase;
    int padActive, padX, padY, padTargetZ;
} FieldJumpMotion;
void FieldJumpMotionInit(FieldJumpMotion* motion, int x, int y, int z,
                         int originX, int originY, int speed);
void FieldJumpPadInit(FieldJumpMotion* motion, int padX, int padY, int targetZ);
void FieldJumpMotionStep(FieldJumpMotion* motion, int sine, int cosine,
                         int moving, int ground);
/* Apply the original wall response after terrain rejects a motion step.
 * Stair/ledge attachment must be handled by the caller instead. */
void FieldJumpMotionWall(FieldJumpMotion* motion, int previousX, int previousY,
                         int ground);
typedef struct FieldJumpPoint { int x, y, z, ground; } FieldJumpPoint;
typedef int (*FieldJumpGround)(FieldJumpPoint* point, void* context);
typedef int (*FieldJumpBlocked)(FieldJumpPoint* point, void* context);
/* Match FldSoraCheckBlocked's front/back footprint and ground update. */
int FieldJumpTerrainCheck(FieldJumpPoint* point, FieldJumpGround ground,
                           FieldJumpBlocked blocked, void* context);
#endif
