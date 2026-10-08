/* Read-only diagnostics. The production encoder remains authoritative. */
#include <stdio.h>
#include "field_save.h"
#include <string.h>
typedef struct NativeEncounter {short pos[4];unsigned char hp,kind,pad[2];} NativeEncounter;
typedef struct NativeSaveState {
 unsigned int seed,roomFlags[12];unsigned char roomEnemies[12];
 FieldDeck deck;unsigned char deckPadding[2];FieldRoster roster;int partyPos[3][4];
 NativeEncounter encounters[12][6];unsigned char roomCached[12],move[3],action[3],partyHp[3];
 unsigned char floor,room,party,hp,guard;unsigned short turn,kills,chests;
 int climbTarget;unsigned char climbing,climbAngle;
} NativeSaveState;
typedef char NativeSize[(sizeof(NativeSaveState)==1136)?1:-1];
int main(int argc,char** argv) {
    FieldSaveState state,decoded;NativeSaveState native;
    unsigned char bytes[32768],encoded[FIELD_SAVE_SIZE];
    unsigned int generation=0,roundtrip=0;
    FILE* file;size_t size;int i,j,k,living,valid;
    if(argc!=2)return 2;
    file=fopen(argv[1],"rb");if(!file)return 2;
    size=fread(bytes,1,sizeof(bytes),file);fclose(file);
    if(size==32768) {
        if(FieldSaveSelect(&state,&generation,bytes,bytes+FIELD_SAVE_SIZE)<0)return 3;
    } else if(size==sizeof(native)) {
        memcpy(&native,bytes,sizeof(native));
#define COPY(field) memcpy(&state.field,&native.field,sizeof(state.field))
        COPY(seed);COPY(roomFlags);COPY(roomEnemies);COPY(deck);COPY(roster);COPY(partyPos);
        COPY(roomCached);COPY(move);COPY(action);COPY(partyHp);COPY(floor);COPY(room);COPY(party);
        COPY(hp);COPY(guard);COPY(turn);COPY(kills);COPY(chests);COPY(climbTarget);COPY(climbing);COPY(climbAngle);
#undef COPY
        for(k=0;k<12;k++)for(i=0;i<6;i++) {
            memcpy(state.encounters[k][i].pos,native.encounters[k][i].pos,sizeof(state.encounters[k][i].pos));
            state.encounters[k][i].hp=native.encounters[k][i].hp;state.encounters[k][i].kind=native.encounters[k][i].kind;
        }
    } else {printf("Unexpected size %lu; native state expects %lu\n",(unsigned long)size,(unsigned long)sizeof(native));return 3;}
    printf("floor=%u room=%u party=%u hp=%u guard=%u deck_valid=%d roster_valid=%d\n",state.floor,state.room,state.party,state.hp,state.guard,FieldDeckValid(&state.deck),FieldRosterValid(&state.roster));
    for(i=0;i<3;i++)printf("party%d hero=%u hp=%u move=%u action=%u\n",i,state.roster.deployed[i],state.partyHp[i],state.move[i],state.action[i]);
    for(k=0;k<12;k++) {
        living=0;
        for(i=0;i<FIELD_SAVE_ENEMIES;i++) {
            FieldEncounter* e=&state.encounters[k][i];if(e->hp)living++;
            if(e->hp>64||(e->kind&7)>6||e->kind>15||((e->kind&8)&&(e->kind&7)!=2))printf("INVALID room%d enemy%d hp=%u kind=%u\n",k,i,e->hp,e->kind);
            for(j=0;j<4;j++)if(e->pos[j]<-4096||e->pos[j]>4096)printf("INVALID room%d enemy%d coordinate%d=%d\n",k,i,j,e->pos[j]);
        }
        if(state.roomCached[k]&&living!=state.roomEnemies[k])printf("INVALID cached room%d living=%d expected=%u\n",k,living,state.roomEnemies[k]);
    }
    valid=FieldSaveEncode(&state,generation+1,encoded);
    if(valid)valid=FieldSaveDecode(&decoded,&roundtrip,encoded)&&roundtrip==generation+1;
    printf("Production encode/decode: %s\n",valid?"PASS":"REJECTED");return valid?0:1;
}
