#include <assert.h>
#include <stdio.h>
#include "field_route.h"
typedef struct Board {int height[81]; int blocked[81];} Board;
static int SegmentOracle(const int a[3],const int b[3],const int half[3]) {
    double enter=0,leave=1,low,high,swap,delta;int i;
    for(i=0;i<3;i++) {
        delta=b[i]-a[i];
        if(!delta){if(a[i]<=-half[i]||a[i]>=half[i])return 0;continue;}
        low=(-half[i]-a[i])/delta;high=(half[i]-a[i])/delta;
        if(low>high){swap=low;low=high;high=swap;}
        if(low>enter)enter=low;if(high<leave)leave=high;
        if(enter>=leave)return 0;
    }
    return 1;
}
static int Edge(int from, int to, void* context) {
    Board* board = context;
    int delta = board->height[from] - board->height[to];
    return !board->blocked[to] && delta <= 1 && delta >= -1;
}
int main(void) {
    FieldRoute route;
    Board board = {{0}, {0}};
    int i, step;
    unsigned char path[81],cost[81];
    {
        int a[3]={-4096,0,0},b[3]={4096,0,0},half[3]={512,512,512};
        unsigned int random=12345;int trial,axis;
        assert(FieldRouteSegmentBox(a,b,half));
        assert(FieldRouteSegmentBox(b,a,half));
        a[1]=b[1]=512;assert(!FieldRouteSegmentBox(a,b,half));
        a[1]=b[1]=511;assert(FieldRouteSegmentBox(a,b,half));
        a[1]=-4096;b[1]=4096;a[2]=4096;b[2]=-4096;
        assert(FieldRouteSegmentBox(a,b,half));
        a[2]=b[2]=1024;assert(!FieldRouteSegmentBox(a,b,half));
        a[0]=a[1]=a[2]=b[0]=b[1]=b[2]=0;
        assert(FieldRouteSegmentBox(a,b,half));
        b[0]=4096;assert(FieldRouteSegmentBox(a,b,half));
        for(trial=0;trial<5000;trial++) {
            for(axis=0;axis<3;axis++) {
                random=random*1664525u+1013904223u;a[axis]=(int)(random%16385)-8192;
                random=random*1664525u+1013904223u;b[axis]=(int)(random%16385)-8192;
                random=random*1664525u+1013904223u;half[axis]=(int)(random%4096)+1;
            }
            assert(FieldRouteSegmentBox(a,b,half)==SegmentOracle(a,b,half));
            assert(FieldRouteSegmentBox(b,a,half)==SegmentOracle(a,b,half));
        }
    }
    assert(FieldRouteReach(&route,0,Edge,&board,cost)==0&&cost[40]==0&&cost[41]==255);
    assert(FieldRouteReach(&route,3,Edge,&board,cost)==48);
    assert(cost[70]==3&&cost[43]==3&&cost[44]==255);
    for(i=0;i<81;i++)if(cost[i]!=255) {
        int length=FieldRoutePath(&route,i%9-4,i/9-4,Edge,&board,path);
        assert(length==cost[i]);
    }
    assert(FieldRouteStep(&route, 3, 0, Edge, &board) == 41);
    assert(FieldRoutePath(&route, 3, 0, Edge, &board, path) == 3);
    assert(path[0] == 41 && path[2] == 43);
    assert(FieldRoutePath(&route, 0, 0, Edge, &board, path) == 0);
    assert(FieldRoutePath(&route, 5, 0, Edge, &board, path) == -1);
    /* A wall requires the first step to go around, rather than get stuck. */
    board.blocked[41] = 1;
    step = FieldRouteStep(&route, 3, 0, Edge, &board);
    assert(step == 32 || step == 50);
    assert(FieldRoutePath(&route, 3, 0, Edge, &board, path) == 3);
    assert(path[2] == 43);
    /* An impassable ledge spanning the board cannot be crossed. */
    for (i = 0; i < 81; i++) {board.blocked[i] = 0; if (i % 9 > 4) board.height[i] = 3;}
    assert(FieldRouteStep(&route, 3, 0, Edge, &board) == -1);
    assert(FieldRoutePath(&route, 3, 0, Edge, &board, path) == -1);
    assert(FieldRouteReach(&route,3,Edge,&board,cost)>0&&cost[41]==255);
    /* A low stair is traversable. */
    for (i = 0; i < 81; i++) if (i % 9 > 4) board.height[i] = 1;
    assert(FieldRouteStep(&route, 3, 0, Edge, &board) == 41);
    /* Projected diagonals cost one segment and retain their exact destination. */
    assert(FieldRoutePath(&route, 3, 3, Edge, &board, path) == 3);
    assert(path[0] == 50 && path[2] == 70);
    /* Search must not wrap diagonally across a row boundary. */
    assert(FieldRoutePath(&route, -4, -4, Edge, &board, path) == 4);
    assert(path[0] == 30 && path[3] == 0);
    /* No reachable improvement: stay in place, never return an invalid node. */
    board.blocked[39] = board.blocked[41] = board.blocked[31] = board.blocked[49] = 1;
    board.blocked[30] = board.blocked[32] = board.blocked[48] = board.blocked[50] = 1;
    assert(FieldRouteStep(&route, 1000, -1000, Edge, &board) == -1);
    assert(FieldRouteStep(&route, 0, 0, Edge, &board) == -1);
    assert(FieldRouteReach(&route,3,Edge,&board,cost)==0);
    puts("field route: detours, height limits, stairs and enclosed actors passed");
    return 0;
}
