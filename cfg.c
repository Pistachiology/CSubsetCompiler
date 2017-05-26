#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "csg.h"
 
typedef struct _leaderList {
    int id;
    CSGNode node;
    struct _leaderList *next;
} _leaderList;

typedef struct BlockDesc *Block; 

typedef struct BlockDesc {
  Block fail, branch; // jump targets
  CSGNode first, last; // pointer to first and last instruction in basic block
  int block_id;
} BlockDesc;

typedef struct livenessVariable{
    struct livenessVariable *next;
    CSSIdent var;
} livenessVariable;

typedef livenessVariable *liveVar;

_leaderList *leaders = NULL;

int leaders_count = 0;
int is_changed = 1;
int *visited_blocks = NULL;

Block block;

int compare (const void * a, const void * b) { 
    return ( (*(int*)a) - (*(int*)b) );
}

_leaderList *do_find_leader_id(int id){
    if(!leaders) return NULL;
    _leaderList *it = leaders;
    while(it != NULL){
        if(it->id == id) return it;
        it = it->next;
    }
    return NULL;
}


void do_insert_leader_id(int line, CSGNode node) {
    _leaderList *it = leaders;  
    if(do_find_leader_id(line)) return ;
    if(it == NULL) {
        leaders = malloc(sizeof(_leaderList));
        leaders->id = line;
        leaders->node = node;
        leaders->next = NULL;
        leaders_count++;
    }else{
        while(it->next){
            it = it->next;
        }
        it->next = malloc(sizeof(_leaderList));
        it = it->next;
        it->id = line;
        it->next = NULL;
        it->node = node;
        leaders_count++;
    }
}

Block do_create_block(CSGNode first, int block_id) { 
    Block block = malloc(sizeof(struct BlockDesc));
    block->fail = NULL;
    block->branch = NULL;
    block->first = first;
    block->last = NULL;
    block->block_id = block_id;
    return block;
}

Block find_block_by_leader_id(Block *blocks, int block_cnt, int id) {
    for(int i = 0; i < block_cnt; i++){
        if(blocks[i]->first->line == id) return blocks[i];
    }
    return NULL; 
}

void print_variables(liveVar header){
    liveVar cur_live = header->next;
    while(cur_live != NULL){
        printf("%s ", cur_live->var);
        cur_live = cur_live->next;
    }
    printf("\n");
}


liveVar create_live_var(){
    liveVar new_live = malloc(sizeof(struct livenessVariable *));
    new_live->next = NULL;
    return new_live;
}

liveVar union_var(liveVar first_header, liveVar second_header){
    liveVar temp_header = create_live_var();
    liveVar cur_temp_header = temp_header;
    liveVar cur_second_header = second_header;
    while(cur_second_header != NULL){
        int dup_var = 0;
        liveVar cur_first_header = first_header;
        while(cur_first_header != NULL){
            if(strcmp(cur_second_header->var, cur_first_header->var) == 0){
                dup_var = 1;
                break;
            }
            cur_first_header = cur_first_header->next;
        }
        if(!dup_var){
            cur_temp_header->next = create_live_var();
            cur_temp_header = cur_temp_header->next;
            strcpy(cur_temp_header->var, cur_second_header->var);
        }
        cur_second_header = cur_second_header->next;
    }
    liveVar cur_first_header = first_header;
    while(cur_first_header != NULL){
        cur_temp_header->next = create_live_var();
        cur_temp_header = cur_temp_header->next;
        strcpy(cur_temp_header->var, cur_first_header->var);
        cur_first_header = cur_first_header->next;
    }
    return temp_header;
}

void cut_name(liveVar header, CSSIdent name){
    liveVar cur_live = header;
    while(cur_live != NULL && cur_live->next != NULL){
        if(strcmp(cur_live->next->var, name) == 0){
            liveVar live_to_free = cur_live->next;
            cur_live->next = cur_live->next->next;
            free(live_to_free);
            return ;
        }
        cur_live = cur_live->next;
    }
}

