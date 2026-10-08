#include "field_roster.h"
int FieldHeroMaxHp(int hero) {return hero==0?80:hero==1?56:hero==2||hero==3?72:hero==4?64:0;}
void FieldRosterInit(FieldRoster* r) {
    int i;r->unlocked=23;r->cleared=0;r->phase=FIELD_ASSEMBLY;r->reward=0;r->room=0;
    for(i=0;i<3;i++)r->deployed[i]=i;
    for(i=0;i<FIELD_HEROES;i++){r->power[i]=0;r->sleights[i]=0;r->heroHp[i]=FieldHeroMaxHp(i);r->heroMove[i]=3;r->heroAction[i]=1;r->heroAngle[i]=0;}
}
int FieldRosterValid(const FieldRoster* r) {
    int i,j;
    if(r->deployed[0]!=FIELD_SORA)return 0;
    if(!(r->unlocked&1)||r->unlocked>31||r->phase>FIELD_REWARD||r->reward>FIELD_REWARD_BOSS||r->room>=12||r->cleared>4095)return 0;
    if((r->phase==FIELD_REWARD)!=(r->reward!=0))return 0;
    for(i=0;i<3;i++) {
        if(r->deployed[i]>=FIELD_HEROES||!(r->unlocked&(1<<r->deployed[i])))return 0;
        for(j=0;j<i;j++)if(r->deployed[i]==r->deployed[j])return 0;
    }
    for(i=0;i<FIELD_HEROES;i++)if(r->power[i]>8||r->sleights[i]>7||r->heroHp[i]>FieldHeroMaxHp(i)||r->heroMove[i]>3||r->heroAction[i]>1)return 0;
    return 1;
}
int FieldRosterDeploy(FieldRoster* r,int slot,int hero) {
    int i,old;
    if(!FieldRosterValid(r)||r->phase!=FIELD_ASSEMBLY||slot<1||slot>2||hero<1||hero>=FIELD_HEROES||!(r->unlocked&(1<<hero)))return 0;
    old=r->deployed[slot];
    for(i=0;i<3;i++)if(r->deployed[i]==hero)r->deployed[i]=old;
    r->deployed[slot]=hero;return 1;
}
int FieldRosterBegin(FieldRoster* r,int room) {
    if(!FieldRosterValid(r)||r->phase!=FIELD_ASSEMBLY||room<0||room>=12)return 0;
    r->room=room;r->phase=FIELD_BATTLE;return 1;
}
int FieldRosterClear(FieldRoster* r,int boss) {
    unsigned short bit;int i,n=0;
    if(!FieldRosterValid(r)||r->phase!=FIELD_BATTLE||(boss!=0&&boss!=1))return 0;
    bit=(unsigned short)(1<<r->room);
    if(r->cleared&bit){r->phase=FIELD_ASSEMBLY;return FIELD_REWARD_NONE;}
    r->cleared|=bit;
    for(i=0;i<12;i++)if(r->cleared&(1<<i))n++;
    r->reward=boss?FIELD_REWARD_BOSS:(n%2==0?FIELD_REWARD_UPGRADE:FIELD_REWARD_NONE);
    r->phase=r->reward?FIELD_REWARD:FIELD_ASSEMBLY;return r->reward;
}
int FieldRosterUpgrade(FieldRoster* r,int hero,int sleight) {
    if(!FieldRosterValid(r)||r->phase!=FIELD_REWARD||hero<0||hero>=FIELD_HEROES||!(r->unlocked&(1<<hero))||sleight<0||sleight>3)return 0;
    if(sleight) {
        if(r->sleights[hero]&(1<<(sleight-1)))return 0;
        r->sleights[hero]|=1<<(sleight-1);
    } else {
        if(r->power[hero]>=8)return 0;
        r->power[hero]++;
    }
    r->reward=0;r->phase=FIELD_ASSEMBLY;return 1;
}
int FieldRosterRecruit(FieldRoster* r,int hero) {
    if(!FieldRosterValid(r)||r->phase!=FIELD_REWARD||r->reward!=FIELD_REWARD_BOSS||hero<0||hero>=FIELD_HEROES||(r->unlocked&(1<<hero)))return 0;
    r->unlocked|=1<<hero;r->reward=0;r->phase=FIELD_ASSEMBLY;return 1;
}
int FieldRosterRest(FieldRoster* r) {
    int i;
    if(!FieldRosterValid(r)||r->phase!=FIELD_ASSEMBLY)return 0;
    for(i=0;i<FIELD_HEROES;i++)if(r->unlocked&(1<<i)) {
        r->heroHp[i]=FieldHeroMaxHp(i);r->heroMove[i]=3;r->heroAction[i]=1;
    }
    return 1;
}
