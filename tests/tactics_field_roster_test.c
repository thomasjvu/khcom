#include <assert.h>
#include <stdio.h>
#include "field_roster.h"
int main(void) {
    FieldRoster r;int i;
    FieldRosterInit(&r);assert(FieldRosterValid(&r));
    assert(!FieldRosterDeploy(&r,1,FIELD_CLOUD));
    assert(!FieldRosterDeploy(&r,0,FIELD_GOOFY));
    assert(FieldRosterDeploy(&r,1,FIELD_GOOFY));assert(r.deployed[2]==FIELD_DONALD);
    assert(FieldRosterBegin(&r,0));assert(!FieldRosterDeploy(&r,1,FIELD_SORA));
    assert(FieldRosterClear(&r,0)==FIELD_REWARD_NONE);
    assert(FieldRosterBegin(&r,1));assert(FieldRosterClear(&r,0)==FIELD_REWARD_UPGRADE);
    assert(!FieldRosterRecruit(&r,FIELD_CLOUD));assert(FieldRosterUpgrade(&r,FIELD_DONALD,1));
    assert(r.sleights[FIELD_DONALD]==1);
    assert(FieldRosterBegin(&r,1));assert(FieldRosterClear(&r,0)==FIELD_REWARD_NONE);
    assert(FieldRosterBegin(&r,7));assert(FieldRosterClear(&r,1)==FIELD_REWARD_BOSS);
    assert(FieldRosterRecruit(&r,FIELD_CLOUD));assert(FieldRosterDeploy(&r,1,FIELD_CLOUD));
    assert(r.deployed[1]==FIELD_CLOUD);assert(FieldRosterValid(&r));
    for(i=0;i<8;i++) {
        r.phase=FIELD_REWARD;r.reward=FIELD_REWARD_UPGRADE;
        assert(FieldRosterUpgrade(&r,FIELD_CLOUD,0));
    }
    r.phase=FIELD_REWARD;r.reward=FIELD_REWARD_UPGRADE;
    assert(!FieldRosterUpgrade(&r,FIELD_CLOUD,0));
    assert(FieldRosterUpgrade(&r,FIELD_CLOUD,2));
    r.phase=FIELD_REWARD;r.reward=FIELD_REWARD_UPGRADE;
    assert(!FieldRosterUpgrade(&r,FIELD_CLOUD,2));
    assert(!FieldRosterRest(&r));
    r.phase=FIELD_ASSEMBLY;r.reward=0;
    r.heroHp[FIELD_CLOUD]=0;r.heroHp[FIELD_DONALD]=1;
    r.heroMove[FIELD_CLOUD]=0;r.heroAction[FIELD_DONALD]=0;
    assert(FieldRosterRest(&r));
    assert(r.heroHp[FIELD_CLOUD]==72&&r.heroHp[FIELD_DONALD]==56);
    assert(r.heroMove[FIELD_CLOUD]==3&&r.heroAction[FIELD_DONALD]==1);
    assert(r.deployed[1]==FIELD_CLOUD&&r.power[FIELD_CLOUD]==8&&r.sleights[FIELD_CLOUD]==2);
    r.deployed[0]=FIELD_CLOUD;assert(!FieldRosterValid(&r));
    puts("roster: deployment phases, recruitment, alternate rewards, repeat clears and upgrade limits passed");return 0;
}
