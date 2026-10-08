#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "field_save.h"
int main(int argc,char** argv) {
    unsigned char slots[FIELD_SAVE_SIZE*2],a[FIELD_SAVE_SIZE],b[FIELD_SAVE_SIZE];
    FieldSaveState original,restored;unsigned int generation,loadedGeneration;
    FILE* file;int i;
    assert(argc==2);file=fopen(argv[1],"rb");assert(file);
    assert(fread(slots,1,sizeof(slots),file)==sizeof(slots));fclose(file);
    assert(slots[4]==10||slots[FIELD_SAVE_SIZE+4]==10);
    assert(FieldSaveSelect(&original,&generation,slots,slots+FIELD_SAVE_SIZE)>=0);
    assert(original.roster.unlocked==31&&original.roster.deployed[2]==FIELD_CLOUD);
    assert(original.roster.heroHp[FIELD_DONALD]==51&&original.roster.heroAction[FIELD_DONALD]==0);
    for(i=0;i<FIELD_HEROES;i++)assert(original.roster.heroAngle[i]==0);
    assert(FieldSaveEncode(&original,generation,a)&&a[4]==13);
    assert(FieldSaveDecode(&restored,&loadedGeneration,a)&&loadedGeneration==generation);
    assert(FieldSaveEncode(&restored,generation,b)&&memcmp(a,b,sizeof(a))==0);
    puts("recorded native format-10 save upgrades to format 13 with default facing and exact serialized state");
    return 0;
}
