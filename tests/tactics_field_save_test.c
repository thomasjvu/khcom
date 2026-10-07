#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "field_save.h"
int main(void) {
    FieldSaveState s, loaded;
    unsigned char a[FIELD_SAVE_SIZE],b[FIELD_SAVE_SIZE];
    unsigned int gen;
    int i;
    memset(&s,0,sizeof(s));FieldDeckInit(&s.deck);
    s.seed=0x434f4du;s.hp=64;s.party=2;s.move[2]=2;s.guard=2;
    s.roomEnemies[0]=1;s.enemyHp[0]=19;s.enemyKind[0]=1;
    s.partyPos[2][0]=0x11000;s.partyPos[2][2]=-4096;
    assert(FieldSaveEncode(&s,0xffffffffu,a));
    s.hp=72;assert(FieldSaveEncode(&s,0,b));
    assert(FieldSaveSelect(&loaded,&gen,a,b)==1&&gen==0&&loaded.hp==72);
    assert(loaded.partyPos[2][2]==-4096&&loaded.enemyHp[0]==19);
    for(i=0;i<FIELD_SAVE_SIZE;i++) {
        b[i]^=1;
        assert(!FieldSaveDecode(&loaded,&gen,b));
        b[i]^=1;
    }
    b[0]=0;assert(FieldSaveSelect(&loaded,&gen,a,b)==0&&loaded.hp==64);
    s.roomEnemies[0]=2;assert(!FieldSaveEncode(&s,1,b));
    s.roomEnemies[0]=1;s.hp=0;assert(!FieldSaveEncode(&s,1,b));
    puts("field save: all-byte corruption, fallback, generation wrap, exact state passed");
    return 0;
}
