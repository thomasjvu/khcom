#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "tactics.h"
#include "save.h"

static void Fixture(TacticsState *s) {
    int i;TacticsInit(s);
    memset(s->blocked,0,sizeof(s->blocked));memset(s->units,0,sizeof(s->units));memset(s->piles,0,sizeof(s->piles));
    s->units[0].x=1;s->units[0].y=2;s->units[0].hp=s->hp=20;s->units[0].maxHp=s->maxHp;
    s->units[1].x=2;s->units[1].y=2;s->units[1].hp=12;s->units[1].maxHp=12;s->units[1].value=4;
    for(i=0;i<5;i++){s->piles[i]=1;s->deck[i].kind=i==1?CARD_FIRE:i==2?CARD_CURE:i==3?CARD_GUARD:CARD_KEY;s->deck[i].value=7;}
    TacticsPlanIntents(s);
}
static void TestCommands(void) {
    TacticsState s,before;TacticsPreview p;int y;
    Fixture(&s);before=s;
    assert(!TacticsMove(&s,-1,2));assert(!TacticsMove(&s,1,2));assert(!TacticsMove(&s,2,2));
    assert(!TacticsPlay(&s,20,2,2,0));assert(!TacticsPlay(&s,0,7,5,0));
    assert(memcmp(&s,&before,sizeof(s))==0);
    for(y=0;y<6;y++)s.blocked[y*8+3]=1;
    assert(!TacticsMove(&s,4,2));
    Fixture(&s);s.blocked[2*8+1]=0;s.blocked[1*8+1]=1;
    assert(!TacticsMove(&s,1,0)); /* detour exceeds budget */
    Fixture(&s);assert(TacticsMove(&s,1,3));assert(!TacticsMove(&s,1,4));
    assert(TacticsPlay(&s,1,2,2,0));assert(s.units[1].hp==6);assert(s.intents[1].broken);
    assert(!TacticsPlay(&s,2,1,3,0));assert(TacticsEndTurn(&s));assert(s.hp==20);
    Fixture(&s);p=TacticsPreviewCard(&s,0,2,2,0);assert(p.legal&&p.damage==5&&p.breaks==1);
    assert(TacticsPlay(&s,0,2,2,0));assert(s.units[1].hp==12-p.damage);
    Fixture(&s);s.deck[0].value=1;p=TacticsPreviewCard(&s,0,2,2,0);assert(p.legal&&!p.damage&&!p.breaks);
    assert(TacticsPlay(&s,0,2,2,0));assert(s.units[1].hp==12);assert(s.acted);
    Fixture(&s);s.deck[0].value=0;assert(TacticsPreviewCard(&s,0,2,2,0).breaks==1);
    Fixture(&s);assert(TacticsPlay(&s,2,1,2,0));assert(s.hp==24);
    Fixture(&s);assert(TacticsPlay(&s,3,1,2,0));assert(s.shield==6);assert(TacticsEndTurn(&s));assert(s.hp==20);
    Fixture(&s);assert(TacticsEndTurn(&s));assert(s.hp==18);
    Fixture(&s);assert(TacticsMove(&s,1,3));assert(TacticsEndTurn(&s));assert(s.hp==20); /* dodge locked target */
    Fixture(&s);assert(TacticsPlay(&s,0,2,2,1));assert(s.piles[0]==3&&s.piles[1]==2&&s.piles[2]==2);
    assert(s.phase==TACTICS_REWARD);assert(!TacticsEndTurn(&s));
    Fixture(&s);s.units[0].hp=s.hp=1;assert(TacticsEndTurn(&s));assert(s.phase==TACTICS_LOST);assert(!TacticsMove(&s,0,2));
    Fixture(&s);assert(!TacticsReload(&s));s.piles[5]=2;assert(TacticsReload(&s));assert(s.acted&&s.piles[5]==0);
    assert(!TacticsReload(&s));
    Fixture(&s);s.piles[0]=3;s.piles[1]=2;assert(TacticsReload(&s));assert(s.piles[0]==3);
}
static void TestSaves(void) {
    TacticsState s,loaded,before;TacByte a[512],b[512],bad[512];TacWord generation;
    Fixture(&s);assert(TacticsValid(&s));assert(TacticsSaveEncode(&s,7,a));
    assert(TacticsSaveDecode(&loaded,&generation,a));assert(generation==7);assert(memcmp(&s,&loaded,sizeof(s))==0);
    before=loaded;memcpy(bad,a,512);bad[100]^=1;assert(!TacticsSaveDecode(&loaded,&generation,bad));assert(memcmp(&before,&loaded,sizeof(s))==0);
    assert(TacticsMove(&s,1,3));assert(TacticsSaveEncode(&s,8,b));
    assert(TacticsSaveSelect(&loaded,&generation,a,b)==1);assert(generation==8&&loaded.moved);
    b[511]^=1;assert(TacticsSaveSelect(&loaded,&generation,a,b)==0);assert(generation==7&&!loaded.moved);
    a[0]=0;assert(TacticsSaveSelect(&loaded,&generation,a,b)==-1);
    assert(TacticsSaveEncode(&s,0xffffffffu,a));assert(TacticsSaveEncode(&s,0,b));
    assert(TacticsSaveSelect(&loaded,&generation,a,b)==1&&generation==0);
    s.units[1].x=99;assert(!TacticsSaveEncode(&s,1,a));
}
/* Greedy legal-action player: useful for exercising complete runs, not a balance oracle. */
static int sBotX,sBotY,sBotCard,sBotSleight,sBotTargetX,sBotTargetY;
static void BotTurn(TacticsState *s) {
    TacticsState candidate,best;int i,j,x,y,oldTotal,newTotal,score,bestScore=-10000;
    oldTotal=0;for(i=1;i<TACTICS_UNITS;i++)oldTotal+=s->units[i].hp;
    best=*s;
    for(y=0;y<6;y++)for(x=0;x<8;x++) {
        TacticsState moved=*s;
        if(x!=s->units[0].x||y!=s->units[0].y)if(!TacticsMove(&moved,x,y))continue;
        for(i=-1;i<moved.deckCount;i++)for(j=0;j<2;j++) {
            int target,k;
            if(i>=0&&moved.piles[i]!=1)continue;
            for(target=0;target<TACTICS_UNITS;target++) {
                candidate=moved;
                if(i>=0&&!TacticsPlay(&candidate,i,moved.units[target].x,moved.units[target].y,j))continue;
                if(i<0)TacticsReload(&candidate);
                newTotal=0;for(score=1;score<TACTICS_UNITS;score++)newTotal+=candidate.units[score].hp;
                score=(oldTotal-newTotal)*10;
                TacticsEndTurn(&candidate);
                score+=(candidate.hp-s->hp)*7;
                if(candidate.phase==TACTICS_LOST)score-=10000;
                if(candidate.phase==TACTICS_REWARD)score+=1000;
                /* Approach on turns with no targets; preserve distance when ranged enemies aim. */
                for(k=1;k<TACTICS_UNITS;k++)if(candidate.units[k].hp) {
                    int dx=candidate.units[0].x-candidate.units[k].x;
                    int dy=candidate.units[0].y-candidate.units[k].y;
                    score-=(dx<0?-dx:dx)+(dy<0?-dy:dy);
                }
                if(score>bestScore){bestScore=score;best=candidate;sBotX=x;sBotY=y;sBotCard=i;sBotSleight=j;sBotTargetX=moved.units[target].x;sBotTargetY=moved.units[target].y;}
            }
        }
    }
    *s=best;
}
static void TestRuns(void) {
    TacticsState s,replay,loaded;TacByte save[512];TacWord generation;
    int seed,step,wins=0,losses=0,i,choice;
    for(seed=1;seed<=30;seed++) {
        TacticsNewRun(&s,seed);TacticsNewRun(&replay,seed);assert(memcmp(&s,&replay,sizeof(s))==0);
        for(i=0;i<12;i++)if(i%4==3)assert(s.routes[i][0]==ROOM_BOSS&&s.routes[i][1]==ROOM_BOSS);
        for(step=0;step<1000&&s.phase!=TACTICS_CLEAR&&s.phase!=TACTICS_LOST;step++) {
            assert(TacticsValid(&s));
            assert(TacticsSaveEncode(&s,step,save));assert(TacticsSaveDecode(&loaded,&generation,save));assert(memcmp(&s,&loaded,sizeof(s))==0);
            if(s.phase==TACTICS_MAP) {
                choice=s.routes[s.stage][0]==ROOM_REST?0:s.routes[s.stage][1]==ROOM_REST?1:0;
                assert(TacticsChooseRoom(&s,choice));assert(TacticsChooseRoom(&replay,choice));
            } else if(s.phase==TACTICS_REWARD) {
                choice=s.hp<s.maxHp-4?3:0;assert(TacticsReward(&s,choice));assert(TacticsReward(&replay,choice));
            } else {BotTurn(&s);BotTurn(&replay);}
            assert(memcmp(&s,&replay,sizeof(s))==0);
        }
        assert(step<1000);assert(TacticsValid(&s));
        if(s.phase==TACTICS_CLEAR)wins++;else losses++;
    }
    assert(wins>0);printf("full deterministic runs: %d wins, %d losses across 30 seeds\n",wins,losses);
}
int main(void) {TestCommands();TestSaves();TestRuns();puts("tactics rules/save/run checks passed");return 0;}
