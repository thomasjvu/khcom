#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "field_save.h"
int main(void) {
    FieldSaveState s, loaded;
    unsigned char a[FIELD_SAVE_SIZE],b[FIELD_SAVE_SIZE];
    unsigned int gen;
    int i;
    memset(&s,0,sizeof(s));FieldDeckInit(&s.deck);FieldRosterInit(&s.roster);
    s.roster.unlocked=15;s.roster.deployed[1]=FIELD_CLOUD;s.roster.power[FIELD_CLOUD]=3;s.roster.sleights[FIELD_DONALD]=5;
    s.partyHp[0]=80;s.partyHp[1]=56;s.partyHp[2]=64;
    s.seed=0x434f4du;s.hp=64;s.party=2;s.move[2]=2;s.guard=2;
    s.roomEnemies[0]=1;s.roomCached[0]=1;s.encounters[0][0].hp=19;s.encounters[0][0].kind=1;
    s.roomCached[9]=1;s.roomEnemies[9]=1;s.encounters[9][0].hp=7;s.encounters[9][0].pos[2]=-16;
    s.partyPos[2][0]=0x11000;s.partyPos[2][2]=-4096;
    assert(FieldSaveEncode(&s,0xffffffffu,a));
    assert(FieldDeckStock(&s.deck)&&FieldDeckStock(&s.deck));
    s.hp=72;s.partyHp[2]=72;assert(FieldSaveEncode(&s,0,b));
    assert(FieldSaveSelect(&loaded,&gen,a,b)==1&&gen==0&&loaded.hp==72);
    assert(loaded.roster.unlocked==15&&loaded.roster.deployed[1]==FIELD_CLOUD&&loaded.roster.power[FIELD_CLOUD]==3&&loaded.roster.sleights[FIELD_DONALD]==5);
    assert(loaded.deck.stocked==2&&loaded.deck.pile[loaded.deck.stock[0]]==4);
    assert(loaded.partyPos[2][2]==-4096&&loaded.encounters[0][0].hp==19&&loaded.encounters[9][0].hp==7&&loaded.encounters[9][0].pos[2]==-16);
    for(i=0;i<FIELD_SAVE_SIZE;i++) {
        b[i]^=1;
        assert(!FieldSaveDecode(&loaded,&gen,b));
        b[i]^=1;
    }
    b[0]=0;assert(FieldSaveSelect(&loaded,&gen,a,b)==0&&loaded.hp==64);
    s.climbing=1;s.climbTarget=24576;s.climbAngle=211;
    assert(FieldSaveEncode(&s,1,b)&&FieldSaveDecode(&loaded,&gen,b));
    assert(loaded.climbing&&loaded.climbTarget==24576&&loaded.climbAngle==211);
    s.climbAngle=0;assert(!FieldSaveEncode(&s,1,b));s.climbing=0;
    s.encounters[0][0].kind=10;
    s.encounters[9][0].kind=6;
    assert(FieldSaveEncode(&s,1,b)&&FieldSaveDecode(&loaded,&gen,b));
    assert(loaded.encounters[0][0].kind==10&&loaded.encounters[9][0].kind==6);
    s.encounters[0][0].kind=9;assert(!FieldSaveEncode(&s,1,b));
    s.encounters[0][0].kind=7;assert(!FieldSaveEncode(&s,1,b));
    s.encounters[0][0].kind=2;
    s.roomEnemies[0]=2;assert(!FieldSaveEncode(&s,1,b));
    s.roomEnemies[0]=1;s.hp=0;assert(!FieldSaveEncode(&s,1,b));
    puts("field save: all-byte corruption, fallback, generation wrap, exact state passed");
    return 0;
}
