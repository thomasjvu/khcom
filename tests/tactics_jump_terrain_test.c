#include <assert.h>
#include <stdio.h>
#include "field_jump.h"
static int calls;
static int ground(FieldJumpPoint* p, void* context) {
    (void)context;
    return p->y<10000 ? 8192 : 12288;
}
static int clear(FieldJumpPoint* p, void* context) {
    (void)context;
    assert(p->y==8464 || p->y==11536);
    assert(p->ground==(p->y==8464 ? 8192 : 12288));
    calls++;
    return 0;
}
static int wall(FieldJumpPoint* p, void* context) {
    (void)context;
    return p->y>10000;
}
int main(void) {
    FieldJumpPoint p;
    p.x=20000;p.y=10000;p.z=0;p.ground=4096;
    calls=0;
    assert(!FieldJumpTerrainCheck(&p,ground,clear,0));
    assert(calls==2 && p.ground==8192 && p.y==10000);
    p.ground=4096;
    assert(FieldJumpTerrainCheck(&p,ground,wall,0));
    assert(p.ground==4096 && p.y==10000);
    puts("Jump terrain footprint probes, support and blocked-state isolation passed");
    return 0;
}
