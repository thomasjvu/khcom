#ifndef KH_TACTICS_H
#define KH_TACTICS_H

#define TACTICS_WIDTH 8
#define TACTICS_HEIGHT 6
#define TACTICS_CELLS (TACTICS_WIDTH * TACTICS_HEIGHT)
#define TACTICS_UNITS 3
#define TACTICS_PLAYER 0
#define TACTICS_ENEMY 1
#define TACTICS_WON 2
#define TACTICS_LOST 3

typedef struct TacticsUnit {
    unsigned char x, y, hp;
} TacticsUnit;

typedef struct TacticsState {
    unsigned char blocked[TACTICS_CELLS];
    TacticsUnit units[TACTICS_UNITS];
    unsigned char phase, moved, acted;
} TacticsState;

void TacticsInit(TacticsState *state);
int TacticsMove(TacticsState *state, int x, int y);
int TacticsAttack(TacticsState *state, int target);
int TacticsEndTurn(TacticsState *state);

#endif
