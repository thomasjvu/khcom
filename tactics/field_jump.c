#include "field_jump.h"
static int magnitude(int value) { return value < 0 ? -value : value; }
void FieldJumpMotionInit(FieldJumpMotion* m, int x, int y, int z,
                         int originX, int originY, int speed) {
    m->x=x; m->y=y; m->z=z; m->originX=originX; m->originY=originY;
    m->speed=speed; m->velocity=0; m->timer=0; m->phase=0;
}
void FieldJumpMotionStep(FieldJumpMotion* m, int sine, int cosine,
                         int moving, int ground) {
    if (m->phase==3) return;
    /* Native tactical controller stops speed before the original update. */
    if (magnitude(m->x-m->originX)+2*magnitude(m->y-m->originY)>=8192)
        { m->speed=0; moving=0; }
    if (m->phase==0) {
        if (!m->timer) m->speed >>= 1;
        m->x += sine*m->speed >> 8;
        m->y += -cosine*m->speed >> 8;
        if (m->timer>3) {
            m->phase=1; m->velocity=-1331; m->speed <<= 1; m->timer=0;
            m->z += m->velocity; m->velocity += 66;
        } else m->timer++;
    } else {
        if (moving) { m->speed+=17; if (m->speed>512) m->speed=512; }
        else { m->speed-=38; if (m->speed<0) m->speed=0; }
        m->x += sine*m->speed >> 8;
        m->y += -cosine*m->speed >> 8;
        m->z += m->velocity; m->velocity+=66;
        if (m->phase==1 && m->velocity>0) m->phase=2;
        else if (m->phase==2 && m->z>ground) {
            m->z=ground; m->velocity=0; m->phase=3;
        }
    }
}
