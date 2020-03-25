/**
 * lr_reduce.h - LR(1) analyzer reduce functions
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef _LR_ANALYZER_REDUCE_H_
#define _LR_ANALYZER_REDUCE_H_

#include <alib/memory/dynapool.h>

typedef struct _lr_sign {
  int state;
  aobj data;
  deque_node_s deque_elem;
} lr_sign_s, *lr_sign_t;

typedef void (*lr_reduce_func)(dynapool_t, deque_node_t, lr_sign_t*);

void reduce_only_pop(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_text2pure(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_unwrap(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_ambi(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_anto(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_join(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_dist(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);
void reduce_alter(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node);

#endif  //_LR_ANALYZER_REDUCE_H_
