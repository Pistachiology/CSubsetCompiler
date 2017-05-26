#ifndef _IRCSUBSET_C_
#define _IRCSUBSET_C_

#include "ast.h"
#define MAX_VAR_LENGTH 256

enum IRInstruction {
    irfjmp, irtjmp, irjmp, irassign, irarray_assign, ircjmp, irarray_access,
    irfield_access, iraddr_ins, ircopy, irload, irstore, irilabel, irinop, ircall,
    irminus
};

enum IRVarType {
    irvar, ircons, irlabel, irregs, irexpr, irnone, ircall_args, irarray
};

/*
enum {ineg, iadd, isub, imul, idiv, imod, iparam, ienter, ileave, iend, iload, istore, imove,
      icmpeq, icmplt, icmple, iblbs, iblbc, icall, ibr, iret, iread, iwrite, iwrl, inop, ige, igeq, 
      icmpge, icmpgt, iminus, ivar};
*/

typedef char IROp;

union IRVar {
    long cons;
    char regs;
    char name[MAX_VAR_LENGTH];
    char label[MAX_VAR_LENGTH];
    struct {
        union {
            int regs;
            char name[MAX_VAR_LENGTH];
        } u;
        union IRVar *next;
    } args;
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
    enum IRVarType utype;
    IRExpression* e1;
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

extern IRCode* create_ircode(enum IRInstruction iri, union IRVar u, IRExpression* e1, enum IRVarType utype,IRCode *next);
extern void push_ircode(IRCode **ircodes, enum IRInstruction iri, union IRVar u, IRExpression* e1, enum IRVarType utype, IRCode *next) ;
extern IRExpression* create_irexpression(IROp irop, union IRVar e1, enum IRVarType e1_type, 
                                                   union IRVar e2, enum IRVarType e2_type);
extern IRFunction* create_function(char *name, IRCode *irc);
extern void push_function(IRFunctions **irfs, IRFunction *irf);
extern IRFunctions* do_gen_ir(A_expList expList);


IRCode* _do_parse_A_exp(A_exp exp, int regs, int is_store);
void _push_ircode(IRCode **, IRCode*);
void do_print_ircode(IRCode *ircodes);
void do_print_irexpression(IRExpression* ire) ;
void do_print_irvar(union IRVar irvar, enum IRVarType type)  ;

#endif