#include "tactics.h"
static int Abs(int n) { return n < 0 ? -n : n; }
static int Manhattan(int x, int y, int a, int b) { return Abs(x-a)+Abs(y-b); }
static TacWord Random(TacticsState *s) {
    s->rng = s->rng * 1664525u + 1013904223u;
    return s->rng;
}
static void Zero(void *p, int size) {
    TacByte *out = (TacByte *)p;
    while (size--) *out++ = 0;
}
static int Occupied(const TacticsState *s, int x, int y, int except) {
    int i;
    for (i=0; i<TACTICS_UNITS; i++)
        if (i!=except && s->units[i].hp && s->units[i].x==x && s->units[i].y==y) return 1;
    return 0;
}
static void Distances(const TacticsState *s, int actor, TacByte *dist) {
    TacByte queue[TACTICS_CELLS];
    int head, tail, cell, next, nx, ny, d, i;
    static const signed char dx[4]={1,-1,0,0}, dy[4]={0,0,1,-1};
    for(i=0;i<TACTICS_CELLS;i++) dist[i]=255;
    cell=s->units[actor].y*TACTICS_WIDTH+s->units[actor].x;
    head=0; tail=1; queue[0]=cell; dist[cell]=0;
    while(head<tail) {
        cell=queue[head++];
        for(d=0;d<4;d++) {
            nx=cell%TACTICS_WIDTH+dx[d]; ny=cell/TACTICS_WIDTH+dy[d];
            if(nx<0||nx>=TACTICS_WIDTH||ny<0||ny>=TACTICS_HEIGHT) continue;
            next=ny*TACTICS_WIDTH+nx;
            if(dist[next]!=255||s->blocked[next]||Occupied(s,nx,ny,actor)) continue;
            dist[next]=dist[cell]+1; queue[tail++]=next;
        }
    }
}
void TacticsMovementMap(const TacticsState *s,TacByte *dist) { Distances(s,0,dist); }
int TacticsMoveDistance(const TacticsState *s,int x,int y) {
    TacByte dist[TACTICS_CELLS];
    if(x<0||x>=TACTICS_WIDTH||y<0||y>=TACTICS_HEIGHT) return -1;
    Distances(s,0,dist);
    return dist[y*TACTICS_WIDTH+x]==255?-1:dist[y*TACTICS_WIDTH+x];
}
int TacticsHandCount(const TacticsState *s) {
    int n=0,i;
    for(i=0;i<s->deckCount;i++) if(s->piles[i]==1) n++;
    return n;
}
int TacticsHandCard(const TacticsState *s,int position) {
    int i;
    if(position<0) return -1;
    for(i=0;i<s->deckCount;i++) if(s->piles[i]==1 && position--==0) return i;
    return -1;
}
static void Draw(TacticsState *s) {
    int pool[TACTICS_DECK], n,i, card;
    while(TacticsHandCount(s)<TACTICS_HAND) {
        n=0;
        for(i=0;i<s->deckCount;i++) if(!s->piles[i]) pool[n++]=i;
        if(!n) break;
        card=pool[(Random(s)>>16)%n]; s->piles[card]=1;
    }
}
void TacticsPlanIntents(TacticsState *s) {
    TacticsState forecast;
    TacByte dist[TACTICS_CELLS];
    int i,cell,best,bestScore,score,range,steps,x,y;
    forecast=*s;
    for(i=1;i<TACTICS_UNITS;i++) {
        TacticsIntent *intent=&s->intents[i];
        TacticsUnit *u=&forecast.units[i];
        Zero(intent,sizeof(*intent));
        intent->x=u->x; intent->y=u->y;
        intent->targetX=s->units[0].x; intent->targetY=s->units[0].y;
        if(!u->hp) continue;
        range=u->kind==ENEMY_MAGE?4:1;
        steps=u->kind==ENEMY_KNIGHT?1:2;
        Distances(&forecast,i,dist);
        best=u->y*TACTICS_WIDTH+u->x;
        bestScore=Manhattan(u->x,u->y,intent->targetX,intent->targetY)*16;
        for(cell=0;cell<TACTICS_CELLS;cell++) {
            if(dist[cell]>steps) continue;
            x=cell%TACTICS_WIDTH; y=cell/TACTICS_WIDTH;
            score=Manhattan(x,y,intent->targetX,intent->targetY);
            if(score<=range) score=0;
            score=score*16+dist[cell];
            if(score<bestScore) { bestScore=score; best=cell; }
        }
        u->x=best%TACTICS_WIDTH; u->y=best/TACTICS_WIDTH;
        intent->x=u->x; intent->y=u->y;
        if(Manhattan(u->x,u->y,intent->targetX,intent->targetY)<=range)
            intent->damage= u->kind==ENEMY_BOSS?4:u->kind==ENEMY_KNIGHT?3:2;
    }
}
void TacticsNewRun(TacticsState *s,TacWord seed) {
    int stage,choice,i;
    Zero(s,sizeof(*s)); s->seed=seed; s->rng=seed;
    s->maxHp=24; s->hp=24; s->deckCount=10;
    for(i=0;i<10;i++) { s->deck[i].kind=i<5?CARD_KEY:i<7?CARD_FIRE:i<9?CARD_CURE:CARD_GUARD; s->deck[i].value=4+i%5; }
    for(stage=0;stage<TACTICS_STAGES;stage++) for(choice=0;choice<2;choice++) {
        s->routes[stage][choice]= stage%4==3?ROOM_BOSS:
            ((Random(s)>>16)%5==0?ROOM_REST:choice==1 && stage%4==2?ROOM_ELITE:ROOM_BATTLE);
        s->terrain[stage][choice]=(Random(s)>>16)%3;
    }
    s->routes[0][0]=ROOM_BATTLE; s->routes[0][1]=ROOM_BATTLE;
    s->phase=TACTICS_MAP;
}
static void Advance(TacticsState *s) {
    s->stage++;
    s->phase=s->stage>=TACTICS_STAGES?TACTICS_CLEAR:TACTICS_MAP;
}
int TacticsChooseRoom(TacticsState *s,int choice) {
    int i,level,modifier;
    if(s->phase!=TACTICS_MAP||choice<0||choice>1||s->stage>=TACTICS_STAGES) return 0;
    s->room=s->routes[s->stage][choice];
    if(s->room==ROOM_REST) {
        s->hp+=8; if(s->hp>s->maxHp) s->hp=s->maxHp;
        Advance(s); return 1;
    }
    Zero(s->blocked,sizeof(s->blocked)); Zero(s->units,sizeof(s->units));
    Zero(s->intents,sizeof(s->intents)); Zero(s->piles,sizeof(s->piles));
    modifier=s->terrain[s->stage][choice];
    if(modifier) { s->blocked[2*8+3]=1; s->blocked[3*8+3]=1; }
    if(modifier==2) { s->blocked[1*8+4]=1; s->blocked[4*8+4]=1; }
    s->units[0].x=1; s->units[0].y=2; s->units[0].hp=s->hp; s->units[0].maxHp=s->maxHp;
    level=s->stage/4;
    for(i=1;i<(s->room==ROOM_BATTLE?3:4);i++) {
        TacticsUnit *u=&s->units[i];
        u->x=6; u->y=i==1?1:i==2?4:2;
        u->kind=i==1?ENEMY_SHADOW:i==2?ENEMY_MAGE:ENEMY_KNIGHT;
        u->hp=5+level*2+(u->kind==ENEMY_KNIGHT?3:0);
        if(s->room==ROOM_BOSS&&i==3) {u->kind=ENEMY_BOSS;u->hp=14+level*4;}
        u->maxHp=u->hp;u->value=3+level+(i==3?2:0);
    }
    s->phase=TACTICS_PLAYER; s->moved=0;s->acted=0;s->shield=0;s->turn=1;
    Draw(s); TacticsPlanIntents(s); return 1;
}
void TacticsInit(TacticsState *s) { TacticsNewRun(s,1);TacticsChooseRoom(s,0); }
int TacticsMove(TacticsState *s,int x,int y) {
    int d;
    if(s->phase!=TACTICS_PLAYER||s->moved) return 0;
    d=TacticsMoveDistance(s,x,y);
    if(d<1||d>3) return 0;
    s->units[0].x=x;s->units[0].y=y;s->moved=1;return 1;
}
static int SleightCards(const TacticsState *s,int selected,int *out) {
    int i,n=0;
    if(selected<0||selected>=s->deckCount||s->piles[selected]!=1) return 0;
    out[n++]=selected;
    for(i=0;i<s->deckCount && n<3;i++) if(i!=selected&&s->piles[i]==1) out[n++]=i;
    return n;
}
TacticsPreview TacticsPreviewCard(const TacticsState *s,int card,int x,int y,int sleight) {
    TacticsPreview p;
    int distance,i,ids[3],value,kind,found=0;
    Zero(&p,sizeof(p));
    if(s->phase!=TACTICS_PLAYER||s->acted||card<0||card>=s->deckCount||s->piles[card]!=1||
        x<0||x>=8||y<0||y>=6) return p;
    kind=s->deck[card].kind; value=s->deck[card].value;
    if(sleight) {
        if(SleightCards(s,card,ids)!=3) return p;
        value=s->deck[ids[0]].value+s->deck[ids[1]].value+s->deck[ids[2]].value;
        kind=CARD_FIRE;
    }
    distance=Manhattan(s->units[0].x,s->units[0].y,x,y);
    if(kind==CARD_CURE||kind==CARD_GUARD) {
        if(x!=s->units[0].x||y!=s->units[0].y) return p;
        if(kind==CARD_CURE) { p.heal=5+value; if(p.heal>s->maxHp-s->units[0].hp) p.heal=s->maxHp-s->units[0].hp; }
        else p.shield=3+value/2;
        p.legal=1;return p;
    }
    if(distance<1||distance>(kind==CARD_KEY?1:4)) return p;
    for(i=1;i<TACTICS_UNITS;i++) if(s->units[i].hp &&
        Manhattan(x,y,s->units[i].x,s->units[i].y)<=(sleight?1:0)) {
        found=1;
        if(!value||value>=s->units[i].value||sleight) p.breaks++;
    }
    if(!found) return p;
    p.damage=(sleight?6+value/2:kind==CARD_KEY?3+value/3:4+value/3)+s->power;
    if(!p.breaks) p.damage=0;
    p.legal=1;return p;
}
static void Win(TacticsState *s) {
    int i;
    s->hp=s->units[0].hp;s->victories++;
    for(i=0;i<3;i++) {
        s->rewards[i].kind=(Random(s)>>16)%4;
        s->rewards[i].value=5+(Random(s)>>16)%5;
    }
    s->phase=TACTICS_REWARD;
}
int TacticsPlay(TacticsState *s,int card,int x,int y,int sleight) {
    TacticsPreview p=TacticsPreviewCard(s,card,x,y,sleight);
    int i,alive=0,ids[3],value;
    if(!p.legal) return 0;
    value=s->deck[card].value;
    s->acted=1; s->piles[card]=2;
    if(sleight) {
        s->piles[card]=1; SleightCards(s,card,ids);
        s->piles[ids[0]]=3; s->piles[ids[1]]=2; s->piles[ids[2]]=2;
    }
    s->units[0].hp+=p.heal;s->hp=s->units[0].hp;
    if(p.shield) s->shield=p.shield;
    if(p.damage) for(i=1;i<TACTICS_UNITS;i++) if(s->units[i].hp &&
        Manhattan(x,y,s->units[i].x,s->units[i].y)<=(sleight?1:0)) {
        if(value && value<s->units[i].value && !sleight) continue;
        s->units[i].hp=s->units[i].hp>p.damage?s->units[i].hp-p.damage:0;
        s->intents[i].broken=1;
    }
    for(i=1;i<TACTICS_UNITS;i++) alive+=s->units[i].hp!=0;
    if(!alive) Win(s);
    return 1;
}
int TacticsAttack(TacticsState *s,int target) {
    int i;
    if(target<1||target>=TACTICS_UNITS) return 0;
    for(i=0;i<s->deckCount;i++) if(s->piles[i]==1&&s->deck[i].kind==CARD_KEY)
        return TacticsPlay(s,i,s->units[target].x,s->units[target].y,0);
    return 0;
}
int TacticsReload(TacticsState *s) {
    int i,n=0;
    if(s->phase!=TACTICS_PLAYER||s->acted) return 0;
    for(i=0;i<s->deckCount;i++) n+=s->piles[i]==2;
    if(!n) return 0;
    for(i=0;i<s->deckCount;i++) if(s->piles[i]==2) s->piles[i]=0;
    s->acted=1; Draw(s); return 1;
}
int TacticsThreatens(const TacticsState *s,int enemy,int x,int y) {
    const TacticsIntent *in;
    int distance;
    if(enemy<1||enemy>=TACTICS_UNITS||!s->units[enemy].hp)return 0;
    in=&s->intents[enemy];
    if(!in->damage||in->broken)return 0;
    distance=Manhattan(x,y,in->targetX,in->targetY);
    if(s->units[enemy].kind==ENEMY_BOSS)
        return distance<=2&&(x==in->targetX||y==in->targetY);
    if(s->units[enemy].kind==ENEMY_MAGE||s->units[enemy].kind==ENEMY_KNIGHT) return distance<=1;
    return distance==0;
}
int TacticsEndTurn(TacticsState *s) {
    int i,damage;
    if(s->phase!=TACTICS_PLAYER) return 0;
    for(i=1;i<TACTICS_UNITS;i++) {
        TacticsIntent *in=&s->intents[i];
        if(!s->units[i].hp||in->broken) continue;
        if(!s->blocked[in->y*8+in->x]&&!Occupied(s,in->x,in->y,i)) {
            s->units[i].x=in->x;s->units[i].y=in->y;
        }
        if(TacticsThreatens(s,i,s->units[0].x,s->units[0].y) &&
            Manhattan(s->units[i].x,s->units[i].y,in->targetX,in->targetY)<=
            (s->units[i].kind==ENEMY_MAGE?4:1)) {
            damage=in->damage;
            if(s->shield>=damage) {s->shield-=damage;damage=0;}
            else {damage-=s->shield;s->shield=0;}
            s->units[0].hp=s->units[0].hp>damage?s->units[0].hp-damage:0;
            if(!s->units[0].hp) {s->hp=0;s->phase=TACTICS_LOST;return 1;}
        }
    }
    s->hp=s->units[0].hp;s->shield=0;s->moved=0;s->acted=0;
    if(s->turn<255) s->turn++;
    Draw(s); TacticsPlanIntents(s);return 1;
}
int TacticsReward(TacticsState *s,int choice) {
    if(s->phase!=TACTICS_REWARD||choice<0||choice>3) return 0;
    if(choice==3) { if(s->maxHp<60) {s->maxHp+=2;if(s->maxHp>60)s->maxHp=60;} s->hp+=2;if(s->hp>s->maxHp)s->hp=s->maxHp; }
    else if(s->deckCount<TACTICS_DECK) s->deck[s->deckCount++]=s->rewards[choice];
    else { if(s->power<10) s->power++; }
    Advance(s);return 1;
}
int TacticsValid(const TacticsState *s) {
    int i,j;
    if(sizeof(TacWord)!=4||s->deckCount<3||s->deckCount>TACTICS_DECK||s->stage>TACTICS_STAGES||
        s->maxHp<1||s->maxHp>60||s->hp>s->maxHp||s->power>10||s->victories>12||
        s->moved>1||s->acted>1||s->shield>12||s->room>ROOM_BOSS) return 0;
    if(s->phase!=TACTICS_PLAYER&&s->phase!=TACTICS_MAP&&s->phase!=TACTICS_REWARD&&
        s->phase!=TACTICS_CLEAR&&s->phase!=TACTICS_LOST) return 0;
    if(s->phase==TACTICS_LOST&&s->hp) return 0;
    if((s->phase==TACTICS_CLEAR)!=(s->stage==TACTICS_STAGES)) return 0;
    for(i=0;i<48;i++) if(s->blocked[i]>1) return 0;
    for(i=0;i<s->deckCount;i++) if(s->deck[i].kind>3||s->deck[i].value>9||s->piles[i]>3) return 0;
    if(TacticsHandCount(s)>TACTICS_HAND) return 0;
    for(i=0;i<3;i++) if(s->rewards[i].kind>3||s->rewards[i].value>9) return 0;
    for(i=0;i<12;i++) for(j=0;j<2;j++) if(s->routes[i][j]>3||s->terrain[i][j]>2) return 0;
    for(i=0;i<TACTICS_UNITS;i++) {
        const TacticsUnit *u=&s->units[i];const TacticsIntent *in=&s->intents[i];
        if(u->x>=8||u->y>=6||u->hp>u->maxHp||u->kind>3||u->value>9||
            in->x>=8||in->y>=6||in->targetX>=8||in->targetY>=6||in->damage>4||in->broken>1) return 0;
        if(s->phase==TACTICS_PLAYER && u->hp && (s->blocked[u->y*8+u->x]||Occupied(s,u->x,u->y,i))) return 0;
    }
    if(s->phase==TACTICS_PLAYER && (!s->hp||s->hp!=s->units[0].hp||s->maxHp!=s->units[0].maxHp)) return 0;
    return 1;
}