int visit_block(int block_id){
    if(visited_blocks == NULL){
        visited_blocks = malloc(sizeof(int) * leaders_count);
        return 0;
    }
    if(visited_blocks[block_id - 1] == 1){
        return 1;
    }
    return 0;
}

liveVar analyze_liveness(Block cur_block){
    if(cur_block == NULL) return create_live_var();
    if(visit_block(cur_block->block_id)){
        return create_live_var();
    }
    CSGNode it = cur_block->last;
    printf("%d\n", cur_block->block_id);
    visited_blocks[cur_block->block_id - 1] = 1;
    liveVar fail_header = analyze_liveness(cur_block->fail);
    liveVar branch_header = analyze_liveness(cur_block->branch);
    liveVar header = union_var(fail_header->next, branch_header->next);
    do{
        switch(it->op){
            case iadd:
            case isub:
            case imul:
            case idiv:
            case imod:
                if(it->x->class == CSGVar){
                    liveVar temp_live = create_live_var();
                    strcpy(temp_live->var, it->x->name);
                    header = union_var(header->next, temp_live);
                }
                if(it->y->class == CSGVar){
                    liveVar temp_live = create_live_var();
                    strcpy(temp_live->var, it->y->name);
                    header = union_var(header->next, temp_live);
                }
                break;
            case imove:
                cut_name(header, it->y->name);
                if(it->x->class == CSGVar){
                    liveVar temp_live = create_live_var();
                    strcpy(temp_live->var, it->x->name);
                    header = union_var(header->next, temp_live);
                }
                break;
            default:
                break;
        }
        if(it == cur_block->first) break;
        it = it->prv;
    }while(1);
    return header;
}

liveVar liveness_manager(Block cur_block){
    liveVar header;
    while(is_changed <= 1){
        header = analyze_liveness(cur_block);
        memset(visited_blocks, 0, sizeof(int) * leaders_count);
        is_changed += 1;
    }
}

void print_CFG(Block* blocks) {
    CSGNode it;
    for(int i = 0; i < leaders_count; i++){
        printf("*** block %d ", blocks[i]->block_id);
        printf("fail "); if(blocks[i]->fail) printf("%d", blocks[i]->fail->block_id); else printf("-");
        printf(" branch "); if(blocks[i]->branch) printf("%d", blocks[i]->branch->block_id); else printf("-");
        printf("\n");
        it = blocks[i]->first;
        do {
            printf("    instr %d: ", it->line);
            switch (it->op)
            {
                case iadd:
                    printf("add");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case isub:
                    printf("sub");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case imul:
                    printf("mul");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case idiv:
                    printf("div");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case imod:
                    printf("mod");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case ineg:
                    printf("neg");
                    PrintNode(it->x);
                    break;

                case iparam:
                    printf("param");
                    PrintNode(it->x);
                    break;
                case ienter:
                    printf("enter");
                    break;
                case ileave:
                    printf("leave");
                    break;
                case iret:
                    printf("ret");
                    break;
                case iend:
                    printf("end");
                    break;

                case icall:
                    printf("call");
                    PrintNode(it->x);
                    break;
                case ibr:
                    printf("br");
                    PrintBrakNode(it->x);
                    break;

                case iblbc:
                    printf("blbc");
                    PrintNode(it->x);
                    PrintBrakNode(it->y);
                    break;
                case iblbs:
                    printf("blbs");
                    PrintNode(it->x);
                    PrintBrakNode(it->y);
                    break;
                case icmpeq:
                    printf("cmpeq");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case icmple:
                    printf("cmple");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case icmplt:
                    printf("cmplt");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case iread:
                    printf("read");
                    break;
                case iwrite:
                    printf("write");
                    PrintNode(it->x);
                    break;
                case iwrl:
                    printf("wrl");
                    break;
                case iload:
                    printf("load");
                    PrintNode(it->x);
                    break;
                case istore:
                    printf("store");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case imove:
                    printf("move");
                    PrintNode(it->x);
                    PrintNode(it->y);
                    break;
                case inop:
                    printf("nop");
                    break;
                default:
                    printf("unknown instruction");
            }
            printf("\n"); //%d %d\n", blocks[i]->last->line, it->line);
            if(it == blocks[i]->last) break;
            it = it->nxt;
        } while(1);

    }
}

