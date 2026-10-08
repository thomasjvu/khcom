#include <stdio.h>
#include <stdlib.h>
#include "field_jump.h"
int main(int argc, char** argv) {
    FILE* input;
    char line[256];
    int video, frame, x,y,z,ground,menu,busy,direction,move,action;
    int started=0, count=0, originX=0, originY=0;
    int speed,angle,sine,cosine;
    FieldJumpMotion motion;
    if (argc!=2 || !(input=fopen(argv[1],"r"))) return 2;
    if (!fgets(line,sizeof(line),input)) return 2;
    while (fgets(line,sizeof(line),input)) {
        if (sscanf(line,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                   &video,&frame,&x,&y,&z,&ground,&menu,&busy,&direction,&move,&action,&speed,&angle,&sine,&cosine)!=15) return 2;
        if (!started && busy==2 && direction!=0) {
            FieldJumpMotionInit(&motion,x,y,z,originX,originY,speed); started=1;
            continue;
        }
        if (!started) { originX=x; originY=y; continue; }
        if (motion.phase==3) continue;
        FieldJumpMotionStep(&motion,sine,cosine,1,ground);
        if (motion.x!=x || motion.y!=y || motion.z!=z) {
            fprintf(stderr,"frame%d predicted%d,%d,%d actual%d,%d,%d\n",frame,motion.x,motion.y,motion.z,x,y,z);
            return 1;
        }
        count++;
    }
    fclose(input);
    if (count<30 || motion.phase!=3) return 1;
    printf("Native horizontal jump trajectory: %d exact motion samples passed\n",count);
    return 0;
}
