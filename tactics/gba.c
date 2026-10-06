/* Standalone bitmap presentation; rules and run state remain in tactics.c. */
#include "tactics.h"
#include "save.h"
#include "tactics_assets.h"
#include "songs.h"
#include "m4a.h"
#include "m4a_song.h"
#define REG16(addr) (*(volatile unsigned short *)(addr))
#define COLOR(r,g,b) ((r)|((g)<<5)|((b)<<10))
#define KEY_A 1
#define KEY_B 2
#define KEY_SELECT 4
#define KEY_START 8
#define KEY_RIGHT 16
#define KEY_LEFT 32
#define KEY_UP 64
#define KEY_DOWN 128
#define KEY_R 256
#define KEY_L 512
#define UI_MOVE 0
#define UI_CARD 1
#define UI_END 2
#define UI_RELOAD 3
#define UI_SUSPEND 4

TacticsState gTacticsRun;
unsigned int gTacticsFrames;
static TacByte sSaveBytes[TACTICS_SAVE_SIZE*2];
static TacWord sGeneration, sSeed;
static int sSaveSlot, sHasSave, sTitle, sPage, sDirty, sCursorX, sCursorY;
static int sChoice, sUi, sCard, sSleight, sConfirm, sLock, sMusic, sAnimTotal;
static TacticsUnit sPreviousUnits[TACTICS_UNITS];
static unsigned short sKeys;
static const char *sNotice;
static volatile unsigned short *sCanvas;

static const char sGlyphChars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-/+.?!<> ";
static const unsigned char sGlyphs[][5]={
 {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},
 {127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},
 {0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},
 {127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},
 {62,65,81,33,94},{127,9,25,41,70},{38,73,73,73,50},{1,1,127,1,1},
 {63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},
 {7,8,112,8,7},{97,81,73,69,67},
 {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},
 {24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},
 {54,73,73,73,54},{6,73,73,41,30},
 {0,54,54,0,0},{8,8,8,8,8},{32,16,8,4,2},{8,8,62,8,8},
 {0,96,96,0,0},{2,1,81,9,6},{0,0,95,0,0},{8,20,34,65,0},{65,34,20,8,0},{0,0,0,0,0}
};
static const char *sCardNames[4]={"KEY","FIRE","CURE","GUARD"};
static const char *sRoomNames[4]={"BATTLE","REST","ELITE","BOSS"};
static const char *sTerrainNames[3]={"OPEN COURT","STONE PILLARS","NARROW HALL"};

