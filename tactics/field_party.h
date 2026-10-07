#ifndef FIELD_PARTY_H
#define FIELD_PARTY_H
typedef struct FieldParty {
    unsigned char hp[3];
    unsigned char maxHp[3];
} FieldParty;
void FieldPartyInit(FieldParty* party);
int FieldPartyDamage(FieldParty* party, int member, int amount);
int FieldPartyHeal(FieldParty* party, int member, int amount);
int FieldPartyNext(const FieldParty* party, int member);
int FieldPartyValid(const FieldParty* party);
#endif
