#ifndef FIELD_ROSTER_H
#define FIELD_ROSTER_H
#define FIELD_HEROES 5
#define FIELD_SORA 0
#define FIELD_DONALD 1
#define FIELD_GOOFY 2
#define FIELD_CLOUD 3
#define FIELD_RALLY 4
#define FIELD_ASSEMBLY 0
#define FIELD_BATTLE 1
#define FIELD_REWARD 2
#define FIELD_REWARD_NONE 0
#define FIELD_REWARD_UPGRADE 1
#define FIELD_REWARD_BOSS 2
/* Identities belong to the roster; deployed[] maps tactical slots to heroes. */
typedef struct FieldRoster {
    unsigned char unlocked, deployed[3], power[FIELD_HEROES];
    unsigned char sleights[FIELD_HEROES];
    unsigned short cleared;
    unsigned char phase, reward, room, heroHp[FIELD_HEROES], heroMove[FIELD_HEROES], heroAction[FIELD_HEROES];
    unsigned char heroAngle[FIELD_HEROES];
} FieldRoster;
int FieldHeroMaxHp(int hero);
void FieldRosterInit(FieldRoster* roster);
int FieldRosterValid(const FieldRoster* roster);
int FieldRosterDeploy(FieldRoster* roster, int slot, int hero);
int FieldRosterBegin(FieldRoster* roster, int room);
int FieldRosterClear(FieldRoster* roster, int boss);
int FieldRosterUpgrade(FieldRoster* roster, int hero, int sleight);
int FieldRosterRecruit(FieldRoster* roster, int hero);
int FieldRosterRest(FieldRoster* roster);
#endif
