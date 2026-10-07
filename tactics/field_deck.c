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
    int i, n = 0;
    if (d->count < 1 || d->count > FIELD_DECK_MAX) return 0;
    for (i = 0; i < d->count; i++) {
        if (d->kind[i] > 3 || d->value[i] > 9 || d->pile[i] > 2) return 0;
        if (d->pile[i] == 1) n++;
    }
    return n <= FIELD_HAND_MAX && (!n || d->selected < n);
}
