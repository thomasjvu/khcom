#include <assert.h>
#include <stdio.h>
#include "field_enemy.h"
int main(void) {
    int floor, slot, kind;
    unsigned int seed;
    for (seed=0;seed<1000;seed++) for(floor=0;floor<3;floor++) {
        assert(FieldEnemyKind(seed,floor,7,0)==2);
        for(slot=0;slot<6;slot++) {
            kind=FieldEnemyKind(seed,floor,3,slot);
            assert(kind>=0&&kind<=6&&kind!=2);
            assert(FieldEnemyHp(kind,floor)>0&&FieldEnemyHp(kind,floor)<=64);
        }
    }
    assert(FieldEnemyRange(1)>FieldEnemyRange(0));
    assert(FieldEnemyHeight(1)>FieldEnemyHeight(0));
    assert(FieldEnemyHp(2,2)==56&&FieldEnemyDamage(2,2)==14);
    puts("field enemy: seeded world groups, boss HP, ranged and height roles passed");
    return 0;
}
