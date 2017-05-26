#ifndef _CSubCFG_H_
#define _CSubCFG_H_

#include <stdio.h>
#include "ast.h"
#include "csg.h"
#include "ir.h"


typedef struct _leaderList {
    int id;
    IRCode *node;
    struct _leaderList *next;
} _leaderList;
typedef struct BlockDesc Block; 

typedef struct BlockDesc {
  Block *fail, *branch; // jump targets
  IRCode *first, *last; // pointer to first and last instruction in basic block
  int block_id;
} BlockDesc;

extern Block *block;
extern int leaders_count;
extern Block** genCFG(IRCode*);
extern void printCFG(Block**);
#endif
