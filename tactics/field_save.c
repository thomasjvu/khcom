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
    int i,j,k,n;
    if(s->floor>2||s->room>=12||s->party>2||!s->hp||s->hp>80||s->guard>2||!FieldDeckValid(&s->deck)||!FieldRosterValid(&s->roster))return 0;
    if(s->climbing>1||s->climbTarget<-0x100000||s->climbTarget>0x100000||
        (s->climbing&&s->climbAngle!=45&&s->climbAngle!=211))return 0;
    for(i=0;i<12;i++)if(s->roomEnemies[i]>FIELD_SAVE_ENEMIES)return 0;
    for(i=0;i<3;i++) {
        if(s->move[i]>3||s->action[i]>1||s->partyHp[i]>FieldHeroMaxHp(s->roster.deployed[i]))return 0;
        for(j=0;j<4;j++)if(s->partyPos[i][j]<-0x100000||s->partyPos[i][j]>0x100000)return 0;
    }
    for(k=0;k<12;k++) {
        if(s->roomCached[k]>1)return 0;
        n=0;
        for(i=0;i<FIELD_SAVE_ENEMIES;i++) {
            const FieldEncounter* e=&s->encounters[k][i];
            if(e->hp>64||(e->kind&7)>6||e->kind>15||((e->kind&8)&&(e->kind&7)!=2))return 0;
            if(e->hp)n++;
            for(j=0;j<4;j++)if(e->pos[j]<-4096||e->pos[j]>4096)return 0;
        }
        if(s->roomCached[k]&&n!=s->roomEnemies[k])return 0;
    }
    return s->roomCached[s->room]&&s->partyHp[0]&&s->partyHp[s->party]==s->hp;
}
static int Pack(const FieldSaveState* s,unsigned char* p) {
    int n=0,i,j,k;
#define BYTE(v) p[n++]=(unsigned char)(v)
#define WORD(v) Put(p+n,(unsigned int)(v));n+=4
    WORD(s->seed);
    BYTE(s->floor);BYTE(s->room);BYTE(s->party);BYTE(s->hp);BYTE(s->guard);
    WORD(s->turn);WORD(s->kills);WORD(s->chests);
    for(i=0;i<12;i++){WORD(s->roomFlags[i]);BYTE(s->roomEnemies[i]);}
    BYTE(s->deck.count);BYTE(s->deck.selected);
    BYTE(s->deck.stocked);for(i=0;i<3;i++){BYTE(s->deck.stock[i]);}
    for(i=0;i<FIELD_DECK_MAX;i++){BYTE(s->deck.kind[i]);BYTE(s->deck.value[i]);BYTE(s->deck.pile[i]);}
    for(i=0;i<3;i++){for(j=0;j<4;j++){WORD(s->partyPos[i][j]);}BYTE(s->move[i]);BYTE(s->action[i]);BYTE(s->partyHp[i]);}
    for(k=0;k<12;k++) {
        BYTE(s->roomCached[k]);
        for(i=0;i<FIELD_SAVE_ENEMIES;i++) {
            for(j=0;j<4;j++) {
                unsigned short v=(unsigned short)s->encounters[k][i].pos[j];
                BYTE(v);BYTE(v>>8);
            }
            BYTE(s->encounters[k][i].hp);BYTE(s->encounters[k][i].kind);
        }
    }
    BYTE(s->climbing);BYTE(s->climbAngle);WORD(s->climbTarget);
    BYTE(s->roster.unlocked);
    for(i=0;i<3;i++){BYTE(s->roster.deployed[i]);}
    for(i=0;i<FIELD_HEROES;i++){BYTE(s->roster.power[i]);BYTE(s->roster.sleights[i]);}
    BYTE(s->roster.cleared);BYTE(s->roster.cleared>>8);
    BYTE(s->roster.phase);BYTE(s->roster.reward);BYTE(s->roster.room);
    for(i=0;i<FIELD_HEROES;i++){BYTE(s->roster.heroHp[i]);}
    for(i=0;i<FIELD_HEROES;i++){BYTE(s->roster.heroMove[i]);BYTE(s->roster.heroAction[i]);}
    for(i=0;i<FIELD_HEROES;i++){BYTE(s->roster.heroAngle[i]);}
#undef BYTE
#undef WORD
    return n;
}
static int Unpack(FieldSaveState* s,const unsigned char* p,int version) {
    int n=0,i,j,k,heroes=version>=12?FIELD_HEROES:4;
#define BYTE(v) v=p[n++]
#define WORD(v) v=Get(p+n);n+=4
    WORD(s->seed);
    BYTE(s->floor);BYTE(s->room);BYTE(s->party);BYTE(s->hp);BYTE(s->guard);
    WORD(s->turn);WORD(s->kills);WORD(s->chests);
    for(i=0;i<12;i++){WORD(s->roomFlags[i]);BYTE(s->roomEnemies[i]);}
    BYTE(s->deck.count);BYTE(s->deck.selected);
    BYTE(s->deck.stocked);for(i=0;i<3;i++){BYTE(s->deck.stock[i]);}
    for(i=0;i<FIELD_DECK_MAX;i++){BYTE(s->deck.kind[i]);BYTE(s->deck.value[i]);BYTE(s->deck.pile[i]);}
    for(i=0;i<3;i++){for(j=0;j<4;j++){WORD(s->partyPos[i][j]);}BYTE(s->move[i]);BYTE(s->action[i]);BYTE(s->partyHp[i]);}
    for(k=0;k<12;k++) {
        BYTE(s->roomCached[k]);
        for(i=0;i<FIELD_SAVE_ENEMIES;i++) {
            for(j=0;j<4;j++) {
                unsigned int v=p[n]|((unsigned int)p[n+1]<<8);n+=2;
                s->encounters[k][i].pos[j]=v>=0x8000u?(int)v-65536:(int)v;
            }
            BYTE(s->encounters[k][i].hp);BYTE(s->encounters[k][i].kind);
        }
    }
    BYTE(s->climbing);BYTE(s->climbAngle);WORD(s->climbTarget);
    FieldRosterInit(&s->roster);
    if(version>=9) {
        BYTE(s->roster.unlocked);
        for(i=0;i<3;i++){BYTE(s->roster.deployed[i]);}
        for(i=0;i<heroes;i++){BYTE(s->roster.power[i]);BYTE(s->roster.sleights[i]);}
        s->roster.cleared=p[n]|((unsigned int)p[n+1]<<8);n+=2;
        BYTE(s->roster.phase);BYTE(s->roster.reward);BYTE(s->roster.room);
    }
    if(version>=10) {
        for(i=0;i<heroes;i++){BYTE(s->roster.heroHp[i]);}
        for(i=0;i<heroes;i++){BYTE(s->roster.heroMove[i]);BYTE(s->roster.heroAction[i]);}
    }
    if(version>=11)for(i=0;i<heroes;i++){BYTE(s->roster.heroAngle[i]);}
#undef BYTE
#undef WORD
    if(version<12)s->roster.unlocked|=1<<FIELD_RALLY;
    return n;
}
int FieldSaveEncode(const FieldSaveState* s,unsigned int gen,unsigned char* out) {
    int i,n;if(!Valid(s))return 0;
    for(i=0;i<FIELD_SAVE_SIZE;i++)out[i]=0;
    out[0]='K';out[1]='T';out[2]='F';out[3]='S';out[4]=12;
    Put(out+8,gen);n=Pack(s,out+16);if(n>FIELD_SAVE_SIZE-20)return 0;out[6]=n&255;out[7]=n>>8;
    Put(out+FIELD_SAVE_SIZE-4,Crc(out));return 1;
}
int FieldSaveDecode(FieldSaveState* s,unsigned int* gen,const unsigned char* data) {
    FieldSaveState candidate;int n;
    if(data[0]!='K'||data[1]!='T'||data[2]!='F'||data[3]!='S'||(data[4]!=8&&data[4]!=9&&data[4]!=10&&data[4]!=11&&data[4]!=12)||data[5]||Get(data+FIELD_SAVE_SIZE-4)!=Crc(data))return 0;
    n=Unpack(&candidate,data+16,data[4]);
    if(n!=(data[6]|data[7]<<8)||!Valid(&candidate))return 0;
    *s=candidate;*gen=Get(data+8);return 1;
}
int FieldSaveSelect(FieldSaveState* s,unsigned int* gen,const unsigned char* a,const unsigned char* b) {
    unsigned int gb;int va;
    /* Decode has one bounded candidate. Keeping two full room states here
     * would overflow the GBA's IWRAM stack when called through native boot. */
    va=FieldSaveDecode(s,gen,a);
    if(!va)return FieldSaveDecode(s,gen,b)?1:-1;
    gb=Get(b+8);
    if(gb!=*gen&&(gb-*gen)<0x80000000u&&FieldSaveDecode(s,gen,b))return 1;
    return 0;
}
