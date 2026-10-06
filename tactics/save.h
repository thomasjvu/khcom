#ifndef TACTICS_SAVE_H
#define TACTICS_SAVE_H
#include "tactics.h"
#define TACTICS_SAVE_SIZE 512
int TacticsSaveEncode(const TacticsState *s, TacWord generation, TacByte *out);
int TacticsSaveDecode(TacticsState *s, TacWord *generation, const TacByte *data);
int TacticsSaveSelect(TacticsState *s,TacWord *generation,const TacByte *a,const TacByte *b);
#endif
