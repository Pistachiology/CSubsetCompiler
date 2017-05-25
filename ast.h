#ifndef _AST_CPP_
#define _AST_CPP_


#include "css.h"
#include "csg.h"
#include "cfg.h"

typedef struct A_exp_ *A_exp;
typedef struct A_expList_ *A_expList;
typedef char* S_symbol;
typedef char* A_var;
enum AST_operator { 
    AST_add, AST_minus, AST_mult, AST_mod, AST_eq, AST_neq, AST_gt, AST_ge, 
    AST_lt, AST_le
};
typedef unsigned int A_pos; 
A_exp root;
struct A_expList_{
    A_exp exp;
    A_expList next;
};
struct A_exp_
{
    enum {  A_varExp, A_intExp, A_callExp,
            A_opExp, A_assignExp, A_ifExp,
            A_whileExp, A_arrayExp, A_op1Exp,
            A_structExp
    } kind;
    A_pos pos;
    union {
        A_var var;
    /* nil; - needs only the pos */
        int intt;
        struct { 
            S_symbol func;
            A_expList args;
        } call;
        struct { 
            A_oper oper; 
            A_exp left; 
            A_exp right;
        } op;

        struct {
            A_oper oper;
            A_exp left;
        } op1;
        struct { 
            A_exp var; 
            A_exp exp; 
        } assign;
        struct { 
            A_exp test;
            A_expList then, elsee; 
        } iff; /* elsee is optional */ 
        struct { 
            A_exp test;
            A_expList body; 
        } whilee;
        struct {
            A_exp var;
            A_exp exp;
        } structt;

        struct {
            A_exp var;
            A_exp exp;
        } array;
    } u; 
};

A_exp A_StructExp(A_pos pos, A_exp var, A_exp exp){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_structExp;
    p->pos = pos;
    p->u.structt.var = var;
    p->u.structt.exp = exp;
    return p;
}

A_exp A_ArrayExp(A_pos pos, A_exp var, A_exp exp){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_arrayExp;
    p->pos = pos;
    p->u.array.var = var;
    p->u.array.exp = exp;
    return p;
}

A_exp A_Op1Exp(A_pos pos, A_exp exp, A_oper oper){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_op1Exp;
    p->pos = pos;
    p->u.op1.left = exp;
    p->u.op1.oper = oper;
    return p;
}
A_exp A_call(A_pos pos, S_symbol func, A_expList args){
    A_exp p = malloc(sizeof(*p));
    p->pos = pos;
    p->kind = A_callExp;
    p->u.call.func = func;
    p->u.call.args = args;
    return p;
}

A_exp A_OpExp(A_pos pos, A_oper oper, A_exp left, A_exp right){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_opExp;
    p->pos = pos;
    p->u.op.oper = oper;
    p->u.op.left = left;
    p->u.op.right = right;
    return p;
}

A_exp A_AssignExp(A_pos pos, A_exp var, A_exp exp){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_assignExp;
    p->u.assign.var = var;
    p->u.assign.exp = exp;
    return p;
}

A_exp A_IfExp(A_pos pos, A_exp test, A_expList then, A_expList elsee){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_ifExp;
    p->u.iff.test = test;
    p->u.iff.then = then;
    p->u.iff.elsee = elsee;
    return p;
}

A_exp A_VarExp(A_pos pos, A_var var){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_varExp;
    p->u.var = var;
}
A_exp A_WhileExp(A_pos pos, A_exp test, A_expList body){
    A_exp p = malloc(sizeof(*p));
    p->kind = A_whileExp;
    p->u.whilee.test = test;
    p->u.whilee.body = body;
    return p;
}



