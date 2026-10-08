/* Diagnostic only: reproduce the recorded Agrabah room5 door approach.
 * This writes an explicit suspend fixture, not campaign-completion evidence. */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "field_save.h"
#include "field_enemy.h"
#include "worldgen.h"
typedef char RosterFixtureSize[(offsetof(FieldRoster,heroAngle)+FIELD_HEROES==39)?1:-1];
typedef char DeckFixtureSize[(sizeof(FieldDeck)==78)?1:-1];
int main(int argc,char** argv) {
    unsigned char bytes[32768];FieldSaveState state;unsigned int generation;
    FILE* file;int i,j;
    if(argc!=3)return 1;
    file=fopen(argv[1],"rb");if(!file)return 2;
    if(fread(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 3;}fclose(file);
    if(FieldSaveSelect(&state,&generation,bytes,bytes+FIELD_SAVE_SIZE)<0)return 4;
    if(state.seed!=4411213u)return 5;
    state.floor=1;state.room=5;state.party=0;state.hp=71;state.guard=0;
    {
        const unsigned char roster[39]={31,0,3,2,6,0,0,0,0,0,0,0,0,0,12,14,1,0,5,71,56,12,12,64,2,3,3,3,3,1,1,1,1,1,83,0,0,0,0};
        const unsigned char deck[78]={0,1,2,3,0,1,2,3,0,1,2,3,0,2,0,3,1,3,2,3,0,1,2,3,5,6,7,8,0,5,6,7,8,9,5,6,6,8,9,7,9,5,8,9,5,6,7,8,2,2,2,1,1,2,2,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,4,0,0,0,0};
        memcpy(&state.roster,roster,sizeof(roster));
        memcpy(&state.deck,deck,sizeof(state.deck));
    }
    state.climbing=0;state.climbTarget=0;state.climbAngle=0;
    state.roster.phase=FIELD_BATTLE;state.roster.reward=0;state.roster.room=5;
    for(i=0;i<12;i++) {
        state.roomFlags[i]=1; /* FLOOR_ROOM_FLAG_CREATED: preserve native open doors. */state.roomEnemies[i]=0;state.roomCached[i]=0;
        memset(state.encounters[i],0,sizeof(state.encounters[i]));
    }
    state.roomCached[5]=1;
    {
        struct TacWorld world;
        const short positions[3][4]={{432,288,0,0},{448,296,0,0},{416,280,0,0}};
        const unsigned char health[3]={8,12,8};
        TacWorldGenerate(&world,state.seed,1);
        state.roomEnemies[5]=3;
        for(i=0;i<3;i++) {
            memcpy(state.encounters[5][i].pos,positions[i],sizeof(positions[i]));
            state.encounters[5][i].hp=health[i];
            state.encounters[5][i].kind=FieldEnemyKind(world.rooms[5].seed,1,5,i);
        }
    }
    for(i=0;i<3;i++) {
        state.partyPos[i][0]=i==0?40612:31744;state.partyPos[i][1]=i==0?84592:78336;
        state.partyPos[i][2]=8192;state.partyPos[i][3]=8192;
        state.move[i]=i==0?2:3;state.action[i]=1;state.partyHp[i]=i==0?71:12;
        j=state.roster.deployed[i];state.roster.heroHp[j]=state.partyHp[i];
        state.roster.heroMove[j]=state.move[i];state.roster.heroAction[j]=1;
    }
    memset(bytes,0xff,sizeof(bytes));
    if(!FieldSaveEncode(&state,generation+1,bytes))return 6;
    file=fopen(argv[2],"wb");if(!file)return 7;
    if(fwrite(bytes,1,sizeof(bytes),file)!=sizeof(bytes)){fclose(file);return 8;}fclose(file);
    return 0;
}
