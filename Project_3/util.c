#include "util.h"

LoopList* createLoopList(int ID, LoopList* outerLoop) {
    LoopList* res = (LoopList*)calloc(1, sizeof(LoopList));
    res->loopID = ID;
    res->outer = outerLoop;
    return res;
}

LoopList* freeLoopList(LoopList* list) {
    LoopList* ret = list->outer;
    free(list);
    return ret;
}