#ifndef _IRCSUBSET_C_
#define _IRCSUBSET_C_
#define MAX_VAR_LENGTH 256


extern enum IRInstruction {
    fjmp, tjmp, jmp, assign, array_assign, cjmp, array_access,
    field_access, addr_ins, copy, load, store
};

extern enum IRVar {
    var, const, label
}

extern enum IROp {
    add, sub, div, mul,
    and, or, xor,
    eq, neq, le, leq, ge, geq,
    minus, neg
}
union IRVar{
    long cons;
    char name[MAX_VAR_LENGTH];
    char label[MAX_VAR_LENGTH];
} ;
typedef struct IRCode {
    enum IRInstruction iri;
    union IRVar u;
    IRExpression e1, e2;
} IRCode;

typedef struct IRExpression {
    enum IROp irop;
    union IRVar e1;
    union IRVar e2;

} IRExpression;

extern IRCode create_ircode(enum IRData irdata, IRExpression *l, IRExpression *r);
extern IRExpression create_irexpression(enum IROp irop);
extern void do_gen_ir(A_expList expList);

#endif
