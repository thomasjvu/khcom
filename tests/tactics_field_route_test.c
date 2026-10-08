#include <assert.h>
#include <stdio.h>
#include "field_route.h"
typedef struct Board {int height[81]; int blocked[81];} Board;
typedef struct ResourceGraph {int move[12][12],act[12][12];} ResourceGraph;
static int ResourceLinks(int node,int index,int* to,int* movement,int* action,int* kind,void* context) {
    ResourceGraph* graph=context;int i,n=0;
    for(i=0;i<12;i++)if(graph->move[node][i]>=0&&n++==index) {
        *to=i;*movement=graph->move[node][i];*action=graph->act[node][i];
        *kind=*action?FIELD_EDGE_JUMP:FIELD_EDGE_WALK;return 1;
    }
    return 0;
}
/* Independent repeated relaxation checks every resource state, including
 * directed cycles, action-only links, unreachable nodes and budget limits. */
static void ResourceOracle(void) {
    ResourceGraph graph;FieldTacticsRoute route;unsigned int random=98765;
    int trial,i,j,pass,spent,next,value,budget,actions,expected[24];
    for(trial=0;trial<1000;trial++) {
        budget=trial%9;actions=trial%2;
        for(i=0;i<12;i++)for(j=0;j<12;j++) {
            random=random*1664525u+1013904223u;
            graph.move[i][j]=random%4==0?(int)((random>>8)%4):-1;
            graph.act[i][j]=(random>>16)%2;
            if(graph.move[i][j]==0&&!graph.act[i][j])graph.move[i][j]=1;
        }
        for(i=0;i<24;i++)expected[i]=255;
        expected[0]=0;
        for(pass=0;pass<24;pass++)for(spent=0;spent<=actions;spent++)
            for(i=0;i<12;i++)for(j=0;j<12;j++) {
                if(graph.move[i][j]<0||expected[i+spent*12]==255||spent+graph.act[i][j]>actions)continue;
                value=expected[i+spent*12]+graph.move[i][j];
                next=j+(spent+graph.act[i][j])*12;
                if(value<=budget&&value<expected[next])expected[next]=value;
            }
        assert(FieldTacticsSearch(&route,12,0,budget,actions,ResourceLinks,&graph)>0);
        for(i=0;i<24;i++)assert(route.move[i]==expected[i]);
    }
}
static int HeightLinks(int node,int index,int* to,int* movement,int* action,int* kind,void* context) {
    static const int edges[][5]={{0,1,1,0,FIELD_EDGE_WALK},{1,3,1,0,FIELD_EDGE_CLIMB},
        {0,3,0,1,FIELD_EDGE_JUMP},{3,4,1,1,FIELD_EDGE_JUMP},{4,5,1,0,FIELD_EDGE_WALK}};
    int i,n=0;(void)context;
    for(i=0;i<5;i++)if(edges[i][0]==node&&n++==index) {
        *to=edges[i][1];*movement=edges[i][2];*action=edges[i][3];*kind=edges[i][4];return 1;
    }
    return 0;
}
static int SegmentOracle(const int a[3],const int b[3],const int half[3]) {
    double enter=0,leave=1,low,high,swap,delta;int i;
    for(i=0;i<3;i++) {
        delta=b[i]-a[i];
        if(!delta){if(a[i]<=-half[i]||a[i]>=half[i])return 0;continue;}
        low=(-half[i]-a[i])/delta;high=(half[i]-a[i])/delta;
        if(low>high){swap=low;low=high;high=swap;}
        if(low>enter)enter=low;
        if(high<leave)leave=high;
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
    ResourceOracle();
    {
        FieldTacticsRoute tactical;unsigned short nodes[6];unsigned char kinds[6];int move,act;
        assert(FieldTacticsSearch(&tactical,6,0,3,1,HeightLinks,0)>0);
        assert(FieldTacticsPath(&tactical,3,nodes,kinds,6,&move,&act)==1&&move==0&&act==1&&kinds[0]==FIELD_EDGE_JUMP);
        /* Keep the more expensive action-preserving route to the same ledge:
         * arriving by the cheap first jump would make the second jump illegal. */
        assert(FieldTacticsPath(&tactical,4,nodes,kinds,6,&move,&act)==3&&move==3&&act==1);
        assert(nodes[0]==1&&nodes[1]==3&&nodes[2]==4);
        assert(kinds[0]==FIELD_EDGE_WALK&&kinds[1]==FIELD_EDGE_CLIMB&&kinds[2]==FIELD_EDGE_JUMP);
        assert(FieldTacticsPath(&tactical,5,nodes,kinds,6,&move,&act)==-1);
        assert(FieldTacticsPath(&tactical,4,nodes,kinds,2,&move,&act)==-1);
        assert(FieldTacticsSearch(&tactical,6,0,3,0,HeightLinks,0)>0);
        assert(FieldTacticsPath(&tactical,3,nodes,kinds,6,&move,&act)==2&&move==2&&act==0);
        assert(FieldTacticsPath(&tactical,4,nodes,kinds,6,&move,&act)==-1);
        assert(FieldTacticsSearch(&tactical,6,0,0,1,HeightLinks,0)>0);
        assert(FieldTacticsPath(&tactical,3,nodes,kinds,6,&move,&act)==1&&move==0&&act==1);
        assert(FieldTacticsPath(&tactical,1,nodes,kinds,6,&move,&act)==-1);
        assert(FieldTacticsSearch(&tactical,6,0,3,2,HeightLinks,0)==-1);
        assert(FieldTacticsPath(&tactical,3,nodes,kinds,6,&move,&act)==-1);
    }
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
    puts("field route: detours, height, occupancy and 1000 resource-state graph oracles passed");
    return 0;
}
