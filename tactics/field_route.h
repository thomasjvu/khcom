#ifndef FIELD_ROUTE_H
#define FIELD_ROUTE_H
#define FIELD_ROUTE_SIDE 9
#define FIELD_ROUTE_CELLS 81
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
#endif
