#include "field_route.h"
static int Abs(int value) {return value < 0 ? -value : value;}
int FieldRouteStep(FieldRoute* route, int targetX, int targetY,
    FieldRouteEdge edge, void* context) {
    int head, tail, node, next, direction, x, y, score, best, bestScore;
    static const int offsets[4] = {-1, 1, -9, 9};
    if (!route || !edge) return -1;
    for (node = 0; node < FIELD_ROUTE_CELLS; node++) route->parent[node] = 255;
    head = 0; tail = 1; best = 40;
    bestScore = Abs(targetX) + Abs(targetY);
    route->queue[0] = 40; route->parent[40] = 40;
    while (head < tail) {
        node = route->queue[head++];
        x = node % 9; y = node / 9;
        score = Abs(x - 4 - targetX) + Abs(y - 4 - targetY);
        if (score < bestScore) {best = node; bestScore = score;}
        if (!bestScore) break;
        for (direction = 0; direction < 4; direction++) {
            if ((direction == 0 && x == 0) || (direction == 1 && x == 8)) continue;
            next = node + offsets[direction];
            if (next < 0 || next >= FIELD_ROUTE_CELLS || route->parent[next] != 255) continue;
            if (!edge(node, next, context)) continue;
            route->parent[next] = node;
            route->queue[tail++] = next;
        }
    }
    if (best == 40) return -1;
    while (route->parent[best] != 40) best = route->parent[best];
    return best;
}
