#ifndef KH_TACTICS_H
#define KH_TACTICS_H
#define TACTICS_WIDTH 8
#define TACTICS_HEIGHT 6
#define TACTICS_CELLS 48
#define TACTICS_UNITS 5
#define TACTICS_DECK 24
#define TACTICS_HAND 5
#define TACTICS_STAGES 12
#define TACTICS_PLAYER 0
#define TACTICS_WON 2
#define TACTICS_LOST 3
#define TACTICS_MAP 4
#define TACTICS_REWARD 5
#define TACTICS_CLEAR 6
#define CARD_KEY 0
#define CARD_FIRE 1
#define CARD_CURE 2
#define CARD_GUARD 3
#define ROOM_BATTLE 0
#define ROOM_REST 1
#define ROOM_ELITE 2
#define ROOM_BOSS 3
#define ENEMY_SHADOW 0
#define ENEMY_MAGE 1
#define ENEMY_KNIGHT 2
#define ENEMY_BOSS 3

typedef unsigned char TacByte;
typedef unsigned int TacWord;
typedef struct TacticsCard { TacByte kind, value; } __attribute__((packed)) TacticsCard;
typedef struct TacticsUnit { TacByte x, y, hp, maxHp, kind, value; } __attribute__((packed)) TacticsUnit;
typedef struct TacticsIntent { TacByte x, y, targetX, targetY, damage, broken; } __attribute__((packed)) TacticsIntent;
typedef struct TacticsState {
    TacWord seed, rng;
    TacByte blocked[TACTICS_CELLS];
    TacticsUnit units[TACTICS_UNITS];
    TacticsIntent intents[TACTICS_UNITS];
    TacticsCard deck[TACTICS_DECK], rewards[3];
    TacByte piles[TACTICS_DECK]; /* 0 draw, 1 hand, 2 discard, 3 exhausted */
    TacByte routes[TACTICS_STAGES][2], terrain[TACTICS_STAGES][2];
    TacByte deckCount, stage, room, phase, moved, acted, shield, turn;
    TacByte maxHp, hp, power, victories;
} TacticsState;

typedef struct TacticsPreview { int legal, damage, heal, shield, breaks; } TacticsPreview;
void TacticsInit(TacticsState *s);
void TacticsNewRun(TacticsState *s, TacWord seed);
int TacticsChooseRoom(TacticsState *s, int choice);
int TacticsReward(TacticsState *s, int choice);
int TacticsMove(TacticsState *s, int x, int y);
void TacticsMovementMap(const TacticsState *s, TacByte *dist);
int TacticsThreatens(const TacticsState *s, int enemy, int x, int y);
int TacticsMoveDistance(const TacticsState *s, int x, int y);
int TacticsAttack(TacticsState *s, int target);
int TacticsPlay(TacticsState *s, int card, int x, int y, int sleight);
TacticsPreview TacticsPreviewCard(const TacticsState *s, int card, int x, int y, int sleight);
int TacticsReload(TacticsState *s);
int TacticsEndTurn(TacticsState *s);
int TacticsHandCard(const TacticsState *s, int position);
int TacticsHandCount(const TacticsState *s);
void TacticsPlanIntents(TacticsState *s);
int TacticsValid(const TacticsState *s);
#endif
