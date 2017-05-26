#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "csg.h"
#include "ast.h"
#include "ir.h"
#include "new_cfg.h"
 

_leaderList *leaders = NULL;
int leaders_count = 0;
Block *block;

_leaderList *do_find_leader_id(int id){
    if(!leaders) return NULL;
    _leaderList *it = leaders;
    while(it != NULL){
        if(it->id == id) return it;
        it = it->next;
    }
    return NULL;
}


void do_insert_leader_id(int line, IRCode *node) {
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

Block *do_create_block(IRCode* first, int block_id) { 
    Block *block = malloc(sizeof(struct BlockDesc));
    block->fail = NULL;
    block->branch = NULL;
    block->first = first;
    block->last = NULL;
    block->block_id = block_id;
    return block;
}

Block *find_block_by_leader_id(Block **blocks, int block_cnt, int id) {
    for(int i = 0; i < block_cnt; i++){
        if(blocks[i]->first->line == id) return blocks[i];
    }
   return NULL; 
}

Block *find_block_by_label(Block **blocks, int block_cnt, char *label) {
    for(int i = 0; i < block_cnt; i++){
        if(strcmp(blocks[i]->first->u.label,label) == 0) return blocks[i];
    }
   return NULL; 
}

void printCFG(Block **blocks) {
    IRCode *irc;
    for(int j = 0; j < leaders_count; j++) {
        printf("block_id: %d firstl: %d lastl: %d branch: %d fail: %d\n", 
            blocks[j]->block_id,
            blocks[j]->first->line, blocks[j]->last->line, 
            blocks[j]->branch?blocks[j]->branch->block_id:-1, 
            blocks[j]->fail?blocks[j]->fail->block_id:-1);
        for(irc = blocks[j]->first; irc != blocks[j]->last; irc = irc->next) {
            printf("line %02d: ", irc->line);
            _do_print_ircode(irc);
        }
        printf("line %02d: ", irc->line);
        _do_print_ircode(irc);
    }
}
Block** genCFG(IRCode *ircodes)
{
    register IRCode *it_irc;
    register int cnt;
    int *tmp_leaders;
    Block **blocks = NULL;
    int j;
    Block *tmp_block = NULL;
    IRCode *first = NULL, *last = NULL;
    leaders_count = 0;

    /* find leader */
    for(it_irc = ircodes; it_irc; it_irc = it_irc->next) {
        switch(it_irc->iri) {
            case irilabel:
                first = it_irc;
                leaders_count++;
            break;
            case irret:
            case irjmp:
            case ircjmp:
            case irfjmp:
            case irtjmp:
                if (it_irc->next && it_irc->next->iri != irilabel)
                {
                leaders_count++;
                }
            break;
        }
    }
    /* construct block */ 
    blocks = malloc(sizeof(*blocks) * leaders_count);
    
    j = 0;
    for(it_irc = ircodes; it_irc; it_irc = it_irc->next) {
        switch(it_irc->iri) {
            case irilabel:
                blocks[j] = do_create_block(it_irc, j + 1);
                j++;
            break;
            case irret:
            case irjmp:
            case ircjmp:
            case irfjmp:
            case irtjmp:
                if(it_irc->next && it_irc->next->iri != irilabel) {
                    blocks[j] = do_create_block(it_irc->next, j + 1);
                    j++;
                }
            break;
        }
    }

    j = 0;
    for(it_irc = ircodes; it_irc; it_irc = it_irc->next) {
        if(it_irc->next) {
            switch (it_irc->next->iri) {
            case irilabel:
                blocks[j]->last = it_irc;
                printf("line %d\n",it_irc->line);
                j++;
                goto rdy_loop;
                break;
            }
            switch (it_irc->iri) {
            case irret:
            case irjmp:
            case ircjmp:
            case irfjmp:
            case irtjmp:
                blocks[j]->last = it_irc;
                j++;
                break;
            }
             
        } else {
            blocks[j]->last = it_irc;
            j++;
        }
        rdy_loop:  1 + 1;
    }
    for(int j = 0; j < leaders_count; j++) {
        it_irc = blocks[j]->last;
        switch(it_irc->iri) {
            case irilabel:
                blocks[j]->branch = NULL;
                blocks[j]->fail = find_block_by_leader_id(blocks, leaders_count, blocks[j]->last->next->line);
                assert(blocks[j]->fail != NULL);
                break;
            case irret:
                blocks[j]->branch = NULL;
                blocks[j]->fail = NULL;
                break;
            case irjmp:
                blocks[j]->fail = NULL;
                blocks[j]->branch = find_block_by_label(blocks, leaders_count, it_irc->u.label);
                break;
            case ircjmp:
            case irfjmp:
            case irtjmp:
                blocks[j]->fail = find_block_by_leader_id(blocks, leaders_count, blocks[j]->last->next->line);
                blocks[j]->branch = find_block_by_label(blocks, leaders_count, it_irc->u.label);
                break;
            default:
                blocks[j]->branch = NULL;
                blocks[j]->fail = find_block_by_leader_id(blocks, leaders_count, blocks[j]->last->next->line);
                assert(blocks[j]->fail != NULL);
                break;
        }
    }
    // DEBUG Purpose
    // for(int j = 0; j < leaders_count; j++){
    //     printf("block_id: %d firstl: %d lastl: %d branch: %d fail: %d\n", 
    //         blocks[j]->block_id,
    //         blocks[j]->first->line, blocks[j]->last->line, 
    //         blocks[j]->branch?blocks[j]->branch->block_id:0, 
            // blocks[j]->fail?blocks[j]->fail->block_id:0);
    // }
    return blocks;
}
