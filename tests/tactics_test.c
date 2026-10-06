#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "tactics.h"

int main(void) {
    TacticsState s, before, replay;
    int y;
    TacticsInit(&s);
    before = s;
    assert(!TacticsMove(&s, -1, 2));
    assert(!TacticsMove(&s, 1, 2));
    assert(!TacticsMove(&s, 5, 1));
    assert(!TacticsAttack(&s, 1));
    assert(memcmp(&s, &before, sizeof(s)) == 0);
    for (y = 0; y < TACTICS_HEIGHT; y++) s.blocked[y * TACTICS_WIDTH + 2] = 1;
    before = s;
    assert(!TacticsMove(&s, 3, 2));
    assert(memcmp(&s, &before, sizeof(s)) == 0);
    TacticsInit(&s);
    s.blocked[2 * TACTICS_WIDTH + 2] = 1;
    assert(!TacticsMove(&s, 3, 2)); /* detour is four steps */
    s.blocked[2 * TACTICS_WIDTH + 2] = 0;
    assert(TacticsMove(&s, 4, 2));
    assert(!TacticsMove(&s, 4, 1));
    assert(TacticsEndTurn(&s));
    assert(TacticsMove(&s, 4, 1));
    assert(TacticsAttack(&s, 1));
    assert(!TacticsAttack(&s, 1));
    assert(TacticsEndTurn(&s));
    assert(s.units[0].hp == 9);
    assert(TacticsAttack(&s, 1));
    assert(s.units[1].hp == 0);
    assert(TacticsMove(&s, 5, 1)); /* dead unit no longer blocks */
    TacticsInit(&s);
    s.units[1].x = 2; s.units[1].y = 2;
    s.units[1].hp = 1; s.units[2].hp = 0;
    assert(TacticsAttack(&s, 1));
    assert(s.phase == TACTICS_WON);
    assert(!TacticsEndTurn(&s));
    TacticsInit(&s);
    s.units[0].hp = 1; s.units[1].x = 2; s.units[1].y = 2;
    assert(TacticsEndTurn(&s));
    assert(s.phase == TACTICS_LOST);
    assert(!TacticsAttack(&s, 1));
    TacticsInit(&s); replay = s;
    assert(TacticsMove(&s, 4, 2) == TacticsMove(&replay, 4, 2));
    assert(TacticsEndTurn(&s) == TacticsEndTurn(&replay));
    assert(memcmp(&s, &replay, sizeof(s)) == 0);
    puts("tactics rules: all checks passed");
    return 0;
}
