#include <assert.h>
#include <stdio.h>
#include "field_enemy.h"
int main(void) {
    int floor, slot, kind;
    unsigned int seed;
    unsigned int roles[3] = {0,0,0};
    for (seed=0;seed<1000;seed++) for(floor=0;floor<3;floor++) {
        assert(FieldEnemyKind(seed,floor,7,0)==2);
        if(floor==0)assert(FieldEnemyKind(seed,floor,9,0)==2);
        for(slot=0;slot<6;slot++) {
            kind=FieldEnemyKind(seed,floor,3,slot);
            assert(kind>=0&&kind<=6&&kind!=2);
            assert(FieldEnemyHp(kind,floor)>0&&FieldEnemyHp(kind,floor)<=64);
            assert(kind==FieldEnemyKind(seed,floor,3,slot&1));
            roles[floor] |= 1u << kind;
        }
    }
    assert(FieldEnemyRange(1)>FieldEnemyRange(0));
    assert(roles[0]==((1u<<0)|(1u<<1)|(1u<<3)));
    assert(roles[1]==roles[0]);
    assert(roles[2]==(roles[0]|(1u<<6)));
    assert(FieldEnemyHeight(1)>FieldEnemyHeight(0));
    assert(FieldEnemyHp(2,2)==56&&FieldEnemyDamage(2,2)==14);
    assert(FieldEnemyBlastRange(0,7)==80);
    assert(FieldEnemyBlastRange(0,3)==64);
    assert(FieldEnemyBlastRange(1,7)==64);
    assert(FieldArmorPhase(27)==0 && FieldArmorPhase(26)==1);
    assert(FieldArmorPhase(14)==1 && FieldArmorPhase(13)==2);
    assert(FieldArmorRange(40)==80 && FieldArmorRange(26)==64 && FieldArmorRange(13)==48);
    assert(FieldArmorDamage(40)==10 && FieldArmorDamage(26)==8 && FieldArmorDamage(13)==6);
    puts("field enemy: seeded world groups, boss HP, ranged and height roles passed");
    return 0;
}
