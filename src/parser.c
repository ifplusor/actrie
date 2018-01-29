//
// Created by james on 1/16/18.
//

#include <obj/pint.h>
#include "parser.h"
#include "lr_table.h"


void reduce_only_pop(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  *node = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
}

void reduce_text2pure(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  lr_sign_t sign = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct pure-pattern
  dstr_t text = sign->data;
  sign->data = _alloc(ptrn, pure, text);
  _release(text);

  *node = sign;
}

void reduce_unwrap(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  lr_sign_t right = deque_pop_front(sign_stack, lr_sign_s, deque_elem); // ')'
  lr_sign_t origin = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t left = deque_pop_front(sign_stack, lr_sign_s, deque_elem); // '(', '(?&!', '(?<!'

  dynapool_free_node(sign_pool, right);
  dynapool_free_node(sign_pool, left);

  *node = origin;
}

void reduce_ambi(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  lr_sign_t anti = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t origin = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct ambi-pattern
  origin->data = _alloc(ptrn, ambi, origin->data, anti->data);

  dynapool_free_node(sign_pool, origin);

  *node = origin;
}

void reduce_anto(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  lr_sign_t origin = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t anti = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct anto-pattern
  anti->data = _alloc(ptrn, anto, origin->data, anti->data);

  dynapool_free_node(sign_pool, origin);

  *node = anti;
}

void reduce_dist(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  lr_sign_t tail = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t dist = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t head = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  uptr_t data = (uptr_t) pint_get(dist->data);
  int min = (int) (data & 0xFFFF);
  int max = (int) ((data >> 16) & 0xFFFF);

  // construct dist-pattern
  head->data = _alloc(ptrn, dist, head->data, tail->data, min, max);

  dynapool_free_node(sign_pool, tail);
  dynapool_free_node(sign_pool, dist);

  *node = head;
}

void reduce_alter(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node) {
  lr_sign_t after = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t alter = deque_pop_front(sign_stack, lr_sign_s, deque_elem); // "|"
  lr_sign_t before = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct anto-pattern
  before->data = _alloc(ptrn, cat, before->data, after->data);

  dynapool_free_node(sign_pool, after);
  dynapool_free_node(sign_pool, alter);

  *node = before;
}

/**
 * parse_reduce - reduce production
 * @param sign_pool [IN]
 * @param sign_stack [IN]
 * @param node [OUT]
 * @param change [IO]
 */
void parse_reduce(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t *node, lr_item_t change) {
  int pdct = change->index;
  // reduce
  lr_reduce_func_table[pdct](sign_pool, sign_stack, node);
  int nonid = lr_pdct2nonid[pdct];
  lr_sign_t sign = deque_peek_front(sign_stack, lr_sign_s, deque_elem);
  // after reduce, check goto table
  *change = lr_goto_table[sign->state][nonid];
}

ptrn_t parse_pattern0(stream_t stream) {
  ptrn_t pattern = NULL;

  deque_node_s sentinel0, sentinel1;
  deque_node_t sign_stack = &sentinel0;
  deque_node_t token_deque = &sentinel1;
  deque_init(sign_stack);
  deque_init(token_deque);

  dynapool_t sign_pool = dynapool_construct_with_type(lr_sign_s);

  // tokenize
  int ch;
  dstr_t token;
  while ((ch = token_next(stream, &token)) != TOKEN_EOF) {
    if (ch == TOKEN_ERR) break;

    if (token != NULL) {
      lr_sign_t node = dynapool_alloc_node(sign_pool);
      node->state = TOKEN_TEXT;
      node->data = token;
      deque_push_back(token_deque, node, lr_sign_s, deque_elem);
    }

    if (ch != TOKEN_TEXT){
      lr_sign_t node = dynapool_alloc_node(sign_pool);
      node->state = -ch;
      if (ch == TOKEN_DIST) {
        int max = token_max_dist();
        int min = token_min_dist();
        node->data = pint(((uptr_t) max & 0xFFFF) << 16 | ((uptr_t) min & 0xFFFF));
      } else {
        node->data = NULL;
      }
      deque_push_back(token_deque, node, lr_sign_s, deque_elem);
    }
  }

  if (ch == TOKEN_ERR) {
    // tokenize error
    goto lr_err;
  }

  // TOKEN_EOF 入队
  lr_sign_t node = dynapool_alloc_node(sign_pool);
  node->state = -TOKEN_EOF;
  node->data = NULL;
  deque_push_back(token_deque, node, lr_sign_s, deque_elem);

  // 初始状态进栈
  lr_sign_t sign = dynapool_alloc_node(sign_pool);
  sign->state = 0;
  sign->data = NULL;
  deque_push_front(sign_stack, sign, lr_sign_s, deque_elem);

  while (!deque_empty(token_deque)) {
    node = deque_pop_front(token_deque, lr_sign_s, deque_elem);
    sign = deque_peek_front(sign_stack, lr_sign_s, deque_elem);
    // check action table
    lr_item_s change = lr_action_table[sign->state][node->state];
lr_rdc:
    switch (change.action) {
      case deny:goto lr_err;
      case acpt:

        goto lr_acc;
      case shft:
        // node 进栈，无需释放内存
        node->state = change.index;
        deque_push_front(sign_stack, node, lr_sign_s, deque_elem);
        break;
      case rduc:
        // node 放回 token_deque，规约产生式
        deque_push_front(token_deque, node, lr_sign_s, deque_elem);
        parse_reduce(sign_pool, sign_stack, &node, &change);
        goto lr_rdc;
    }
  }

lr_acc:
  sign = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  pattern = sign->data;

lr_err:
  while (!deque_empty(token_deque)) {
    node = deque_pop_front(token_deque, lr_sign_s, deque_elem);
    _release(node->data);
  }
  while (!deque_empty(sign_stack)) {
    sign = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
    _release(sign->data);
  }
  dynapool_destruct(sign_pool);

  return pattern;
}

/**
 * wrapper for convert strlen_t to stream_t
 */
ptrn_t parse_pattern(strlen_t pattern) {
  stream_t stream = stream_construct(stream_type_string, pattern);
  ptrn_t ptrn = parse_pattern0(stream);
  stream_destruct(stream);
  return ptrn;
}

void *parse_vocab(vocab_t vocab) {
  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    // TODO: parse pattern, output syntax tree
    ptrn_t syntax = parse_pattern(&keyword);

    _release(syntax);
  }

  return NULL;
}
