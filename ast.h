#ifndef _AST_CPP_
#define _AST_CPP_


#include <stdio.h>
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
extern A_exp root;
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

extern A_exp A_StructExp(A_pos pos, A_exp var, A_exp exp);
extern A_exp A_ArrayExp(A_pos pos, A_exp var, A_exp exp);
extern A_exp A_Op1Exp(A_pos pos, A_exp exp, A_oper oper);
extern A_exp A_call(A_pos pos, S_symbol func, A_expList args);
extern A_exp A_OpExp(A_pos pos, A_oper oper, A_exp left, A_exp right);
extern A_exp A_AssignExp(A_pos pos, A_exp var, A_exp exp);
extern A_exp A_IfExp(A_pos pos, A_exp test, A_expList then, A_expList elsee);
extern A_exp A_VarExp(A_pos pos, A_var var);
extern A_exp A_WhileExp(A_pos pos, A_exp test, A_expList body);



extern void print_space(int times);
extern void print_tree(int level, A_exp root);
extern void gen_ic(A_exp root);
extern void push_AST(A_expList*, A_exp);
#endif