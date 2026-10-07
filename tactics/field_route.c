#include "field_route.h"
static int Abs(int value) {return value < 0 ? -value : value;}
int FieldRouteStep(FieldRoute* route, int targetX, int targetY,
    FieldRouteEdge edge, void* context) {
    int head, tail, node, next, direction, x, y, score, best, bestScore;
    static const int stepX[8] = {-1, 1, 0, 0, -1, 1, -1, 1};
    static const int stepY[8] = {0, 0, -1, 1, -1, -1, 1, 1};
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
        for (direction = 0; direction < 8; direction++) {
            if (x + stepX[direction] < 0 || x + stepX[direction] >= 9 ||
                y + stepY[direction] < 0 || y + stepY[direction] >= 9) continue;
            next = node + stepX[direction] + stepY[direction] * 9;
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

int FieldRoutePath(FieldRoute* route, int targetX, int targetY,
    FieldRouteEdge edge, void* context, unsigned char* path) {
    int node, count, i;
    unsigned char swap;
    if (!route || !edge || !path || targetX < -4 || targetX > 4 ||
        targetY < -4 || targetY > 4) return -1;
    FieldRouteStep(route, targetX, targetY, edge, context);
    node = (targetY + 4) * 9 + targetX + 4;
    if (route->parent[node] == 255) return -1;
    count = 0;
    while (node != 40) {
        path[count++] = node;
        node = route->parent[node];
    }
    for (i = 0; i < count / 2; i++) {
        swap = path[i]; path[i] = path[count - i - 1]; path[count - i - 1] = swap;
    }
    return count;
}

int FieldRouteReach(FieldRoute* route, int budget, FieldRouteEdge edge,
    void* context, unsigned char* cost) {
    int head=0,tail=1,node,next,direction,x,y,i;
    static const int stepX[8]={-1,1,0,0,-1,1,-1,1};
    static const int stepY[8]={0,0,-1,1,-1,-1,1,1};
    if(!route||!edge||!cost||budget<0||budget>=FIELD_ROUTE_CELLS)return -1;
    for(i=0;i<FIELD_ROUTE_CELLS;i++)cost[i]=255;
    route->queue[0]=40;cost[40]=0;
    while(head<tail) {
        node=route->queue[head++];
        if(cost[node]>=budget)continue;
        x=node%9;y=node/9;
        for(direction=0;direction<8;direction++) {
            if(x+stepX[direction]<0||x+stepX[direction]>=9||
               y+stepY[direction]<0||y+stepY[direction]>=9)continue;
            next=node+stepX[direction]+stepY[direction]*9;
            if(cost[next]!=255||!edge(node,next,context))continue;
            cost[next]=cost[node]+1;route->queue[tail++]=next;
        }
    }
    return tail-1;
}
