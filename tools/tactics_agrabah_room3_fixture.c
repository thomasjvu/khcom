/* Diagnostic only: reproduce the recorded Agrabah room3 route stall with live party budgets.
 * This writes an explicit suspend fixture, not campaign-completion evidence. */
#include <stdio.h>
#include <string.h>
#include "field_save.h"
typedef char RosterFixtureSize[(sizeof(FieldRoster)==39)?1:-1];
typedef char DeckFixtureSize[(sizeof(FieldDeck)==78)?1:-1];
int main(int argc,char** argv) {
    unsigned char bytes[32768];FieldSaveState state;unsigned int generation;
    FILE* file;int i,j;
    if(argc!=3)return 1;
    file=fopen(argv[1],"rb");if(!file)return 2;
    if(fread(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 3;}fclose(file);
    if(FieldSaveSelect(&state,&generation,bytes,bytes+FIELD_SAVE_SIZE)<0)return 4;
    if(state.seed!=4411213u)return 5;
    state.floor=1;state.room=3;state.party=0;state.hp=66;state.guard=0;
    {
        const unsigned char roster[39]={31,0,3,2,2,0,0,0,0,0,0,0,0,0,12,0,0,0,3,66,56,72,44,64,3,3,3,3,3,1,1,1,1,1,192,0,0,0,0};
        const unsigned char deck[78]={0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,5,6,7,8,0,5,6,7,8,9,5,6,7,8,9,5,6,7,8,9,5,6,7,8,1,2,2,2,1,2,2,2,1,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,12,2,0,0,0,0};
        memcpy(&state.roster,roster,sizeof(state.roster));
        memcpy(&state.deck,deck,sizeof(state.deck));
    }
    state.climbing=0;state.climbTarget=0;state.climbAngle=0;
    state.roster.phase=FIELD_BATTLE;state.roster.reward=0;state.roster.room=3;
    for(i=0;i<12;i++) {
        state.roomFlags[i]=1; /* FLOOR_ROOM_FLAG_CREATED: preserve native open doors. */state.roomEnemies[i]=0;state.roomCached[i]=0;
        memset(state.encounters[i],0,sizeof(state.encounters[i]));
    }
    state.roomCached[3]=1;
    for(i=0;i<3;i++) {
        state.partyPos[i][0]=i==0?83500:115712;state.partyPos[i][1]=i==0?72836:53760;
        state.partyPos[i][2]=0;state.partyPos[i][3]=0;
        state.move[i]=3;state.action[i]=1;state.partyHp[i]=i==0?66:i==1?44:72;
        j=state.roster.deployed[i];state.roster.heroHp[j]=state.partyHp[i];
        state.roster.heroMove[j]=3;state.roster.heroAction[j]=1;
    }
    memset(bytes,0xff,sizeof(bytes));
    if(!FieldSaveEncode(&state,generation+1,bytes))return 6;
    file=fopen(argv[2],"wb");if(!file)return 7;
    if(fwrite(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 8;}fclose(file);
    return 0;
}
