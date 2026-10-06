#include "save.h"
static void Put32(TacByte *p,TacWord n) {int i;for(i=0;i<4;i++){p[i]=(TacByte)n;n>>=8;}}
static TacWord Get32(const TacByte *p) {return (TacWord)p[0]|((TacWord)p[1]<<8)|((TacWord)p[2]<<16)|((TacWord)p[3]<<24);}
static TacWord Checksum(const TacByte *p) {
    TacWord crc=0xffffffffu;int i,j;
    for(i=0;i<TACTICS_SAVE_SIZE-4;i++) {crc^=p[i];for(j=0;j<8;j++) crc=(crc>>1)^((crc&1)?0xedb88320u:0);}
    return crc^0xffffffffu;
}
static int Pack(const TacticsState *s,TacByte *p) {
    int pos=0,i;
#define COPY(field) for(i=0;i<(int)sizeof(s->field);i++) p[pos++]=((const TacByte *)&s->field)[i]
    Put32(p,s->seed);Put32(p+4,s->rng);pos=8;
    COPY(blocked);COPY(units);COPY(intents);COPY(deck);COPY(rewards);COPY(piles);COPY(routes);COPY(terrain);
    p[pos++]=s->deckCount;p[pos++]=s->stage;p[pos++]=s->room;p[pos++]=s->phase;
    p[pos++]=s->moved;p[pos++]=s->acted;p[pos++]=s->shield;p[pos++]=s->turn;
    p[pos++]=s->maxHp;p[pos++]=s->hp;p[pos++]=s->power;p[pos++]=s->victories;
#undef COPY
    return pos;
}
static int Unpack(TacticsState *s,const TacByte *p) {
    int pos=0,i;
#define COPY(field) for(i=0;i<(int)sizeof(s->field);i++) ((TacByte *)&s->field)[i]=p[pos++]
    for(i=0;i<(int)sizeof(*s);i++) ((TacByte *)s)[i]=0;
    s->seed=Get32(p);s->rng=Get32(p+4);pos=8;
    COPY(blocked);COPY(units);COPY(intents);COPY(deck);COPY(rewards);COPY(piles);COPY(routes);COPY(terrain);
    s->deckCount=p[pos++];s->stage=p[pos++];s->room=p[pos++];s->phase=p[pos++];
    s->moved=p[pos++];s->acted=p[pos++];s->shield=p[pos++];s->turn=p[pos++];
    s->maxHp=p[pos++];s->hp=p[pos++];s->power=p[pos++];s->victories=p[pos++];
#undef COPY
    return pos;
}
int TacticsSaveEncode(const TacticsState *s,TacWord generation,TacByte *out) {
    int i,n;
    if(!TacticsValid(s)) return 0;
    for(i=0;i<TACTICS_SAVE_SIZE;i++)out[i]=0;
    out[0]='K';out[1]='T';out[2]='A';out[3]='C';out[4]=1;
    Put32(out+8,generation);n=Pack(s,out+16);out[6]=n&255;out[7]=n>>8;
    Put32(out+TACTICS_SAVE_SIZE-4,Checksum(out));return 1;
}
int TacticsSaveDecode(TacticsState *s,TacWord *generation,const TacByte *data) {
    TacticsState candidate;int n;
    if(data[0]!='K'||data[1]!='T'||data[2]!='A'||data[3]!='C'||data[4]!=1||data[5]!=0||
        Get32(data+TACTICS_SAVE_SIZE-4)!=Checksum(data))return 0;
    n=Unpack(&candidate,data+16);
    if(n!=(data[6]|data[7]<<8)||!TacticsValid(&candidate)) return 0;
    *s=candidate;*generation=Get32(data+8);return 1;
}
int TacticsSaveSelect(TacticsState *s,TacWord *generation,const TacByte *a,const TacByte *b) {
    TacticsState first,second;TacWord ga=0,gb=0;int va,vb;
    va=TacticsSaveDecode(&first,&ga,a);vb=TacticsSaveDecode(&second,&gb,b);
    if(!va&&!vb)return -1;
    if(vb&&(!va||(gb!=ga && (gb-ga)<0x80000000u))) {*s=second;*generation=gb;return 1;}
    *s=first;*generation=ga;return 0;
}
