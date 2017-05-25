
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ir.h"

void do_gen_ir(A_expList expList) {
    A_exp exp;
    for(A_expList it = expList; it; it->next){
        exp = it->exp;
        assert(exp->kind == A_callExp);
        gen_ic(it->exp);
        if (strcmp(exp->u.call.func, "main") == 0){

        } else {

        }
    }
}

IRCode create_ircode(enum IRInstruction iri, union IRVar u, IRExpression e1, IRCode *next) {
    IRCode irc = malloc(sizeof(IRCode));
    irc->iri = iri;
    irc->u = u;
    irc->e1 = e1;
    irc->next = next;
    return irc;
}

IRExpression create_irexpression(enum IROp irop, union IRVar e1, enum IRVar e1_type, 
                                            union IRVar e2, enum IRVar e2_type) {
    IRExpression ire = malloc(sizeof(IRExpression));
    ire->irop = irop;
    ire->e1 = e1;
    ire->e1_type = e1_type;
    ire->e2_type = e2_type;
    ire->e2 = e2;
}

void push_ircode(IRCode **ircodes, enum IRInstruction iri, union IRVar u, IRExpression e1, IRCode *next) {
    IRCode tmp = create_ircode(iri, u, e1, next);
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