#include "field_route.h"
static int Abs(int value) {return value < 0 ? -value : value;}
int FieldTacticsSearch(FieldTacticsRoute* r,int nodes,int origin,int movement,
    int action,FieldTacticsLinks links,void* context) {
    int i,best,count=0,index,to,move,act,kind,state,cost,result,level,node,spent;
    if(!r)return -1;
    r->nodes=0;
    if(!links||nodes<1||nodes>FIELD_TACTICS_NODES||origin<0||origin>=nodes||movement<0||movement>254||action<0||action>1)return -1;
    r->nodes=nodes;r->origin=origin;
    for(i=0;i<nodes*2;i++){r->move[i]=255;r->visited[i]=0;r->parent[i]=65535;r->kind[i]=0;}
    r->move[origin]=0;r->parent[origin]=origin;
    /* Movement costs are small bounded integers. Visit each cost level once
     * instead of finding the global minimum again for every destination.
     * A zero-movement edge must spend the sole action, so its destination
     * lies in the later action-spent half of this same level's scan. */
    for(level=0;level<=movement;level++)for(best=0;best<nodes*(action+1);best++) {
        if(r->move[best]!=level)continue;
        r->visited[best]=1;count++;
        spent=best>=nodes;node=best-spent*nodes;
        for(index=0;index<=FIELD_TACTICS_NODES;index++) {
            result=links(node,index,&to,&move,&act,&kind,context);
            if(!result)break;
            if(result!=1||to<0||to>=nodes||move<0||move>254||act<0||act>1||
                move+act==0||kind<0||kind>FIELD_EDGE_JUMP){r->nodes=0;return -1;}
            if(spent+act>action)continue;
            cost=r->move[best]+move;if(cost>movement)continue;
            state=to+(spent+act)*nodes;
            if(cost<r->move[state]) {
                r->move[state]=cost;r->parent[state]=best;r->kind[state]=kind;
            }
        }
        if(index>FIELD_TACTICS_NODES){r->nodes=0;return -1;}
    }
    return count;
}
int FieldTacticsPath(const FieldTacticsRoute* r,int target,unsigned short* nodes,
    unsigned char* kinds,int capacity,int* movement,int* action) {
    int state,count=0,current,i;
    if(!r||!nodes||!kinds||!movement||!action||r->nodes<1||r->nodes>FIELD_TACTICS_NODES||
        r->origin>=r->nodes||target<0||target>=r->nodes||capacity<0)return -1;
    state=target;
    if(r->move[target+r->nodes]<r->move[target])state+=r->nodes;
    if(r->move[state]==255)return -1;
    current=state;
    while(current!=r->origin) {
        if(current<0||current>=r->nodes*2||r->parent[current]==65535||count>=r->nodes*2)return -1;
        current=r->parent[current];count++;
    }
    if(count>capacity)return -1;
    *movement=r->move[state];*action=state/r->nodes;current=state;
    for(i=count-1;i>=0;i--){nodes[i]=current%r->nodes;kinds[i]=r->kind[current];current=r->parent[current];}
    return count;
}
int FieldRouteSegmentBox(const int from[3],const int to[3],const int half[3]) {
    int axis,delta,low,high,enter,leave,denom;
    int enterN=0,enterD=1,leaveN=1,leaveD=1;
    if(!from||!to||!half)return 0;
    for(axis=0;axis<3;axis++) {
        if(half[axis]<=0||half[axis]>32767)return 0;
        low=from[axis]<to[axis]?from[axis]:to[axis];
        high=from[axis]>to[axis]?from[axis]:to[axis];
        if(low>=half[axis]||high<=-half[axis])return 0;
        delta=to[axis]-from[axis];
        if(!delta)continue;
        denom=Abs(delta);if(denom>32767)return 0;
        enter=low < -half[axis] ? -half[axis]-low : 0;
        leave=high > half[axis] ? half[axis]-low : denom;
        if(delta<0) {
            enter=high > half[axis] ? high-half[axis] : 0;
            leave=low < -half[axis] ? high+half[axis] : denom;
        }
        if(enter*enterD>enterN*denom){enterN=enter;enterD=denom;}
        if(leave*leaveD<leaveN*denom){leaveN=leave;leaveD=denom;}
        if(enterN*leaveD>=leaveN*enterD)return 0;
    }
    return 1;
}
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
