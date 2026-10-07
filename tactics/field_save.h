#ifndef FIELD_SAVE_H
#define FIELD_SAVE_H
#include "field_deck.h"
#define FIELD_SAVE_SIZE 512
#define FIELD_SAVE_ENEMIES 6
typedef struct FieldSaveState {
    unsigned int seed;
    unsigned int roomFlags[12];
    unsigned char roomEnemies[12];
    FieldDeck deck;
    int partyPos[3][4];
    int enemyPos[FIELD_SAVE_ENEMIES][4];
    unsigned char enemyHp[FIELD_SAVE_ENEMIES];
    unsigned char enemyKind[FIELD_SAVE_ENEMIES];
    unsigned char move[3], action[3];
    unsigned char floor, room, party, hp, guard;
    unsigned short turn, kills, chests;
} FieldSaveState;
int FieldSaveEncode(const FieldSaveState* state, unsigned int generation, unsigned char* out);
int FieldSaveDecode(FieldSaveState* state, unsigned int* generation, const unsigned char* data);
int FieldSaveSelect(FieldSaveState* state, unsigned int* generation, const unsigned char* a, const unsigned char* b);
#endif
