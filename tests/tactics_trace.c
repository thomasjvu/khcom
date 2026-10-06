/* Reuse the test policy to emit an input-only emulator replay. */
#define main TacticsRulesMain
#include "tactics_test.c"
#undef main
int main(void) {
    TacticsState s;int step,choice,i,position;
    TacticsNewRun(&s,1);
    puts("new 1");
    for(step=0;step<1000&&s.phase!=TACTICS_CLEAR&&s.phase!=TACTICS_LOST;step++) {
        if(s.phase==TACTICS_MAP) {
            choice=s.routes[s.stage][0]==ROOM_REST?0:s.routes[s.stage][1]==ROOM_REST?1:0;
            printf("room %d\n",choice);TacticsChooseRoom(&s,choice);
        } else if(s.phase==TACTICS_REWARD) {
            choice=s.hp<s.maxHp-4?3:0;printf("reward %d\n",choice);TacticsReward(&s,choice);
        } else {
            TacticsState before=s;
            BotTurn(&s);position=-1;
            for(i=0;i<TacticsHandCount(&before);i++)if(TacticsHandCard(&before,i)==sBotCard)position=i;
            printf("turn %d %d %d %d %d %d\n",sBotX,sBotY,position,sBotSleight,sBotTargetX,sBotTargetY);
        }
        assert(TacticsValid(&s));
        /* Field-by-field state bytes are identical on host and GBA; trailing padding is zero. */
        printf("state ");for(i=0;i<(int)sizeof(s);i++)printf("%02x",((TacByte *)&s)[i]);puts("");
    }
    assert(s.phase==TACTICS_CLEAR);puts("clear");return 0;
}
