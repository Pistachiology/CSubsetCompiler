
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ir.h"
IRFunction irfuncs;

IRFunctions* do_gen_ir(A_expList expList) {
    A_exp exp, exp2;
    IRFunctions* irfuncs = NULL;
    IRFunction* irf = NULL;
    IRCode *irc;
    for(A_expList it = expList; it; it = it->next){
        exp = it->exp;
        assert(exp->kind == A_callExp);
        irc = NULL;
        for(A_expList it2 = exp->u.call.args; it; it = it->next) {
            exp2 = it2->exp;
            _push_ircode(&irc, _do_parse_A_exp(exp2, 1, 0));
        }
        irf = create_function(exp->u.call.func, irc);
        push_function(&irfuncs, irf);
    }
}

IRCode* _do_parse_A_exp(A_exp exp, int regs, int is_store) {
    IRCode *irc = NULL;
    switch (exp->kind) {
        case A_varExp:
        break;
        case A_intExp:
        break;
        case A_callExp:
        break;
        case A_opExp:
            _push_ircode(&irc, _do_parse_A_exp(exp->u.op.left, regs + 1, 1) );
            _push_ircode(&irc, _do_parse_A_exp(exp->u.op.right, regs + 2, 1) );
        break;
        case A_assignExp:
        break;
        case A_ifExp:
        break;
        case A_whileExp:
        break;
        case A_arrayExp:
        break;
        case A_op1Exp:
        break;
        case A_structExp:
        break;
    }

}

IRCode* create_ircode(enum IRInstruction iri, union IRVar u, IRExpression e1, IRCode *next) {
    IRCode *irc = malloc(sizeof(IRCode));
    irc->iri = iri;
    irc->u = u;
    irc->e1 = e1;
    irc->next = next;
    return irc;
}

IRExpression* create_irexpression(IROp irop, union IRVar e1, enum IRVarType e1_type, 
                                            union IRVar e2, enum IRVarType e2_type) {
    IRExpression *ire = malloc(sizeof(IRExpression));
    ire->irop = irop;
    ire->e1 = e1;
    ire->e1_type = e1_type;
    ire->e2_type = e2_type;
    ire->e2 = e2;
}

void push_ircode(IRCode **ircodes, enum IRInstruction iri, union IRVar u, IRExpression e1, IRCode *next) {
    IRCode *tmp = create_ircode(iri, u, e1, next);
    _push_ircode(ircodes, tmp);
}

void _push_ircode(IRCode **ircodes, IRCode *ircode) {
    if(!ircodes) return;
    if(!*ircodes) { 
        *ircodes = ircode;
    } else {
        IRCode *it = *ircodes;
        while(it->next) it = it->next;
        it->next = ircode;
    }
}

IRFunction* create_function(char *name, IRCode *irc) {
    IRFunction *irf = malloc(sizeof(IRFunction));
    strncpy(irf->name, name, MAX_VAR_LENGTH);
    irf->irc = irc;
}

void push_function(IRFunctions **irfs, IRFunction *irf) {
    if(!irfs) return;
    if(!*irfs) {
        *irfs = malloc(sizeof(IRFunctions));
        (*irfs)->irf = irf;
        (*irfs)->next = NULL;
    } else {
        IRFunctions *it = *irfs;
        while(it->next) it = it->next;
        it->next = malloc(sizeof(IRFunctions));
        it = it->next;
        it->irf = irf;
        it->next = NULL;
    }
}