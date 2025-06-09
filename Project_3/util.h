#pragma once
#include <stdlib.h>

typedef struct LoopList {
  int loopID;
  struct LoopList* outer; // 指向上一層
} LoopList;

LoopList* createLoopList(int ID, LoopList* outerLoop);
LoopList* freeLoopList(LoopList* list);
