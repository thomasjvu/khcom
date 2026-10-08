#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "field_save.h"
/* Recorded native 0.40 suspend: verify every old payload byte, rather than
 * only checking a few fields after the roster gains a sixth hero. */
int main(void) {
    unsigned char slots[FIELD_SAVE_SIZE*2],out[FIELD_SAVE_SIZE];
    const unsigned char* old;
    FieldSaveState state;
    unsigned int generation;
    FILE* file;
    int slot, length, prefix, a, b, i;
    static const int oldSizes[6]={4,10,5,5,10,5};
    static const int newSizes[6]={4,12,5,6,12,6};
    file=fopen("tests/fixtures/phase-ui-format12.sav","rb");assert(file);
    assert(fread(slots,1,sizeof(slots),file)==sizeof(slots));fclose(file);
    slot=FieldSaveSelect(&state,&generation,slots,slots+FIELD_SAVE_SIZE);
    assert(slot>=0);old=slots+slot*FIELD_SAVE_SIZE;assert(old[4]==12);
    assert(!(state.roster.unlocked&(1<<FIELD_ALADDIN)));
    assert(state.roster.heroHp[FIELD_ALADDIN]==60);
    assert(state.roster.heroMove[FIELD_ALADDIN]==3&&state.roster.heroAction[FIELD_ALADDIN]==1);
    assert(state.roster.heroAngle[FIELD_ALADDIN]==0);
    assert(state.roster.power[FIELD_ALADDIN]==0&&state.roster.sleights[FIELD_ALADDIN]==0);
    assert(FieldSaveEncode(&state,generation,out)&&out[4]==13);
    assert(memcmp(old+8,out+8,4)==0);
    length=old[6]|old[7]<<8;prefix=length-39;
    assert(prefix>0&&(out[6]|out[7]<<8)==length+6);
    assert(memcmp(old+16,out+16,prefix)==0);
    a=b=16+prefix;
    for(i=0;i<6;i++) {
        assert(memcmp(old+a,out+b,oldSizes[i])==0);
        a+=oldSizes[i];b+=newSizes[i];
    }
    assert(a==16+length&&b==16+length+6);
    puts("native format 12 migration: every prior payload byte preserved; sixth hero locked with fresh resources");
    return 0;
}
