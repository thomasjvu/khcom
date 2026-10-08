#include <assert.h>
#include <stdio.h>
#include "field_save.h"
int main(int argc,char** argv) {
    FieldSaveState state;
    unsigned char slots[FIELD_SAVE_SIZE*2];
    unsigned int generation;
    FILE* file;
    int i,hero;
    assert(argc==2);file=fopen(argv[1],"rb");assert(file);
    assert(fread(slots,1,sizeof(slots),file)==sizeof(slots));fclose(file);
    assert(FieldSaveSelect(&state,&generation,slots,slots+FIELD_SAVE_SIZE)>=0);
    /* Native restore treats deployed slot resources as authoritative. */
    for(i=0;i<3;i++) {
        hero=state.roster.deployed[i];
        state.roster.heroHp[hero]=state.partyHp[i];
        state.roster.heroMove[hero]=state.move[i];
        state.roster.heroAction[hero]=state.action[i];
    }
    printf("local expectedParty=%u\nlocal expectedRoster={",state.party);
    for(i=0;i<45;i++)printf("%s%u",i?",":"",((unsigned char*)&state.roster)[i]);
    printf("}\nlocal expectedDeck={");
    for(i=0;i<(int)sizeof(state.deck);i++)printf("%s%u",i?",":"",((unsigned char*)&state.deck)[i]);
    printf("}\nlocal expectedHealth={%u,%u,%u}\n",state.partyHp[0],state.partyHp[1],state.partyHp[2]);
    return 0;
}
