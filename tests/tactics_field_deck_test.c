#include <assert.h>
#include <stdio.h>
#include "field_deck.h"
int main(void) {
    FieldDeck d;
    int i, card;
    FieldDeckInit(&d);
    assert(FieldDeckValid(&d));
    assert(FieldDeckHand(&d, 4) == 4 && FieldDeckHand(&d, 5) == -1);
    FieldDeckCycle(&d, -1);assert(d.selected == 4);
    FieldDeckCycle(&d, 1);assert(d.selected == 0);
    for (i = 0; i < 12; i++) {
        card = FieldDeckPlay(&d);assert(card >= 0);
        assert(d.pile[card] == 2 && FieldDeckValid(&d));
    }
    assert(FieldDeckPlay(&d) == -1);
    assert(FieldDeckReload(&d));assert(FieldDeckValid(&d));
    assert(FieldDeckHand(&d, 4) >= 0);
    for (i = 0; i < 12; i++) assert(FieldDeckReward(&d, i * 983u));
    assert(!FieldDeckReward(&d, 1));assert(FieldDeckValid(&d));
    d.kind[0] = 4;assert(!FieldDeckValid(&d));
    puts("field deck: depletion, reload, values, selection, rewards passed");
    return 0;
}