static void Pixel(int x,int y,int c) {
    unsigned short value;int index;
    if(x<0||x>=240||y<0||y>=160)return;
    index=y*120+(x>>1);value=sCanvas[index];
    sCanvas[index]=(x&1)?(value&255)|(c<<8):(value&0xff00)|c;
}
static void Rect(int x,int y,int w,int h,int c) {
    int yy,xx,right,bottom;unsigned short packed=c|(c<<8);
    right=x+w;bottom=y+h;
    if(x<0)x=0;if(y<0)y=0;if(right>240)right=240;if(bottom>160)bottom=160;
    if(x>=right||y>=bottom)return;
    for(yy=y;yy<bottom;yy++) {
        xx=x;
        if(xx&1){Pixel(xx,yy,c);xx++;}
        while(xx+1<right){sCanvas[yy*120+(xx>>1)]=packed;xx+=2;}
        if(xx<right)Pixel(xx,yy,c);
    }
}
static void Border(int x,int y,int w,int h,int c) {
    Rect(x,y,w,1,c);Rect(x,y+h-1,w,1,c);Rect(x,y,1,h,c);Rect(x+w-1,y,1,h,c);
}
static void Text(int x,int y,const char *text,int color) {
    int i,col,row;char ch;
    while((ch=*text++)!=0) {
        for(i=0;sGlyphChars[i]&&sGlyphChars[i]!=ch;i++);
        if(!sGlyphChars[i])i=45;
        for(col=0;col<5;col++)for(row=0;row<7;row++)
            if(sGlyphs[i][col]&(1<<row))Pixel(x+col,y+row,color);
        x+=6;
    }
}
static void Number(int x,int y,TacWord n,int color) {
    char buf[11];int length=0,i;TacWord div=1000000000u;
    for(i=0;i<10;i++){int digit=n/div;n%=div;div/=10;if(digit||length||i==9)buf[length++]='0'+digit;}
    buf[length]=0;Text(x,y,buf,color);
}
static void Actor(int x,int y,const unsigned char *pixels) {
    int xx,yy,c;
    for(yy=0;yy<24;yy++)for(xx=0;xx<24;xx++)if((c=pixels[yy*24+xx]))Pixel(x+xx,y+yy,c);
}
static void CaptureUnits(void) {
    int i;for(i=0;i<TACTICS_UNITS;i++)sPreviousUnits[i]=gTacticsRun.units[i];
}
static void Animate(int frames) {sLock=frames;sAnimTotal=frames;}
static void Clear(void) {int i;for(i=0;i<19200;i++)sCanvas[i]=0x0101;}
static void Header(const char *label) {
    Rect(0,0,240,23,2);Text(8,7,label,7);
    Text(146,7,"HP",6);Number(165,7,gTacticsRun.hp,7);Text(180,7,"/",6);Number(188,7,gTacticsRun.maxHp,6);
    Border(0,22,240,1,4);
}
static void DrawTitle(void) {
    int i;
    Text(65,16,"KINGDOM HEARTS",6);Text(61,33,"CHAIN OF MEMORIES",7);
    Text(91,54,"TACTICS",8);
    for(i=0;i<5;i++){Rect(22+i*10,71-i*4,6,46+i*4,2);Rect(178+i*10,71-i*4,6,46+i*4,2);}
    Actor(108,70,sSoraPixels);
    Text(65,103,"A  NEW CASTLE RUN",7);
    Text(65,116,sHasSave?"B  RESUME SUSPEND":"B  HOW TO PLAY",6);
    Text(65,130,"SEED",6);Number(101,130,sSeed,8);
    Text(50,146,"LEFT/RIGHT CHANGE SEED",5);
}
static void DrawHelp(void) {
    Header("HOW TO PLAY");
    Text(8,30,"MOVE 3 CELLS AND PLAY 1 CARD",7);
    Text(8,43,"B SWITCHES MOVE/CARDS",6);
    Text(8,56,"L/R SELECT CARD  A PREVIEWS",6);
    Text(8,69,"A AGAIN COMMITS  B CANCELS",6);
    Text(8,82,"RED CELLS: ENEMY TARGETS",9);
    Text(8,95,"DODGE OR BREAK WITH HIGH VALUE",6);
    Text(8,108,"L+R: SLEIGHT  FIRST EXHAUSTS",6);
    Text(8,121,"START: END TURN / RELOAD",6);
    Text(8,134,"SELECT: SAVE AND RETURN TITLE",6);
    Text(8,149,"A/B RETURN",8);
}
static void DrawMap(void) {
    int i,y;
    Header("CASTLE OBLIVION");Text(8,29,"FLOOR",6);Number(46,29,gTacticsRun.stage/4+1,8);
    Text(88,29,"ROOM",6);Number(121,29,gTacticsRun.stage%4+1,8);
    for(i=0;i<2;i++) {
        y=46+i*41;Rect(12,y,216,35,sChoice==i?3:2);Border(12,y,216,35,sChoice==i?8:4);
        Text(23,y+7,sRoomNames[gTacticsRun.routes[gTacticsRun.stage][i]],7);
        Text(23,y+21,gTacticsRun.routes[gTacticsRun.stage][i]==ROOM_REST?"RESTORE 8 HP":sTerrainNames[gTacticsRun.terrain[gTacticsRun.stage][i]],6);
    }
    Text(12,136,"UP/DOWN CHOOSE  A ENTER",7);Text(12,149,"SELECT SUSPEND",5);
}
static void DrawRewards(void) {
    int i,y;
    Header("ROOM CLEARED");Text(8,29,"CHOOSE YOUR MEMORY",8);
    for(i=0;i<4;i++) {
        y=43+i*22;Rect(12,y,216,19,sChoice==i?3:2);Border(12,y,216,19,sChoice==i?8:4);
        if(i==3)Text(22,y+6,"VITALITY +2 MAX HP",7);
        else if(gTacticsRun.deckCount>=TACTICS_DECK)Text(22,y+6,"DECK FULL: +1 POWER",7);
        else {Text(22,y+6,sCardNames[gTacticsRun.rewards[i].kind],7);Text(104,y+6,"VALUE",6);Number(145,y+6,gTacticsRun.rewards[i].value,8);}
    }
    Text(12,147,"UP/DOWN CHOOSE  A CLAIM",6);
}
static void DrawEnd(void) {
    Header(gTacticsRun.phase==TACTICS_CLEAR?"CASTLE CONQUERED":"MEMORIES LOST");
    Actor(108,38,sSoraPixels);
    Text(37,75,gTacticsRun.phase==TACTICS_CLEAR?"ALL THREE FLOORS CLEARED":"YOUR NEXT RUN AWAITS",8);
    Text(30,96,"SEED",6);Number(72,96,gTacticsRun.seed,7);
    Text(30,111,"VICTORIES",6);Number(96,111,gTacticsRun.victories,7);
    Text(30,126,"DECK",6);Number(72,126,gTacticsRun.deckCount,7);
    Text(30,146,"A RETURN TO TITLE",8);
}
static void DrawBattle(void) {
    int x,y,i,index,distance,card,target=0,damage=0;
    TacticsPreview preview;TacByte distances[48];
    TacticsMovementMap(&gTacticsRun,distances);
    Header("TACTICS");Text(65,7,"TURN",6);Number(96,7,gTacticsRun.turn,8);
    for(y=0;y<6;y++)for(x=0;x<8;x++) {
        int color=2;
        index=y*8+x;
        if(sUi==UI_MOVE&&!gTacticsRun.moved) {distance=distances[index];if(distance>0&&distance<=3)color=3;}
        for(i=1;i<TACTICS_UNITS;i++)if(TacticsThreatens(&gTacticsRun,i,x,y)) color=10;
        Rect(8+x*24,30+y*16,23,15,color);
        if(gTacticsRun.blocked[index]) {Rect(11+x*24,32+y*16,17,11,4);Border(11+x*24,32+y*16,17,11,5);}
    }
    for(i=1;i<TACTICS_UNITS;i++)if(gTacticsRun.units[i].hp) {
        TacticsIntent *in=&gTacticsRun.intents[i];
        Border(11+in->x*24,33+in->y*16,17,9,in->broken?4:9);
        if(TacticsThreatens(&gTacticsRun,i,sCursorX,sCursorY)) damage+=in->damage;
    }
    for(i=0;i<TACTICS_UNITS;i++)if(gTacticsRun.units[i].hp) {
        TacticsUnit *u=&gTacticsRun.units[i];x=8+u->x*24;y=30+u->y*16;
        if(sLock&&sAnimTotal) {
            x+=(sPreviousUnits[i].x-u->x)*24*sLock/sAnimTotal;
            y+=(sPreviousUnits[i].y-u->y)*16*sLock/sAnimTotal;
        }
        Actor(x,y-10,!i?sSoraPixels:u->kind==ENEMY_MAGE?sMagePixels:
            u->kind==ENEMY_KNIGHT?sKnightPixels:u->kind==ENEMY_BOSS?sBossPixels:sShadowPixels);
        Rect(x+4,y+12,16,2,1);Rect(x+4,y+12,16*u->hp/u->maxHp,2,i?9:11);
        if(i) {Pixel(x+21,y-2,9);if(u->kind==ENEMY_MAGE)Text(x+18,y-9,"F",12);else if(u->kind>=ENEMY_KNIGHT)Text(x+18,y-9,"K",8);}
        if(u->x==sCursorX&&u->y==sCursorY)target=i;
    }
    Border(8+sCursorX*24,30+sCursorY*16,23,15,8);
    Text(205,30,"HP",6);Number(205,40,gTacticsRun.units[target].hp,7);
    if(target) {Text(205,55,"VAL",6);Number(205,65,gTacticsRun.units[target].value,8);}
    Text(205,82,"HIT",9);Number(205,92,damage,9);
    Text(205,108,gTacticsRun.moved?"M-":"M+",gTacticsRun.moved?5:11);
    Text(205,118,gTacticsRun.acted?"C-":"C+",gTacticsRun.acted?5:11);
    for(i=0;i<5;i++) {
        card=TacticsHandCard(&gTacticsRun,i);if(card<0)continue;
        x=5+i*47;Rect(x,133,44,24,sUi==UI_CARD&&sCard==i?3:2);Border(x,133,44,24,sUi==UI_CARD&&sCard==i?8:4);
        Text(x+3,136,sCardNames[gTacticsRun.deck[card].kind],7);Number(x+3,146,gTacticsRun.deck[card].value,8);
        if(gTacticsRun.acted)Text(x+29,146,"-",5);
    }
    if(sUi==UI_CARD) {
        card=TacticsHandCard(&gTacticsRun,sCard);
        preview=TacticsPreviewCard(&gTacticsRun,card,sCursorX,sCursorY,sSleight);
        Rect(8,23,192,7,1);
        if(sConfirm)Text(8,23,"A CONFIRM  B CANCEL",8);
        else if(preview.legal) {
            Text(8,23,sSleight?"SLEIGHT":"CARD",6);
            if(card>=0 && gTacticsRun.deck[card].kind==CARD_CURE && !sSleight) {Text(63,23,"HEAL",11);Number(94,23,preview.heal,11);}
            else if(preview.shield) {Text(63,23,"BLOCK",11);Number(99,23,preview.shield,11);}
            else {Text(63,23,preview.breaks?"DMG":"BROKEN",preview.breaks?8:9);if(preview.breaks)Number(91,23,preview.damage,8);}
        } else Text(8,23,"CHOOSE A VALID TARGET",5);
    } else Text(8,23,sNotice?sNotice:"A MOVE  B CARDS  START MENU",6);
    if(sUi>=UI_END) {
        Rect(22,50,195,65,1);Border(22,50,195,65,8);
        if(sUi==UI_SUSPEND) {Text(32,63,"SUSPEND THIS RUN?",7);Text(32,80,"A SAVE  B CANCEL",6);}
        else {Text(32,61,sUi==UI_END?"> END TURN":"  END TURN",7);Text(32,78,sUi==UI_RELOAD?"> RELOAD CARDS":"  RELOAD CARDS",7);Text(32,99,"A CONFIRM  B CANCEL",6);}
    }
}
static void Render(void) {
    sCanvas=(volatile unsigned short *)(sPage?0x0600A000:0x06000000);
    Clear();
    if(sTitle==1)DrawTitle();else if(sTitle==2)DrawHelp();
    else if(gTacticsRun.phase==TACTICS_MAP)DrawMap();
    else if(gTacticsRun.phase==TACTICS_REWARD)DrawRewards();
    else if(gTacticsRun.phase==TACTICS_CLEAR||gTacticsRun.phase==TACTICS_LOST)DrawEnd();
    else DrawBattle();
    if(sUi==UI_SUSPEND && !sTitle && gTacticsRun.phase!=TACTICS_PLAYER) {
        Rect(22,50,195,65,1);Border(22,50,195,65,8);
        Text(32,63,"SUSPEND THIS RUN?",7);Text(32,80,"A SAVE  B CANCEL",6);
    }
    while(REG16(0x04000006)<160);
    REG16(0x04000000)=0x0404|(sPage?0x10:0);sPage^=1;sDirty=0;
}
static int ReadSuspend(void) {
    volatile TacByte *ram=(volatile TacByte *)0x0E000000;int i;
    for(i=0;i<TACTICS_SAVE_SIZE*2;i++)sSaveBytes[i]=ram[i];
    sSaveSlot=TacticsSaveSelect(&gTacticsRun,&sGeneration,sSaveBytes,sSaveBytes+TACTICS_SAVE_SIZE);
    return sSaveSlot>=0;
}
static int WriteSuspend(void) {
    volatile TacByte *ram;int i,slot=sSaveSlot==0?1:0;
    if(!TacticsSaveEncode(&gTacticsRun,sGeneration+1,sSaveBytes))return 0;
    ram=(volatile TacByte *)(0x0E000000+slot*TACTICS_SAVE_SIZE);
    /* Commit the magic last; an interrupted write leaves the previous slot usable. */
    for(i=0;i<4;i++)ram[i]=0;
    for(i=4;i<TACTICS_SAVE_SIZE;i++)ram[i]=sSaveBytes[i];
    for(i=0;i<4;i++)ram[i]=sSaveBytes[i];
    for(i=0;i<TACTICS_SAVE_SIZE;i++)if(ram[i]!=sSaveBytes[i])return 0;
    sSaveSlot=slot;sGeneration++;sHasSave=1;return 1;
}
static void ResetBattleUi(void) {
    CaptureUnits();sLock=0;sAnimTotal=0;
    sUi=UI_MOVE;sCursorX=gTacticsRun.units[0].x;sCursorY=gTacticsRun.units[0].y;
    sCard=0;sSleight=0;sConfirm=0;sNotice=0;sChoice=0;
}
static void MoveCursor(int keys) {
    if(keys&KEY_LEFT&&sCursorX>0)sCursorX--;
    if(keys&KEY_RIGHT&&sCursorX<7)sCursorX++;
    if(keys&KEY_UP&&sCursorY>0)sCursorY--;
    if(keys&KEY_DOWN&&sCursorY<5)sCursorY++;
    if(keys&0xf0)sConfirm=0;
}
static void Input(int keys,int held) {
    int card,result=0;
    if(!keys)return;
    sDirty=1;sNotice=0;
    if(sTitle==2) {if(keys&(KEY_A|KEY_B))sTitle=1;return;}
    if(sTitle==1) {
        if(keys&KEY_LEFT){if(sSeed>1)sSeed--;}
        if(keys&KEY_RIGHT){sSeed++;if(sSeed>65535)sSeed=1;}
        if(keys&KEY_A){TacticsNewRun(&gTacticsRun,sSeed);sTitle=0;ResetBattleUi();}
        else if(keys&KEY_B){if(sHasSave&&ReadSuspend()){sTitle=0;ResetBattleUi();}else sTitle=2;}
        return;
    }
    if(gTacticsRun.phase==TACTICS_CLEAR||gTacticsRun.phase==TACTICS_LOST) {
        if(keys&KEY_A)sTitle=1;return;
    }
    if(sUi==UI_SUSPEND) {
        if(keys&KEY_B)sUi=UI_MOVE;
        else if(keys&KEY_A) {if(WriteSuspend()){sTitle=1;sUi=UI_MOVE;}else{sUi=UI_MOVE;sNotice="SAVE FAILED - TRY AGAIN";}}
        return;
    }
    if(keys&KEY_SELECT){sUi=UI_SUSPEND;sConfirm=0;return;}
    if(gTacticsRun.phase==TACTICS_MAP) {
        if(keys&(KEY_UP|KEY_DOWN))sChoice^=1;
        if(keys&KEY_A){TacticsChooseRoom(&gTacticsRun,sChoice);ResetBattleUi();}
        return;
    }
    if(gTacticsRun.phase==TACTICS_REWARD) {
        if(keys&KEY_UP)sChoice=(sChoice+3)%4;
        if(keys&KEY_DOWN)sChoice=(sChoice+1)%4;
        if(keys&KEY_A){TacticsReward(&gTacticsRun,sChoice);ResetBattleUi();}
        return;
    }
    if(sUi==UI_END||sUi==UI_RELOAD) {
        if(keys&KEY_B)sUi=UI_MOVE;
        else if(keys&(KEY_UP|KEY_DOWN))sUi=sUi==UI_END?UI_RELOAD:UI_END;
        else if(keys&KEY_A){CaptureUnits();result=sUi==UI_END?TacticsEndTurn(&gTacticsRun):TacticsReload(&gTacticsRun);sUi=UI_MOVE;sConfirm=0;sCard=0;if(result)Animate(12);else sNotice="NO DISCARDS TO RELOAD";}
        return;
    }
    if(keys&KEY_START){sUi=UI_END;sConfirm=0;return;}
    if(keys&KEY_B){if(sConfirm)sConfirm=0;else{sUi=sUi==UI_MOVE?UI_CARD:UI_MOVE;sSleight=0;}return;}
    MoveCursor(keys);
    if(sUi==UI_CARD) {
        if((held&(KEY_L|KEY_R))==(KEY_L|KEY_R)&&(keys&(KEY_L|KEY_R))) {sSleight^=1;sConfirm=0;}
        else if(keys&(KEY_L|KEY_R)) {
            int count=TacticsHandCount(&gTacticsRun);
            if(count)sCard=(sCard+(keys&KEY_R?1:count-1))%count;
            sConfirm=0;
        }
        card=TacticsHandCard(&gTacticsRun,sCard);
        if(card>=0 && (gTacticsRun.deck[card].kind==CARD_CURE||gTacticsRun.deck[card].kind==CARD_GUARD)&&!sSleight){sCursorX=gTacticsRun.units[0].x;sCursorY=gTacticsRun.units[0].y;}
        if(keys&KEY_A){
            if(sConfirm){CaptureUnits();result=TacticsPlay(&gTacticsRun,card,sCursorX,sCursorY,sSleight);if(result){Animate(12);sConfirm=0;sUi=UI_MOVE;sCard=0;sSleight=0;sChoice=0;}}
            else if(TacticsPreviewCard(&gTacticsRun,card,sCursorX,sCursorY,sSleight).legal)sConfirm=1;
        }
    } else if(keys&KEY_A) {
        CaptureUnits();result=TacticsMove(&gTacticsRun,sCursorX,sCursorY);
        if(result)Animate(8);else sNotice="MOVE UNAVAILABLE";
    }
}
static void Music(void) {
    int track=sTitle?SONG_BGM_EVENT2:gTacticsRun.room==ROOM_BOSS?SONG_BGM_BOSS1_WORLD:SONG_BGM_TOWN_BTL;
    if(track!=sMusic){m4aSongNumStart(track);sMusic=track;}
}
void TacticsMain(void) {
    int i;unsigned short held,pressed;volatile TacWord *ewram=(volatile TacWord *)0x02000000;
    REG16(0x04000208)=0;REG16(0x04000200)=0;REG16(0x04000000)=0x80;REG16(0x04000204)=0x4317;
    for(i=0;i<0x10000;i++)ewram[i]=0;
    sTitle=1;sSeed=1;sDirty=1;sPage=0;sSaveSlot=-1;sMusic=-1;
    sHasSave=ReadSuspend();if(!sHasSave)TacticsNewRun(&gTacticsRun,1);
    for(i=0;i<256;i++)((volatile unsigned short *)0x05000000)[i]=sActorPalette[i];
    ((volatile unsigned short *)0x05000000)[0]=COLOR(1,2,4);
    ((volatile unsigned short *)0x05000000)[1]=COLOR(2,3,6);
    ((volatile unsigned short *)0x05000000)[2]=COLOR(3,6,10);
    ((volatile unsigned short *)0x05000000)[3]=COLOR(4,10,13);
    ((volatile unsigned short *)0x05000000)[4]=COLOR(8,12,15);
    ((volatile unsigned short *)0x05000000)[5]=COLOR(13,17,20);
    ((volatile unsigned short *)0x05000000)[6]=COLOR(21,24,26);
    ((volatile unsigned short *)0x05000000)[7]=COLOR(29,30,29);
    ((volatile unsigned short *)0x05000000)[8]=COLOR(31,24,10);
    ((volatile unsigned short *)0x05000000)[9]=COLOR(31,10,10);
    ((volatile unsigned short *)0x05000000)[10]=COLOR(12,4,7);
    ((volatile unsigned short *)0x05000000)[11]=COLOR(8,27,21);
    ((volatile unsigned short *)0x05000000)[12]=COLOR(12,19,31);
    m4aSoundInit();m4aSoundVSyncOn();
    for(;;) {
        while(REG16(0x04000006)>=160);
        while(REG16(0x04000006)<160);
        gTacticsFrames++;
        m4aSoundVSync();m4aSoundMain();Music();
        held=(~REG16(0x04000130))&0x3ff;pressed=held&~sKeys;sKeys=held;
        if(sLock){sLock--;if(!(sLock&1))sDirty=1;}else Input(pressed,held);
        if(sDirty)Render();
    }
}
