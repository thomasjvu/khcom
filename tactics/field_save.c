#include "field_save.h"
static void Put(unsigned char* p, unsigned int n) {
    int i;for(i=0;i<4;i++){p[i]=n&255;n>>=8;}
}
static unsigned int Get(const unsigned char* p) {
    return (unsigned int)p[0]|((unsigned int)p[1]<<8)|((unsigned int)p[2]<<16)|((unsigned int)p[3]<<24);
}
static unsigned int Crc(const unsigned char* p) {
    unsigned int c=0xffffffffu;int i,j;
    for(i=0;i<FIELD_SAVE_SIZE-4;i++){c^=p[i];for(j=0;j<8;j++)c=(c>>1)^((c&1)?0xedb88320u:0);}
    return c^0xffffffffu;
}
static int Valid(const FieldSaveState* s) {
    int i,j,n=0;
    if(s->floor>2||s->room>=12||s->party>2||!s->hp||s->hp>80||s->guard>2||!FieldDeckValid(&s->deck))return 0;
    for(i=0;i<12;i++)if(s->roomEnemies[i]>FIELD_SAVE_ENEMIES)return 0;
    for(i=0;i<3;i++) {
        if(s->move[i]>3||s->action[i]>1)return 0;
        for(j=0;j<4;j++)if(s->partyPos[i][j]<-0x100000||s->partyPos[i][j]>0x100000)return 0;
    }
    for(i=0;i<FIELD_SAVE_ENEMIES;i++) {
        if(s->enemyHp[i]>64||s->enemyKind[i]>1)return 0;
        if(s->enemyHp[i])n++;
        for(j=0;j<4;j++)if(s->enemyPos[i][j]<-0x100000||s->enemyPos[i][j]>0x100000)return 0;
    }
    return n==s->roomEnemies[s->room];
}
static int Pack(const FieldSaveState* s,unsigned char* p) {
    int n=0,i,j;
#define BYTE(v) p[n++]=(unsigned char)(v)
#define WORD(v) Put(p+n,(unsigned int)(v));n+=4
    WORD(s->seed);
    BYTE(s->floor);BYTE(s->room);BYTE(s->party);BYTE(s->hp);BYTE(s->guard);
    WORD(s->turn);WORD(s->kills);WORD(s->chests);
    for(i=0;i<12;i++){WORD(s->roomFlags[i]);BYTE(s->roomEnemies[i]);}
    BYTE(s->deck.count);BYTE(s->deck.selected);
    for(i=0;i<FIELD_DECK_MAX;i++){BYTE(s->deck.kind[i]);BYTE(s->deck.value[i]);BYTE(s->deck.pile[i]);}
    for(i=0;i<3;i++){for(j=0;j<4;j++){WORD(s->partyPos[i][j]);}BYTE(s->move[i]);BYTE(s->action[i]);}
    for(i=0;i<FIELD_SAVE_ENEMIES;i++){for(j=0;j<4;j++){WORD(s->enemyPos[i][j]);}BYTE(s->enemyHp[i]);BYTE(s->enemyKind[i]);}
#undef BYTE
#undef WORD
    return n;
}
static int Unpack(FieldSaveState* s,const unsigned char* p) {
    int n=0,i,j;
#define BYTE(v) v=p[n++]
#define WORD(v) v=Get(p+n);n+=4
    WORD(s->seed);
    BYTE(s->floor);BYTE(s->room);BYTE(s->party);BYTE(s->hp);BYTE(s->guard);
    WORD(s->turn);WORD(s->kills);WORD(s->chests);
    for(i=0;i<12;i++){WORD(s->roomFlags[i]);BYTE(s->roomEnemies[i]);}
    BYTE(s->deck.count);BYTE(s->deck.selected);
    for(i=0;i<FIELD_DECK_MAX;i++){BYTE(s->deck.kind[i]);BYTE(s->deck.value[i]);BYTE(s->deck.pile[i]);}
    for(i=0;i<3;i++){for(j=0;j<4;j++){WORD(s->partyPos[i][j]);}BYTE(s->move[i]);BYTE(s->action[i]);}
    for(i=0;i<FIELD_SAVE_ENEMIES;i++){for(j=0;j<4;j++){WORD(s->enemyPos[i][j]);}BYTE(s->enemyHp[i]);BYTE(s->enemyKind[i]);}
#undef BYTE
#undef WORD
    return n;
}
int FieldSaveEncode(const FieldSaveState* s,unsigned int gen,unsigned char* out) {
    int i,n;if(!Valid(s))return 0;
    for(i=0;i<FIELD_SAVE_SIZE;i++)out[i]=0;
    out[0]='K';out[1]='T';out[2]='F';out[3]='S';out[4]=3;
    Put(out+8,gen);n=Pack(s,out+16);out[6]=n&255;out[7]=n>>8;
    Put(out+508,Crc(out));return 1;
}
int FieldSaveDecode(FieldSaveState* s,unsigned int* gen,const unsigned char* data) {
    FieldSaveState candidate;int n;
    if(data[0]!='K'||data[1]!='T'||data[2]!='F'||data[3]!='S'||data[4]!=3||data[5]||Get(data+508)!=Crc(data))return 0;
    n=Unpack(&candidate,data+16);
    if(n!=(data[6]|data[7]<<8)||!Valid(&candidate))return 0;
    *s=candidate;*gen=Get(data+8);return 1;
}
int FieldSaveSelect(FieldSaveState* s,unsigned int* gen,const unsigned char* a,const unsigned char* b) {
    FieldSaveState first,second;unsigned int ga=0,gb=0;int va,vb;
    va=FieldSaveDecode(&first,&ga,a);vb=FieldSaveDecode(&second,&gb,b);
    if(!va&&!vb)return -1;
    if(vb&&(!va||(gb!=ga&&(gb-ga)<0x80000000u))){*s=second;*gen=gb;return 1;}
    *s=first;*gen=ga;return 0;
}
