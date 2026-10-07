#ifndef FIELD_ENEMY_H
#define FIELD_ENEMY_H
int FieldEnemyKind(unsigned int seed, int floor, int room, int slot);
int FieldEnemyHp(int kind, int floor);
int FieldEnemyRange(int kind);
int FieldEnemyHeight(int kind);
int FieldEnemyDamage(int kind, int floor);
#endif
