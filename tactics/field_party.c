#include "field_party.h"
void FieldPartyInit(FieldParty* p) {
    int i;
    p->maxHp[0]=80;p->maxHp[1]=56;p->maxHp[2]=72;
    for(i=0;i<3;i++)p->hp[i]=p->maxHp[i];
}
int FieldPartyDamage(FieldParty* p,int member,int amount) {
    if(member<0||member>2||amount<0)return 0;
    p->hp[member]=amount>=p->hp[member]?0:p->hp[member]-amount;
    /* Sora's defeat ends the run; friends can be revived by Cure/chests. */
    return p->hp[0]==0;
}
int FieldPartyHeal(FieldParty* p,int member,int amount) {
    int missing;
    if(member<0||member>2||amount<0)return 0;
    missing=p->maxHp[member]-p->hp[member];
    if(amount>missing)amount=missing;
    p->hp[member]+=amount;return amount;
}
int FieldPartyNext(const FieldParty* p,int member) {
    int i;
    if(member<0||member>2)return -1;
    for(i=0;i<3;i++){member=(member+1)%3;if(p->hp[member])return member;}
    return -1;
}
int FieldPartyValid(const FieldParty* p) {
    return p->maxHp[0]==80&&(p->maxHp[1]==56||p->maxHp[1]==72)&&(p->maxHp[2]==56||p->maxHp[2]==72)&&
        p->hp[0]<=80&&p->hp[1]<=p->maxHp[1]&&p->hp[2]<=p->maxHp[2];
}
