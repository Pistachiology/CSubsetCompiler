
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ir.h"
#include "css.h"
IRFunction irfuncs;
int label_gen_count = 0;

IRFunctions* do_gen_ir(A_expList expList) {
    A_exp exp, exp2;
    IRFunctions* irfuncs = NULL;
    IRFunction* irf = NULL;
    IRCode *irc;
    for(A_expList it = expList; it; it = it->next){
        exp = it->exp;
        assert(exp->kind == A_callExp);
        irc = NULL;
        for(A_expList it2 = exp->u.call.args; it2; it2 = it2->next) {
            exp2 = it2->exp;
            _push_ircode(&irc, _do_parse_A_exp(exp2, 1, 0));
        }
        irf = create_function(exp->u.call.func, irc);
        push_function(&irfuncs, irf);
        printf("func %s\n", exp->u.call.func);
        do_print_ircode(irc);
        printf("\n\n");
    }

}

IRExpression* _do_parse_ire(A_exp exp, int regs, int is_store) {
    IRExpression* ire = NULL;
    assert(exp != NULL);
    switch (exp->kind) {
        case A_varExp:
        break;
        case A_intExp:
        break;
    }
    assert(ire != NULL);

}

IRCode* _do_parse_A_exp(A_exp exp, int regs, int is_store) {
    IRCode *irc = NULL;
    union IRVar tmp_irvar[3];
    char *new_label[3]; 
    switch (exp->kind) {
        case A_varExp:
            tmp_irvar[0].regs = regs;
            strcpy(tmp_irvar[1].name, exp->u.var);
            push_ircode(&irc, irassign, tmp_irvar[0],
                create_irexpression(CSSvar,
                    tmp_irvar[1], irvar,
                    tmp_irvar[2], irnone
                )
            , irregs, NULL);
            break;
        case A_intExp:
            tmp_irvar[0].regs = regs;
            tmp_irvar[1].cons = exp->u.intt;
            push_ircode(&irc, irassign, tmp_irvar[0],
                create_irexpression(CSSvar,
                    tmp_irvar[1], ircons,
                    tmp_irvar[2], irnone
                )
            , irregs, NULL);
            //assert(1 == 2);
            break;
        case A_callExp:
            {
                int i = 0; // var i shouldn't exist for long
                union IRVar *irvar_it = NULL;
                for(A_expList tmpit = exp->u.call.args; tmpit; tmpit = tmpit->next, i++) {
                    _push_ircode(&irc, _do_parse_A_exp(tmpit->exp, regs + i , 1)); // T [s]
                }
                tmp_irvar[0].args.next = NULL;
                strcpy(tmp_irvar[0].args.u.name, exp->u.call.func);
                irvar_it = &tmp_irvar[0];
                for(int j = 0; j < i; j++) {
                    int reg_num = regs + j;
                    irvar_it->args.next = malloc(sizeof(union IRVar));
                    irvar_it = irvar_it->args.next;
                    irvar_it->args.u.regs = reg_num;
                    irvar_it->args.next = NULL;
                }
                push_ircode(&irc, ircall, tmp_irvar[0], NULL, ircall_args, NULL);
            }
            break;
        case A_opExp:
            _push_ircode(&irc, _do_parse_A_exp(exp->u.op.left, regs + 1, 1) );
            _push_ircode(&irc, _do_parse_A_exp(exp->u.op.right, regs + 2, 1) );
            tmp_irvar[0].regs = (regs);
            tmp_irvar[1].regs = (regs + 1);
            tmp_irvar[2].regs = (regs + 2);
            push_ircode(&irc, irassign, tmp_irvar[0], 
                create_irexpression(
                    exp->u.op.oper, 
                    tmp_irvar[1], irregs,
                    tmp_irvar[2], irregs
                ), irregs, NULL);
        break;
        case A_assignExp:
            assert(exp->u.assign.var->kind == A_varExp || exp->u.assign.var->kind == A_arrayExp);
            _push_ircode(&irc, _do_parse_A_exp(exp->u.assign.exp, regs, 1));
            if(exp->u.assign.var->kind == A_varExp) {
                strcpy(tmp_irvar[0].name, exp->u.assign.var->u.var);
                tmp_irvar[1].regs = (regs);
                tmp_irvar[2].regs = (0);
                push_ircode(&irc, irassign, tmp_irvar[0],
                            create_irexpression(
                                CSSvar,
                                tmp_irvar[1], irregs,
                                tmp_irvar[2], irnone),
                            irvar, NULL);
            } else {
                A_exp array_exp = exp->u.assign.var;
                _push_ircode(&irc, _do_parse_A_exp(exp->u.array.exp, regs + 1, 1));


                strcpy(tmp_irvar[0].name, exp->u.array.var->u.var);
                tmp_irvar[1].regs = regs + 2;
                // load $t2 base_var
                push_ircode(&irc, irload, tmp_irvar[0], create_irexpression(
                    CSSmanual, tmp_irvar[1], irregs, tmp_irvar[2], irnone
                ), irarray, NULL);

                // $t3 = $t1 * 4 
                tmp_irvar[0].regs = regs + 3;
                tmp_irvar[1].regs = regs + 1;
                tmp_irvar[2].cons = 4;
                push_ircode(&irc, irassign, tmp_irvar[0], create_irexpression(
                    CSStimes, tmp_irvar[1], irregs, tmp_irvar[2], ircons
                ), irregs, NULL);

                // $t4 = $t3 + $t2
                tmp_irvar[0].regs = regs + 4;
                tmp_irvar[1].regs = regs + 2;
                tmp_irvar[2].regs = regs + 3;
                push_ircode(&irc, irassign, tmp_irvar[0], create_irexpression(
                    CSSplus, tmp_irvar[1], irregs, tmp_irvar[2], irregs
                ), irregs, NULL);

                // store $t0 $t4
                tmp_irvar[0].regs = regs;
                tmp_irvar[1].regs = regs + 4;
                push_ircode(&irc, irstore, tmp_irvar[0], create_irexpression(
                    CSSmanual, tmp_irvar[1], irregs, tmp_irvar[2], irnone
                ), irregs, NULL);
                /*
                _push_ircode(&irc, _do_parse_A_exp(array_exp->u.array.exp, regs + 1, 1));
                strcpy(tmp_irvar[0].name, array_exp->u.array.var->u.var);
                tmp_irvar[1].regs = (regs + 1);
                tmp_irvar[2].regs = (regs);
                push_ircode(&irc, irarray_assign, tmp_irvar[2],
                            create_irexpression(
                                CSSarray,
                                tmp_irvar[0], irvar,
                                tmp_irvar[1], irregs),
                            irregs, NULL);
                */
            }
        break;
        case A_ifExp:
            /* if then */
            if(!exp->u.iff.elsee) {
                /* t1 = T[e] ; fjump t1 Lend ; T[s] ; label Lend */
                _push_ircode(&irc, _do_parse_A_exp(exp->u.iff.test, regs, 1)); // T [e]
                tmp_irvar[1].regs = regs;
                sprintf(tmp_irvar[0].label, "Lend%d", label_gen_count);
                push_ircode(&irc, irfjmp, tmp_irvar[0], create_irexpression(
                    CSSvar,
                    tmp_irvar[1], irregs,
                    tmp_irvar[2], irnone
                ), irlabel, NULL);
                for(A_expList tmpit = exp->u.iff.then; tmpit; tmpit = tmpit->next)
                    _push_ircode(&irc, _do_parse_A_exp(tmpit->exp, regs, 1)); // T [s]
                push_ircode(&irc, irilabel, tmp_irvar[0], NULL, irlabel, NULL); // label Lend
            } else {
            /* if then else */
                _push_ircode(&irc, _do_parse_A_exp(exp->u.iff.test, regs, 1)); // T [e]
                tmp_irvar[1].regs = regs;
                sprintf(tmp_irvar[0].label, "Lfalse%d", label_gen_count);
                push_ircode(&irc, irfjmp, tmp_irvar[0], create_irexpression(
                    CSSvar,
                    tmp_irvar[1], irregs,
                    tmp_irvar[2], irnone
                ), irlabel, NULL);
                for(A_expList tmpit = exp->u.iff.then; tmpit; tmpit = tmpit->next)
                    _push_ircode(&irc, _do_parse_A_exp(tmpit->exp, regs, 1)); // T [s]
                sprintf(tmp_irvar[0].label, "Lend%d", label_gen_count);
                push_ircode(&irc, irjmp, tmp_irvar[0], create_irexpression(
                    CSSvar,
                    tmp_irvar[1], irnone,
                    tmp_irvar[2], irnone
                ), irlabel, NULL); // jmp Lend
                sprintf(tmp_irvar[0].label, "Lfalse%d", label_gen_count);
                push_ircode(&irc, irilabel, tmp_irvar[0], NULL, irlabel, NULL); // label false
                for(A_expList tmpit = exp->u.iff.elsee; tmpit; tmpit = tmpit->next)
                    _push_ircode(&irc, _do_parse_A_exp(tmpit->exp, regs, 1)); // T [s]
                sprintf(tmp_irvar[0].label, "Lend%d", label_gen_count);
                push_ircode(&irc, irilabel, tmp_irvar[0], NULL, irlabel, NULL); // label end
            }
            label_gen_count++;

        break;
        case A_whileExp:
            sprintf(tmp_irvar[0].label, "Ltest%d", label_gen_count);
            push_ircode(&irc, irilabel, tmp_irvar[0], NULL, irlabel, NULL); // label Ltest
            _push_ircode(&irc, _do_parse_A_exp(exp->u.whilee.test, regs, 1)); // T [e]
            sprintf(tmp_irvar[0].label, "Lend%d", label_gen_count);
            tmp_irvar[1].regs = regs;
            push_ircode(&irc, irfjmp, tmp_irvar[0], create_irexpression(
                CSSvar,
                tmp_irvar[1], irregs,
                tmp_irvar[2], irnone
            ), irlabel, NULL);
            for(A_expList tmpit = exp->u.whilee.body; tmpit; tmpit = tmpit->next)
                _push_ircode(&irc, _do_parse_A_exp(tmpit->exp, regs, 1)); // T [s]
            sprintf(tmp_irvar[0].label, "Ltest%d", label_gen_count);
            push_ircode(&irc, irjmp, tmp_irvar[0], create_irexpression(
                CSSvar,
                tmp_irvar[1], irnone,
                tmp_irvar[2], irnone
            ), irlabel, NULL);
            sprintf(tmp_irvar[0].label, "Lend%d", label_gen_count);
            push_ircode(&irc, irilabel, tmp_irvar[0], NULL, irlabel, NULL); // label Ltest
            label_gen_count++;
        break;
        case A_arrayExp:
            assert(exp->kind == A_arrayExp);
            assert(exp->u.array.var->kind == A_varExp);
            tmp_irvar[0].regs = regs;
            tmp_irvar[2].regs = regs + 1;
            _push_ircode(&irc, _do_parse_A_exp(exp->u.array.exp, regs + 1, 1));
            {
                
                strcpy(tmp_irvar[0].name, exp->u.array.var->u.var);
                tmp_irvar[1].regs = regs + 2;
                // load $t2 base_var
                push_ircode(&irc, irload, tmp_irvar[0], create_irexpression(
                    CSSmanual, tmp_irvar[1], irregs, tmp_irvar[2], irnone
                ), irarray, NULL);

                // $t3 = $t1 * 4 
                tmp_irvar[0].regs = regs + 3;
                tmp_irvar[1].regs = regs + 1;
                tmp_irvar[2].cons = 4;
                push_ircode(&irc, irassign, tmp_irvar[0], create_irexpression(
                    CSStimes, tmp_irvar[1], irregs, tmp_irvar[2], ircons
                ), irregs, NULL);

                // $t4 = $t3 + $t2
                tmp_irvar[0].regs = regs + 4;
                tmp_irvar[1].regs = regs + 2;
                tmp_irvar[2].regs = regs + 3;
                push_ircode(&irc, irassign, tmp_irvar[0], create_irexpression(
                    CSSplus, tmp_irvar[1], irregs, tmp_irvar[2], irregs
                ), irregs, NULL);

                // load $t0 $t4
                tmp_irvar[0].regs = regs;
                tmp_irvar[1].regs = regs + 4;
                push_ircode(&irc, irload, tmp_irvar[0], create_irexpression(
                    CSSmanual, tmp_irvar[1], irregs, tmp_irvar[2], irnone
                ), irregs, NULL);
            }
            /*
            push_ircode(&irc, irarray_access, tmp_irvar[0], create_irexpression(
                CSSarray, tmp_irvar[1], irvar, tmp_irvar[2], irregs
            ), irregs, NULL);
            */
        break;
        case A_op1Exp:
            printf("Not Implement yet\n");
            exit(-1);
        break;
        case A_structExp:
            printf("Not Implement yet\n");
            exit(-1);
        break;
        break;
    }
    if(!irc) {
        printf("[Warning] some AST not implemented\n");
        return create_ircode(irinop, tmp_irvar[0], NULL, irnone, NULL);
    }
    return irc;
}

