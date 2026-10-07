#include <assert.h>
#include <stdio.h>
#include "field_party.h"
int main(void) {
    FieldParty p;
    FieldPartyInit(&p);assert(FieldPartyValid(&p));
    assert(p.hp[0]==80&&p.hp[1]==56&&p.hp[2]==72);
    assert(!FieldPartyDamage(&p,1,60)&&p.hp[1]==0&&p.hp[0]==80);
    assert(FieldPartyNext(&p,0)==2);assert(FieldPartyNext(&p,-2)==-1);
    assert(FieldPartyHeal(&p,1,23)==23&&p.hp[1]==23);
    assert(FieldPartyNext(&p,0)==1);
    assert(FieldPartyHeal(&p,1,99)==33&&p.hp[1]==56);
    assert(FieldPartyDamage(&p,0,80)&&p.hp[0]==0);
    assert(!FieldPartyDamage(&p,3,10));assert(!FieldPartyHeal(&p,0,-1));
    assert(FieldPartyValid(&p));p.hp[1]=57;assert(!FieldPartyValid(&p));
    puts("field party: separate HP, knockouts, revival, selection and leader defeat passed");
    return 0;
}
