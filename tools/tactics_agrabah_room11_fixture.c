/* Diagnostic only: reproduce the recorded second-seed Agrabah room11 stationary-turn loop.
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
    if(state.seed!=2658846982u)return 5;
    state.floor=1;state.room=11;state.party=0;state.hp=73;state.guard=0;
    state.climbing=0;state.climbTarget=0;state.climbAngle=0;
    state.roster.phase=FIELD_BATTLE;state.roster.reward=0;state.roster.room=11;
    state.roster.deployed[1]=FIELD_RALLY;state.roster.deployed[2]=FIELD_DONALD;
    for(i=0;i<12;i++) {
        state.roomFlags[i]=1; /* FLOOR_ROOM_FLAG_CREATED: preserve native open doors. */state.roomEnemies[i]=0;state.roomCached[i]=0;
        memset(state.encounters[i],0,sizeof(state.encounters[i]));
    }
    state.roomCached[11]=1;
    {
        static const unsigned char deck[78]={0,1,2,3,0,1,2,3,0,1,2,3,3,1,3,0,2,1,2,3,0,1,2,3,5,6,7,8,0,5,6,7,8,9,5,6,9,8,5,7,8,7,8,9,5,6,7,8,2,2,2,2,2,2,1,2,1,2,1,2,2,2,2,1,1,0,0,0,0,0,0,0,17,3,0,0,0,0};
        memcpy(&state.deck,deck,sizeof(state.deck));
    }
    state.chests=5;state.kills=25;state.roster.power[0]=7;
    for(i=0;i<3;i++) {
        state.partyPos[i][0]=i==0?54316:91136;state.partyPos[i][1]=i==0?53034:45568;
        state.partyPos[i][2]=0;state.partyPos[i][3]=0;
        state.move[i]=3;state.action[i]=1;state.partyHp[i]=i==0?73:i==1?64:56;
        j=state.roster.deployed[i];state.roster.heroHp[j]=state.partyHp[i];
        state.roster.heroMove[j]=3;state.roster.heroAction[j]=1;
    }
    memset(bytes,0xff,sizeof(bytes));
    if(!FieldSaveEncode(&state,generation+1,bytes))return 6;
    file=fopen(argv[2],"wb");if(!file)return 7;
    if(fwrite(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 8;}fclose(file);
    return 0;
}