IRCode* create_ircode(enum IRInstruction iri, union IRVar u, IRExpression* e1, enum IRVarType utype, IRCode *next) {
    IRCode *irc = malloc(sizeof(IRCode));
    irc->iri = iri;
    irc->u = u;
    irc->e1 = e1;
    irc->utype = utype;
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
    return ire;
}

void push_ircode(IRCode **ircodes, enum IRInstruction iri, union IRVar u, IRExpression* e1, enum IRVarType utype, IRCode *next) {
    IRCode *tmp = create_ircode(iri, u, e1, utype, next);
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

void do_print_ircode(IRCode *ircodes) {
    IRCode *it = ircodes;
    while(it) {
        switch (it->iri)
        {
            case irassign:
                do_print_irvar(it->u, it->utype);
                printf(" = ");
                do_print_irexpression(it->e1); 
                break;
            case irfjmp:
                printf("fjump ");
                do_print_irexpression(it->e1);
                printf(" ");
                do_print_irvar(it->u, it->utype);
                break;
            case irjmp:
                printf("jump ");
                do_print_irvar(it->u, it->utype);
                break;
            case irilabel:
                printf("label ");
                do_print_irvar(it->u, it->utype);
                break;
            case ircall:
            {
                int push_count = 0;
                for(union IRVar *ir_it = it->u.args.next; ir_it; ir_it = ir_it->args.next) {
                    printf("push $t%d\n", ir_it->args.u.regs);
                    push_count++;
                }
                printf("call %s(", it->u.args.u.name);
                for(union IRVar *ir_it = it->u.args.next; ir_it; ir_it = ir_it->args.next) {
                    printf("$t%d", ir_it->args.u.regs);
                    if(ir_it->args.next) printf(", ");
                }
                printf(")\n");
                printf("$sp = $sp - %d", push_count * 4);
            }
                break;
            case irload:
            {
                printf("load ");
                do_print_irvar(it->e1->e1, it->e1->e1_type);
                printf(" ");
                do_print_irvar(it->u, it->utype);
            }
                break;
            case irstore:
            {
                printf("store ");
                do_print_irvar(it->e1->e1, it->e1->e1_type);
                printf(" ");
                do_print_irvar(it->u, it->utype);
                }
            break;
            case irarray_access:
            {
                assert(0);
                IRExpression *ire = it->e1;
                int reg_num = ire->e2.regs;
                printf("load $t%d base_%s\n", reg_num + 1, ire->e1.name);
                printf("$t%d = 4 * $t%d\n", reg_num + 2, reg_num);
                printf("$t%d = $t%d + $t%d\n", reg_num+3, reg_num + 2, reg_num + 1);
                printf("load ");
                do_print_irvar(it->u, it->utype);
                printf(" $t%d", reg_num+3);
                // printf(" = ");
                // do_print_irexpression(it->e1);
            }
                break;
            case irarray_assign:
            {
                assert(0);
                IRExpression *ire = it->e1;
                int reg_num = ire->e2.regs;
                printf("load $t%d base_%s\n", reg_num + 1, ire->e1.name);
                printf("$t%d = 4 * $t%d\n", reg_num + 2, reg_num);
                printf("$t%d = $t%d + $t%d\n", reg_num+3, reg_num + 2, reg_num + 1);
                printf("store ");
                do_print_irvar(it->u, it->utype);
                printf(" $t%d", reg_num+3);
                /*
                do_print_irexpression(it->e1);
                printf(" = ");
                do_print_irvar(it->u, it->utype);
                */
            }
                break;
            default:
                printf("ir_nop %d", it->iri);
        }
        printf("\n");
        it = it->next; 
    }
}

void do_print_irexpression(IRExpression* ire) {
    switch(ire->irop) {
        case CSSvar:
            do_print_irvar(ire->e1, ire->e1_type);
        break; 
        case CSSneq:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" != ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSeql:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" = ");
            do_print_irvar(ire->e2, ire->e2_type);
            break;
        case CSSleq:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" <= ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSlss:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" <= ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSgeq:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" >= ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSgtr:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" > ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSStimes:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" * ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSdiv:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" / ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSmod:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" %% ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSminus:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" - ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSplus:
            do_print_irvar(ire->e1, ire->e1_type);
            printf(" + ");
            do_print_irvar(ire->e2, ire->e2_type);
        break;
        case CSSarray:
            {
            do_print_irvar(ire->e1, ire->e1_type);
            printf("[");
            do_print_irvar(ire->e2, ire->e2_type);
            printf("]"); 
            }
        break;
        default:
            printf("working :< %d", ire->irop);

    }
}

void do_print_irvar(union IRVar var, enum IRVarType type) {
    switch(type) {
        case irvar:
            printf("%s", var.name);
            break;
        case ircons:
            printf("%ld", var.cons);
            break;
        case irlabel:
            printf("%s", var.label);
            break;
        case irregs:
            printf("$t%d", var.regs);
            break;
        case irarray:
            printf("base_%s", var.name);
            break;
        case irnone:
            printf("irnone - -a\n");
            break;
        default:
            printf("bug :(\n");
    }
}
