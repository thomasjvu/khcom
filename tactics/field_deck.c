#include "field_deck.h"
static void Draw(FieldDeck* d) {
    int i, n = 0;
    for (i = 0; i < d->count; i++) if (d->pile[i] == 1) n++;
    for (i = 0; i < d->count && n < FIELD_HAND_MAX; i++)
        if (d->pile[i] == 0) {d->pile[i] = 1; n++;}
}
void FieldDeckInit(FieldDeck* d) {
    int i;
    for (i = 0; i < FIELD_DECK_MAX; i++) {
        d->kind[i] = i % 4;
        d->value[i] = i == 4 ? 0 : 5 + i % 5;
        d->pile[i] = 0;
    }
    d->stocked=0;d->stock[0]=d->stock[1]=d->stock[2]=0;
    d->count = 12; d->selected = 0; Draw(d);
}
int FieldDeckHand(const FieldDeck* d, int slot) {
    int i;
    for (i = 0; i < d->count; i++) if (d->pile[i] == 1) {
        if (!slot) return i;
        slot--;
    }
    return -1;
}
int FieldDeckPlay(FieldDeck* d) {
    int i = FieldDeckHand(d, d->selected);
    if (i < 0) return -1;
    d->pile[i] = 2; Draw(d);
    while (d->selected && FieldDeckHand(d, d->selected) < 0) d->selected--;
    return i;
}
int FieldDeckStock(FieldDeck* d) {
    int i;
    if(d->stocked>=3)return 0;
    i=FieldDeckHand(d,d->selected);if(i<0)return 0;
    d->stock[d->stocked++]=i;d->pile[i]=4;Draw(d);
    while(d->selected&&FieldDeckHand(d,d->selected)<0)d->selected--;
    return 1;
}
int FieldDeckRecipe(const FieldDeck* d) {
    int kind;
    if(d->stocked!=3)return 0;
    kind=d->kind[d->stock[0]];
    if(d->kind[d->stock[1]]!=kind||d->kind[d->stock[2]]!=kind)return 0;
    return kind+1;
}
int FieldDeckSleightPreview(const FieldDeck* d,int* kind,int* value) {
    int i,counts[4]={0,0,0,0};
    if(d->stocked!=3)return 0;
    *value=0;*kind=0;
    for(i=0;i<3;i++){counts[d->kind[d->stock[i]]]++;*value+=d->value[d->stock[i]];}
    for(i=1;i<4;i++)if(counts[i]>counts[*kind])*kind=i;
    return 1;
}
int FieldDeckSleight(FieldDeck* d,int* kind,int* value) {
    if(!FieldDeckSleightPreview(d,kind,value))return 0;
    d->pile[d->stock[0]]=3;
    d->pile[d->stock[1]]=d->pile[d->stock[2]]=2;
    d->stocked=0;return 1;
}
void FieldDeckCancelStock(FieldDeck* d) {
    int i;
    for(i=0;i<d->stocked;i++)d->pile[d->stock[i]]=0;
    d->stocked=0;Draw(d);
}
void FieldDeckCycle(FieldDeck* d, int direction) {
    int n = 0;
    while (FieldDeckHand(d, n) >= 0) n++;
    if (n) d->selected = (d->selected + n + direction) % n;
}
int FieldDeckReload(FieldDeck* d) {
    int i, changed = 0;
    for (i = 0; i < d->count; i++) if (d->pile[i] == 2) {d->pile[i] = 0;changed = 1;}
    Draw(d); return changed;
}
int FieldDeckReward(FieldDeck* d, unsigned int seed) {
    int i = d->count;
    if (i >= FIELD_DECK_MAX) return 0;
    d->kind[i] = seed % 4; d->value[i] = 5 + (seed >> 8) % 5;
    d->pile[i] = 0; d->count++; Draw(d); return 1;
}
int FieldDeckValid(const FieldDeck* d) {
    int i,j,n=0,stocked=0;
    if(d->stocked>3)return 0;
    for(i=0;i<d->stocked;i++){
        if(d->stock[i]>=d->count||d->pile[d->stock[i]]!=4)return 0;
        for(j=0;j<i;j++)if(d->stock[j]==d->stock[i])return 0;
    }
    if (d->count < 1 || d->count > FIELD_DECK_MAX) return 0;
    for (i = 0; i < d->count; i++) {
        if (d->kind[i] > 3 || d->value[i] > 9 || d->pile[i] > 4) return 0;
        if (d->pile[i] == 1) n++;
        if(d->pile[i]==4)stocked++;
    }
    if(stocked!=d->stocked)return 0;
    return n <= FIELD_HAND_MAX && (!n || d->selected < n);
}
