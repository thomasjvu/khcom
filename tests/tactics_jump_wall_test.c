#include <assert.h>
#include <stdio.h>
#include "field_jump.h"
int main(void) {
    FieldJumpMotion m;
    /* Actual leftward native frames63-66, source jump-direction32 trace. */
    FieldJumpMotionInit(&m,40094,75776,28064,45056,75776,512);
    m.phase=1; m.velocity=-803;
    FieldJumpMotionStep(&m,-256,0,1,36864);
    FieldJumpMotionWall(&m,40094,75776,36864);
    assert(m.x==40094 && m.y==75776 && m.z==27261 && m.speed==460);
    FieldJumpMotionStep(&m,-256,0,1,36864);
    FieldJumpMotionWall(&m,40094,75776,36864);
    assert(m.z==26524 && m.speed==428);
    FieldJumpMotionStep(&m,-256,0,1,36864);
    FieldJumpMotionWall(&m,40094,75776,36864);
    assert(m.z==25853 && m.speed==399);
    m.phase=2; m.speed=512; m.z=20000;
    FieldJumpMotionWall(&m,40094,75776,36864);
    assert(m.speed==512);
    m.z=35000;
    FieldJumpMotionWall(&m,40094,75776,36864);
    assert(m.speed==460);
    puts("Native recorded wall rollback/damping and falling-height rules passed");
    return 0;
}
