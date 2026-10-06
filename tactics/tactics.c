#include "tactics.h"

static int Abs(int n) { return n < 0 ? -n : n; }

static int Occupied(const TacticsState *s, int x, int y, int except) {
    int i;
    for (i = 0; i < TACTICS_UNITS; i++)
        if (i != except && s->units[i].hp &&
            s->units[i].x == x && s->units[i].y == y) return 1;
    return 0;
}

/* Bounded BFS finds actual route length rather than Manhattan distance. */
static int Distance(const TacticsState *s, int x, int y) {
    unsigned char queue[TACTICS_CELLS], dist[TACTICS_CELLS];
    int i, head, tail, cell, next, nx, ny, d;
    static const signed char dx[4] = {1, -1, 0, 0};
    static const signed char dy[4] = {0, 0, 1, -1};
    for (i = 0; i < TACTICS_CELLS; i++) dist[i] = 255;
    cell = s->units[0].y * TACTICS_WIDTH + s->units[0].x;
    head = 0; tail = 1; queue[0] = cell; dist[cell] = 0;
    while (head < tail) {
        cell = queue[head++];
        if (cell == y * TACTICS_WIDTH + x) return dist[cell];
        for (d = 0; d < 4; d++) {
            nx = cell % TACTICS_WIDTH + dx[d];
            ny = cell / TACTICS_WIDTH + dy[d];
            if (nx < 0 || nx >= TACTICS_WIDTH || ny < 0 || ny >= TACTICS_HEIGHT) continue;
            next = ny * TACTICS_WIDTH + nx;
            if (dist[next] != 255 || s->blocked[next] || Occupied(s, nx, ny, 0)) continue;
            dist[next] = dist[cell] + 1;
            queue[tail++] = next;
        }
    }
    return -1;
}

void TacticsInit(TacticsState *s) {
    int i;
    for (i = 0; i < TACTICS_CELLS; i++) s->blocked[i] = 0;
    s->units[0].x = 1; s->units[0].y = 2; s->units[0].hp = 10;
    s->units[1].x = 5; s->units[1].y = 1; s->units[1].hp = 4;
    s->units[2].x = 5; s->units[2].y = 4; s->units[2].hp = 4;
    s->phase = TACTICS_PLAYER; s->moved = 0; s->acted = 0;
}

int TacticsMove(TacticsState *s, int x, int y) {
    int distance;
    if (s->phase != TACTICS_PLAYER || s->moved || x < 0 || x >= TACTICS_WIDTH ||
        y < 0 || y >= TACTICS_HEIGHT) return 0;
    distance = Distance(s, x, y);
    if (distance < 1 || distance > 3) return 0;
    s->units[0].x = x; s->units[0].y = y; s->moved = 1;
    return 1;
}

int TacticsAttack(TacticsState *s, int target) {
    TacticsUnit *enemy;
    int i;
    if (s->phase != TACTICS_PLAYER || s->acted || target < 1 || target >= TACTICS_UNITS) return 0;
    enemy = &s->units[target];
    if (!enemy->hp || Abs(enemy->x - s->units[0].x) + Abs(enemy->y - s->units[0].y) != 1) return 0;
    enemy->hp = enemy->hp > 2 ? enemy->hp - 2 : 0;
    s->acted = 1;
    for (i = 1; i < TACTICS_UNITS; i++) if (s->units[i].hp) return 1;
    s->phase = TACTICS_WON;
    return 1;
}

/* Initial enemy prototype: adjacent attacks only; intent/path AI comes next. */
int TacticsEndTurn(TacticsState *s) {
    int i;
    if (s->phase != TACTICS_PLAYER) return 0;
    s->phase = TACTICS_ENEMY;
    for (i = 1; i < TACTICS_UNITS; i++) {
        if (s->units[i].hp && Abs(s->units[i].x - s->units[0].x) +
            Abs(s->units[i].y - s->units[0].y) == 1) {
            s->units[0].hp = s->units[0].hp > 1 ? s->units[0].hp - 1 : 0;
            if (!s->units[0].hp) { s->phase = TACTICS_LOST; return 1; }
        }
    }
    s->phase = TACTICS_PLAYER; s->moved = 0; s->acted = 0;
    return 1;
}