void print_space(int times){
    printf("%02d", times);
    for(int i = 0; i < times; i++) printf(" ");
}
void print_tree(int level, A_exp root){
    if(!root) return;
    print_space(level * 5);
    switch(root->kind){
        case A_callExp:
            printf("%s\n", root->u.call.func);
            for(A_expList it = root->u.call.args; it; it = it->next){
                print_tree(level + 1, it->exp);
            }
            break;
        case A_intExp:
            printf("%d\n", root->u.intt);
            break;
        case A_varExp:
            printf("%s\n", root->u.var);
            break;
        case A_opExp:
        case A_op1Exp: // we don't really need this one.
            switch(root->u.op.oper){
                case CSStimes:
                    printf("*\n");
                    break;
                case CSSdiv:
                    printf("/\n");
                    break;
                case CSSmod:
                    printf("%%\n");
                    break;
                case CSSplus:
                    printf("+\n");
                    break;

                case CSSminus:
                    printf("-\n");
                    break;
                case CSSeql:
                    printf("=\n");
                    break;
                case CSSneq:
                    printf("!=\n");
                    break;
                case CSSlss:
                    printf("<\n");
                    break;
                case CSSleq:
                    printf("<=\n");
                    break;
                case CSSgtr:
                    printf(">\n");
                    break;
                case CSSgeq:
                    printf(">=\n");
                    break;
                case CSSperiod:
                    printf(".\n");
                    break;
                case CSScomma:
                    printf(",\n");
                    break;
                case CSSrparen:
                    printf(")\n");
                    break;
                case CSSrbrak:
                    printf("]\n");
                    break;
                case CSSrbrace:
                    printf("}\n");
                    break;
                case CSSlparen: printf("(\n"); break;
                case CSSlbrak: printf("[\n"); break;
                case CSSlbrace: printf("{\n"); break;
                case CSSident: printf("ident\n"); break;
                case CSSsemicolon: printf("semicolon\n"); break;
                case CSSelse: printf("else\n"); break;
                case CSSif: printf("if\n"); break;

                default:
                    printf("found :D %d\n", root->u.op.oper);
            }
            print_tree(level + 1, root->u.op.left);
            if(root->kind != A_op1Exp) print_tree(level + 1, root->u.op.right);
            break;
        case A_assignExp:
            printf("=\n");
            print_tree(level + 1, root->u.assign.var);
            print_tree(level + 1, root->u.assign.exp);
            break;
        case A_ifExp:
            printf("If\n");
            print_tree(level + 1, root->u.iff.test);
            for(A_expList it = root->u.iff.then; it; it = it->next){
                print_tree(level + 1, it->exp);
            }
            if(root->u.iff.elsee) {
                for(A_expList it = root->u.iff.elsee; it; it = it->next){
                    print_tree(level + 1, it->exp);
                }
            }
            break;
        case A_whileExp:
            printf("while\n");
            print_tree(level + 1, root->u.whilee.test);
            for(A_expList it = root->u.whilee.body; it; it = it->next){
                print_tree(level + 1, it->exp);
            }
            break;
        case A_arrayExp:
            printf("array\n");
            print_tree(level + 1, root->u.array.var);
            print_tree(level + 1, root->u.array.exp);
            break;
        case A_structExp:
            if(root->u.structt.exp){
                printf(".\n");
                print_tree(level + 1, root->u.structt.var);
                print_tree(level + 1, root->u.structt.exp);
            } else {
                
                /* fixed bug it's shouldn't enter here */
                assert(1 == 2);
                printf("struct_%s\n", root->u.structt.var->u.var);
            }
            break;
        default:
            printf("what?\n");
    }
}

void gen_ic(int level, A_exp root){
    if(!root) return;
    switch(root->kind){
        case A_callExp:
            for(A_expList it = root->u.call.args; it; it = it->next){

            }
            break;
        case A_intExp:
            printf("%d\n", root->u.intt);
            break;
        case A_varExp:
            printf("%s\n", root->u.var);
            break;
        case A_opExp:
        case A_op1Exp: // we don't really need this one.
            switch(root->u.op.oper){
                case CSStimes:
                    printf("*\n");
                    break;
                case CSSdiv:
                    printf("/\n");
                    break;
                case CSSmod:
                    printf("%%\n");
                    break;
                case CSSplus:
                    printf("+\n");
                    break;

                case CSSminus:
                    printf("-\n");
                    break;
                case CSSeql:
                    printf("=\n");
                    break;
                case CSSneq:
                    printf("!=\n");
                    break;
                case CSSlss:
                    printf("<\n");
                    break;
                case CSSleq:
                    printf("<=\n");
                    break;
                case CSSgtr:
                    printf(">\n");
                    break;
                case CSSgeq:
                    printf(">=\n");
                    break;
                case CSSperiod:
                    printf(".\n");
                    break;
                case CSScomma:
                    printf(",\n");
                    break;
                case CSSrparen:
                    printf(")\n");
                    break;
                case CSSrbrak:
                    printf("]\n");
                    break;
                case CSSrbrace:
                    printf("}\n");
                    break;
                case CSSlparen: printf("(\n"); break;
                case CSSlbrak: printf("[\n"); break;
                case CSSlbrace: printf("{\n"); break;
                case CSSident: printf("ident\n"); break;
                case CSSsemicolon: printf("semicolon\n"); break;
                case CSSelse: printf("else\n"); break;
                case CSSif: printf("if\n"); break;

                default:
                    printf("found :D %d\n", root->u.op.oper);
            }
            print_tree(level + 1, root->u.op.left);
            if(root->kind != A_op1Exp) print_tree(level + 1, root->u.op.right);
            break;
        case A_assignExp:
            printf("=\n");
            print_tree(level + 1, root->u.assign.var);
            print_tree(level + 1, root->u.assign.exp);
            break;
        case A_ifExp:
            printf("If\n");
            print_tree(level + 1, root->u.iff.test);
            for(A_expList it = root->u.iff.then; it; it = it->next){
                print_tree(level + 1, it->exp);
            }
            if(root->u.iff.elsee) {
                for(A_expList it = root->u.iff.elsee; it; it = it->next){
                    print_tree(level + 1, it->exp);
                }
            }
            break;
        case A_whileExp:
            printf("while\n");
            print_tree(level + 1, root->u.whilee.test);
            for(A_expList it = root->u.whilee.body; it; it = it->next){
                print_tree(level + 1, it->exp);
            }
            break;
        case A_arrayExp:
            printf("array\n");
            print_tree(level + 1, root->u.array.var);
            print_tree(level + 1, root->u.array.exp);
            break;
        case A_structExp:
            if(root->u.structt.exp){
                printf(".\n");
                print_tree(level + 1, root->u.structt.var);
                print_tree(level + 1, root->u.structt.exp);
            } else {
                
                /* fixed bug it's shouldn't enter here */
                assert(1 == 2);
                printf("struct_%s\n", root->u.structt.var->u.var);
            }
            break;
        default:
            printf("what?\n");
    }
}
#endif