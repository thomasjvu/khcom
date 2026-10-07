#ifndef FIELD_ROUTE_H
#define FIELD_ROUTE_H
#define FIELD_ROUTE_SIDE 9
#define FIELD_ROUTE_CELLS 81
#define FIELD_TACTICS_NODES (FIELD_ROUTE_CELLS * 2)
#define FIELD_TACTICS_STATES (FIELD_TACTICS_NODES * 2)
#define FIELD_EDGE_WALK 0
#define FIELD_EDGE_CLIMB 1
#define FIELD_EDGE_JUMP 2
/* Geometry supplies directed links between standing surfaces. A destination
 * keeps separate states with its action available or spent: a cheap jump
 * must not erase a walking route needed before a later jump. */
typedef int (*FieldTacticsLinks)(int node, int index, int* destination,
    int* movement, int* action, int* kind, void* context);
typedef struct FieldTacticsRoute {
    unsigned char move[FIELD_TACTICS_STATES], visited[FIELD_TACTICS_STATES];
    unsigned short parent[FIELD_TACTICS_STATES];
    unsigned char kind[FIELD_TACTICS_STATES];
    unsigned short nodes, origin;
} FieldTacticsRoute;
/* Returns reachable resource states; -1 invalidates the route. */
int FieldTacticsSearch(FieldTacticsRoute* route,int nodes,int origin,int movement,
    int action,FieldTacticsLinks links,void* context);
int FieldTacticsPath(const FieldTacticsRoute* route,int target,
    unsigned short* nodes,unsigned char* kinds,int capacity,int* movement,int* action);
/* A bounded local search. The caller validates native geometry per edge. */
typedef int (*FieldRouteEdge)(int from, int to, void* context);
typedef struct FieldRoute {
    unsigned char queue[FIELD_ROUTE_CELLS];
    unsigned char parent[FIELD_ROUTE_CELLS];
} FieldRoute;
int FieldRouteStep(FieldRoute* route, int targetX, int targetY,
    FieldRouteEdge edge, void* context);
/* Exact destination; -1 means unreachable, 0 is the origin. */
int FieldRoutePath(FieldRoute* route, int targetX, int targetY,
    FieldRouteEdge edge, void* context, unsigned char* path);
/* Reachable destinations within budget; 255 means unavailable. Origin is 0. */
int FieldRouteReach(FieldRoute* route, int budget, FieldRouteEdge edge,
    void* context, unsigned char* cost);
/* Segment against an open occupancy box. Coordinates are relative to the
 * box center; edge deltas and half extents must fit signed 15-bit values. */
int FieldRouteSegmentBox(const int from[3], const int to[3], const int half[3]);
#endif
