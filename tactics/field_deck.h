#ifndef FIELD_DECK_H
#define FIELD_DECK_H
#define FIELD_DECK_MAX 24
#define FIELD_HAND_MAX 5
#define FIELD_CARD_KEY 0
#define FIELD_CARD_FIRE 1
#define FIELD_CARD_CURE 2
#define FIELD_CARD_GUARD 3
/* Explicit piles: draw, hand, discard. A reload costs the active action. */
typedef struct FieldDeck {
    unsigned char kind[FIELD_DECK_MAX];
    unsigned char value[FIELD_DECK_MAX];
    unsigned char pile[FIELD_DECK_MAX];
    unsigned char count;
    unsigned char selected;
    unsigned char stock[3], stocked;
} FieldDeck;
void FieldDeckInit(FieldDeck* deck);
int FieldDeckHand(const FieldDeck* deck, int slot);
int FieldDeckStock(FieldDeck* deck);
/* Three matching types unlock enhanced field recipes; zero means mixed. */
int FieldDeckRecipe(const FieldDeck* deck);
int FieldDeckSleightPreview(const FieldDeck* deck, int* kind, int* value);
int FieldDeckSleight(FieldDeck* deck, int* kind, int* value);
void FieldDeckCancelStock(FieldDeck* deck);
int FieldDeckPlay(FieldDeck* deck);
void FieldDeckCycle(FieldDeck* deck, int direction);
int FieldDeckReload(FieldDeck* deck);
int FieldDeckReward(FieldDeck* deck, unsigned int seed);
int FieldDeckValid(const FieldDeck* deck);
#endif
