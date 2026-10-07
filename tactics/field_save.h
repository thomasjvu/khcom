#ifndef FIELD_SAVE_H
#define FIELD_SAVE_H
#include "field_deck.h"
#define FIELD_SAVE_SIZE 1024
#define FIELD_SAVE_ENEMIES 6
/* Field enemies move on whole pixels. Signed pixel coordinates preserve the
 * native 8.8 values exactly while bounding all twelve room snapshots. */
typedef struct FieldEncounter {
    short pos[4];
    unsigned char hp, kind;
} FieldEncounter;
typedef struct FieldSaveState {
    unsigned int seed;
    unsigned int roomFlags[12];
    unsigned char roomEnemies[12];
    FieldDeck deck;
    int partyPos[3][4];
    FieldEncounter encounters[12][FIELD_SAVE_ENEMIES];
    unsigned char roomCached[12];
    unsigned char move[3], action[3], partyHp[3];
    unsigned char floor, room, party, hp, guard;
    unsigned short turn, kills, chests;
} FieldSaveState;
int FieldSaveEncode(const FieldSaveState* state, unsigned int generation, unsigned char* out);
int FieldSaveDecode(FieldSaveState* state, unsigned int* generation, const unsigned char* data);
int FieldSaveSelect(FieldSaveState* state, unsigned int* generation, const unsigned char* a, const unsigned char* b);
#endif
