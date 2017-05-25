#ifndef _IRCSUBSET_C_
#define _IRCSUBSET_C_

#include "ast.h"
#define MAX_VAR_LENGTH 256

enum IRInstruction {
    fjmp, tjmp, jmp, assign, array_assign, cjmp, array_access,
    field_access, addr_ins, copy, load, store
};

enum IRVarType {
    var, cons, label
};

/*
enum {ineg, iadd, isub, imul, idiv, imod, iparam, ienter, ileave, iend, iload, istore, imove,
      icmpeq, icmplt, icmple, iblbs, iblbc, icall, ibr, iret, iread, iwrite, iwrl, inop, ige, igeq, 
      icmpge, icmpgt, iminus};
*/

typedef char IROp;

union IRVar {
    long cons;
    char name[MAX_VAR_LENGTH];
    char label[MAX_VAR_LENGTH];
} ;

struct IRExpression {
    IROp irop;
    union IRVar e1;
    enum IRVarType e1_type;
    union IRVar e2;
    enum IRVarType e2_type;
};
typedef struct IRExpression IRExpression;

struct IRCode {
    enum IRInstruction iri;
    union IRVar u;
    IRExpression e1;
    struct IRCode *next;
};
typedef struct IRCode IRCode;

struct IRFunction {
    char name[MAX_VAR_LENGTH];
    IRCode *irc;
};
typedef struct IRFunction IRFunction;

struct IRFunctions {
    IRFunction* irf;
    struct IRFunctions* next;
};
typedef struct IRFunctions IRFunctions;

extern IRCode* create_ircode(enum IRInstruction iri, union IRVar u, IRExpression e1, IRCode *next);
extern void push_ircode(IRCode **ircodes, enum IRInstruction iri, union IRVar u, IRExpression e1, IRCode *next) ;
extern IRExpression* create_irexpression(IROp irop, union IRVar e1, enum IRVarType e1_type, 
                                                   union IRVar e2, enum IRVarType e2_type);
extern IRFunction* create_function(char *name, IRCode *irc);
extern void push_function(IRFunctions **irfs, IRFunction *irf);
extern IRFunctions* do_gen_ir(A_expList expList);

void _push_ircode(IRCode **, IRCode*);

#endif