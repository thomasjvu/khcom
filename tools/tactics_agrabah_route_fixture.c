/* Diagnostic only: reproduce the recorded Agrabah route stall.
 * This writes an explicit suspend fixture, not campaign-completion evidence. */
#include <stdio.h>
#include <string.h>
#include "field_save.h"
int main(int argc,char** argv) {
    unsigned char bytes[32768];FieldSaveState state;unsigned int generation;
    FILE* file;int i,j;
    if(argc!=3)return 1;
    file=fopen(argv[1],"rb");if(!file)return 2;
    if(fread(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 3;}fclose(file);
    if(FieldSaveSelect(&state,&generation,bytes,bytes+FIELD_SAVE_SIZE)<0)return 4;
    if(state.seed!=4411213u)return 5;
    state.floor=1;state.room=5;state.party=0;state.hp=73;state.guard=0;
    state.climbing=0;state.climbTarget=0;state.climbAngle=0;
    state.roster.phase=FIELD_BATTLE;state.roster.reward=0;state.roster.room=5;
    for(i=0;i<12;i++) {
        state.roomFlags[i]=1; /* FLOOR_ROOM_FLAG_CREATED: preserve native open doors. */state.roomEnemies[i]=0;state.roomCached[i]=0;
        memset(state.encounters[i],0,sizeof(state.encounters[i]));
    }
    state.roomCached[5]=1;
    for(i=0;i<3;i++) {
        state.partyPos[i][0]=76060;state.partyPos[i][1]=86995-8192;
        state.partyPos[i][2]=8192;state.partyPos[i][3]=8192;
        state.move[i]=3;state.action[i]=1;state.partyHp[i]=i==0?73:0;
        j=state.roster.deployed[i];state.roster.heroHp[j]=state.partyHp[i];
        state.roster.heroMove[j]=3;state.roster.heroAction[j]=1;
    }
    memset(bytes,0xff,sizeof(bytes));
    if(!FieldSaveEncode(&state,generation+1,bytes))return 6;
    file=fopen(argv[2],"wb");if(!file)return 7;
    if(fwrite(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 8;}fclose(file);
    return 0;
}
