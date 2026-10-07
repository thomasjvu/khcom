#include "field_enemy.h"
int FieldEnemyKind(unsigned int seed, int floor, int room, int slot) {
    static const unsigned char groups[3][4] = {{0,0,1,3},{1,3,0,1},{0,3,6,1}};
    if (room == 7 && slot == 0) return 2;
    if (floor < 0 || floor > 2 || slot < 0) return 0;
    /* The entry room remains a predictable introduction to field combat. */
    if (room == 0 && floor == 0) return slot & 1;
    return groups[floor][((seed >> 8) + slot) & 3];
}
int FieldEnemyHp(int kind, int floor) {
    if (kind == 2) return 40 + floor * 8;
    if (kind == 3) return 10 + floor * 2;
    if (kind == 6) return 12 + floor * 2;
    return 6 + floor * 2;
}
int FieldEnemyRange(int kind) {
    if (kind == 1) return 96;
    if (kind == 2) return 64;
    if (kind == 3) return 56;
    return 40;
}
int FieldEnemyHeight(int kind) {return kind == 1 ? 32 : kind == 2 || kind == 3 ? 24 : 16;}
int FieldEnemyDamage(int kind, int floor) {
    if (kind == 2) return 10 + floor * 2;
    if (kind == 1) return 3 + floor;
    if (kind == 3) return 5 + floor;
    return 4 + floor;
}
