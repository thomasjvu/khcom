#include <assert.h>
#include <stdio.h>
#include "field_route.h"
typedef struct Board {int height[81]; int blocked[81];} Board;
static int Edge(int from, int to, void* context) {
    Board* board = context;
    int delta = board->height[from] - board->height[to];
    return !board->blocked[to] && delta <= 1 && delta >= -1;
}
int main(void) {
    FieldRoute route;
    Board board = {{0}, {0}};
    int i, step;
    unsigned char path[81];
    assert(FieldRouteStep(&route, 3, 0, Edge, &board) == 41);
    assert(FieldRoutePath(&route, 3, 0, Edge, &board, path) == 3);
    assert(path[0] == 41 && path[2] == 43);
    assert(FieldRoutePath(&route, 0, 0, Edge, &board, path) == 0);
    assert(FieldRoutePath(&route, 5, 0, Edge, &board, path) == -1);
    /* A wall requires the first step to go around, rather than get stuck. */
    board.blocked[41] = 1;
    step = FieldRouteStep(&route, 3, 0, Edge, &board);
    assert(step == 31 || step == 49);
    assert(FieldRoutePath(&route, 3, 0, Edge, &board, path) == 5);
    assert(path[4] == 43);
    /* An impassable ledge spanning the board cannot be crossed. */
    for (i = 0; i < 81; i++) {board.blocked[i] = 0; if (i % 9 > 4) board.height[i] = 3;}
    assert(FieldRouteStep(&route, 3, 0, Edge, &board) == -1);
    assert(FieldRoutePath(&route, 3, 0, Edge, &board, path) == -1);
    /* A low stair is traversable. */
    for (i = 0; i < 81; i++) if (i % 9 > 4) board.height[i] = 1;
    assert(FieldRouteStep(&route, 3, 0, Edge, &board) == 41);
    /* No reachable improvement: stay in place, never return an invalid node. */
    board.blocked[39] = board.blocked[41] = board.blocked[31] = board.blocked[49] = 1;
    assert(FieldRouteStep(&route, 1000, -1000, Edge, &board) == -1);
    assert(FieldRouteStep(&route, 0, 0, Edge, &board) == -1);
    puts("field route: detours, height limits, stairs and enclosed actors passed");
    return 0;
}
