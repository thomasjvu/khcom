#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "field_save.h"
int main(int argc,char** argv) {
    unsigned char slots[FIELD_SAVE_SIZE*2];
    FieldSaveState state;unsigned int generation;
    FILE* file;int selected;
    assert(argc==2||argc==3);file=fopen(argv[1],"rb");assert(file);
    assert(fread(slots,1,sizeof(slots),file)==sizeof(slots));fclose(file);
    assert(slots[4]==8||slots[4]==9||slots[FIELD_SAVE_SIZE+4]==8||slots[FIELD_SAVE_SIZE+4]==9);
    selected=FieldSaveSelect(&state,&generation,slots,slots+FIELD_SAVE_SIZE);
    assert(selected>=0);assert(FieldRosterValid(&state.roster));
    assert(state.roster.unlocked==(argc==3?atoi(argv[2]):7)&&state.roster.deployed[0]==0&&state.roster.deployed[1]==1&&state.roster.deployed[2]==2);
    assert(state.roster.power[3]==0&&state.roster.sleights[1]==0);
    assert(state.roster.heroHp[FIELD_CLOUD]==72);
    puts("legacy native format-8/9 SRAM decodes with default bench health migration");return 0;
}
