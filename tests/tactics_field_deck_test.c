#include <assert.h>
#include <stdio.h>
#include <string.h>
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
    FieldDeckInit(&d);
    assert(FieldDeckStock(&d)&&FieldDeckStock(&d)&&FieldDeckStock(&d));
    assert(!FieldDeckStock(&d)&&FieldDeckValid(&d));
    {
        int kind,value;
        FieldDeck before;
        assert(FieldDeckRecipe(&d)==0);
        before=d;
        assert(FieldDeckSleightPreview(&d,&kind,&value)&&kind==0&&value==18);
        assert(memcmp(&before,&d,sizeof(d))==0);
        d.kind[d.stock[0]]=FIELD_CARD_FIRE;
        d.kind[d.stock[1]]=FIELD_CARD_CURE;
        d.kind[d.stock[2]]=FIELD_CARD_GUARD;
        assert(FieldDeckSleightPreview(&d,&kind,&value)&&kind==FIELD_CARD_FIRE);
        d.kind[d.stock[2]]=FIELD_CARD_CURE;
        assert(FieldDeckSleightPreview(&d,&kind,&value)&&kind==FIELD_CARD_CURE);
        d.kind[d.stock[0]]=d.kind[d.stock[1]]=d.kind[d.stock[2]]=FIELD_CARD_FIRE;
        assert(FieldDeckRecipe(&d)==2);
        d.kind[d.stock[0]]=d.kind[d.stock[1]]=d.kind[d.stock[2]]=FIELD_CARD_CURE;
        assert(FieldDeckRecipe(&d)==3);
        d=before;
        assert(FieldDeckSleight(&d,&kind,&value)&&kind==0&&value==18);
        assert(d.pile[0]==3&&d.pile[1]==2&&d.pile[2]==2);
        assert(FieldDeckReload(&d)&&d.pile[0]==3);
        assert(FieldDeckStock(&d));FieldDeckCancelStock(&d);
        assert(FieldDeckValid(&d)&&d.stocked==0);
        assert(!FieldDeckSleightPreview(&d,&kind,&value));
        assert(!FieldDeckSleight(&d,&kind,&value));
    }
    d.kind[0] = 4;assert(!FieldDeckValid(&d));
    puts("field deck: depletion, reload, values, selection, rewards passed");
    return 0;
}