Block* genCFG(void)
{
    register CSGNode i;
    register int cnt;
    int *tmp_leaders;
    Block *blocks = NULL;
    int j;
    Block tmp_block = NULL;
    leaders_count = 0;

    // assign line numbers
    cnt = 1;
    i = code;
    while (i != NULL) {
        i->line = cnt;
        cnt++;
        i = i->nxt;
    }

    i = code;
    /* find leader */
    while (i != NULL) {
        switch (i->op) {
            case ienter:
                do_insert_leader_id(i->line, i);
                break;
            case iret:
                break;
            case iend: 
                break;
/* work went here */
            case ibr: 
                do_insert_leader_id(i->nxt->line, i->nxt);
                do_insert_leader_id(i->x->line, i->x);
                break;
            case iblbc:
                do_insert_leader_id(i->y->line, i->y);
                do_insert_leader_id(i->nxt->line, i->nxt);
                break;
            case iblbs:
                do_insert_leader_id(i->y->line, i->y);
                do_insert_leader_id(i->nxt->line, i->nxt);
                break;
            /* I think we don't need this one 
             * case icmpeq: printf("cmpeq"); PrintNode(i->x); PrintNode(i->y); break;
             * case icmple: printf("cmple"); PrintNode(i->x); PrintNode(i->y); break;
             * case icmplt: printf("cmplt"); PrintNode(i->x); PrintNode(i->y); break;
             */
            /* do nothing */
            default: break;
        }

        i = i->nxt;
    }

    tmp_leaders = malloc(sizeof(int) * leaders_count);
    j = 0;
    for (_leaderList *it = leaders; j < leaders_count; j++, it = it->next) {
        tmp_leaders[j] = it->id;
    }
    qsort(tmp_leaders, leaders_count, sizeof(int), compare);

    /* construct block */ 
    blocks = malloc(sizeof(*blocks) * leaders_count);
    for (j = 0; j < leaders_count; j++) {
        blocks[j] = do_create_block(do_find_leader_id(tmp_leaders[j])->node, j + 1);
    }
    i = code;
    j = 1;

    while (i != NULL) {
            switch(i->op) {
                case iblbc:
                case iblbs:
                    if(i->line == tmp_leaders[j] - 1){
                        tmp_block = find_block_by_leader_id(blocks, leaders_count, i->y->line);
                        assert(tmp_block != NULL);
                        blocks[j-1]->branch = tmp_block;
                        assert(tmp_block != NULL);
                        tmp_block = find_block_by_leader_id(blocks, leaders_count, i->nxt->line);
                        blocks[j-1]->fail = tmp_block;
                        blocks[j-1]->last = i;
                        j++;
                    }
                break;
                case ibr:
                    if(i->line == tmp_leaders[j] - 1){
                        tmp_block = find_block_by_leader_id(blocks, leaders_count, i->x->line);
                        assert(tmp_block != NULL);
                        blocks[j-1]->branch = tmp_block;
                        blocks[j-1]->last = i;
                        j++;
                    }
                break;
                case iend:
                case iret:
                    blocks[j-1]->last = i;
                    blocks[j-1]->branch = NULL;
                    j++;
                break;
                default:
                    if(i->nxt && i->line == tmp_leaders[j] - 1) {
                        tmp_block = find_block_by_leader_id(blocks, leaders_count, i->nxt->line);
                        assert(tmp_block != NULL);
                        blocks[j-1]->fail = tmp_block;
                        blocks[j-1]->last = i;
                        j++;
                    } 
            }
        i = i->nxt;
    }
    // DEBUG Purpose
    for(int j = 0; j < leaders_count; j++){
        printf("block_id: %d firstl: %d lastl: %d branch: %d fail: %d\n", 
            blocks[j]->block_id,
            blocks[j]->first->line, blocks[j]->last->line, 
            blocks[j]->branch?blocks[j]->branch->block_id:0, 
            blocks[j]->fail?blocks[j]->fail->block_id:0);
    }
    return blocks;
}
