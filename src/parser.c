//
// Created by james on 1/16/18.
//

#include "tokenizer.h"
#include "parser.h"
#include <dynapool.h>
#include <obj/pint.h>

#include "lr_table.h"


typedef struct _sign_node {
  int state;
  aobj data;
  deque_node_s deque_elem;
} sign_s, *sign_t;


typedef void (*lr_reduce_func)(dynapool_t, deque_node_t, sign_t*);

void reduce_only_pop(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  *node = deque_pop_front(sign_stack, sign_s, deque_elem);
}

void reduce_text2pure(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  sign_t sign = deque_pop_front(sign_stack, sign_s, deque_elem);

  // construct pure-pattern
  dstr_t text = sign->data;
  sign->data = _alloc(ptrn, pure, text);
  _release(text);

  *node = sign;
}

void reduce_unwrap(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  sign_t right = deque_pop_front(sign_stack, sign_s, deque_elem); // ')'
  sign_t origin = deque_pop_front(sign_stack, sign_s, deque_elem);
  sign_t left = deque_pop_front(sign_stack, sign_s, deque_elem); // '(', '(?&!', '(?<!'

  dynapool_free_node(sign_pool, right);
  dynapool_free_node(sign_pool, left);

  *node = origin;
}

void reduce_ambi(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  sign_t anti = deque_pop_front(sign_stack, sign_s, deque_elem);
  sign_t origin = deque_pop_front(sign_stack, sign_s, deque_elem);

  // construct ambi-pattern
  origin->data = _alloc(ptrn, ambi, origin->data, anti->data);

  dynapool_free_node(sign_pool, origin);

  *node = origin;
}

void reduce_anto(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  sign_t origin = deque_pop_front(sign_stack, sign_s, deque_elem);
  sign_t anti = deque_pop_front(sign_stack, sign_s, deque_elem);

  // construct anto-pattern
  anti->data = _alloc(ptrn, anto, origin->data, anti->data);

  dynapool_free_node(sign_pool, origin);

  *node = anti;
}

void reduce_dist(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  sign_t tail = deque_pop_front(sign_stack, sign_s, deque_elem);
  sign_t dist = deque_pop_front(sign_stack, sign_s, deque_elem);
  sign_t head = deque_pop_front(sign_stack, sign_s, deque_elem);

  uptr_t data = (uptr_t) pint_get(dist->data);
  int min = (int) (data & 0xFFFF);
  int max = (int) ((data >> 16) & 0xFFFF);

  // construct dist-pattern
  head->data = _alloc(ptrn, dist, head->data, tail->data, min, max);

  dynapool_free_node(sign_pool, tail);
  dynapool_free_node(sign_pool, dist);

  *node = head;
}

void reduce_alter(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node) {
  sign_t after = deque_pop_front(sign_stack, sign_s, deque_elem);
  sign_t alter = deque_pop_front(sign_stack, sign_s, deque_elem); // "|"
  sign_t before = deque_pop_front(sign_stack, sign_s, deque_elem);

  // construct anto-pattern
  before->data = _alloc(ptrn, cat, before->data, after->data);

  dynapool_free_node(sign_pool, after);
  dynapool_free_node(sign_pool, alter);

  *node = before;
}

static const lr_reduce_func reduce_func[LR_PDCT_NUM] = {
    [0] = NULL,

    [1] = reduce_only_pop,
    [2] = reduce_only_pop,
    [6] = reduce_only_pop,
    [7] = reduce_only_pop,
    [8] = reduce_only_pop,
    [16] = reduce_only_pop,
    [17] = reduce_only_pop,
    [25] = reduce_only_pop,
    [26] = reduce_only_pop,

    [34] = reduce_text2pure,

    [4] = reduce_unwrap,
    [13] = reduce_unwrap,
    [20] = reduce_unwrap,
    [21] = reduce_unwrap,
    [22] = reduce_unwrap,
    [29] = reduce_unwrap,
    [30] = reduce_unwrap,
    [31] = reduce_unwrap,

    [27] = reduce_ambi,
    [28] = reduce_ambi,

    [3] = reduce_anto,
    [18] = reduce_anto,
    [19] = reduce_anto,

    [9] = reduce_dist,
    [10] = reduce_dist,
    [11] = reduce_dist,
    [12] = reduce_dist,

    [5] = reduce_alter,
    [14] = reduce_alter,
    [15] = reduce_alter,
    [23] = reduce_alter,
    [24] = reduce_alter,
    [32] = reduce_alter,
    [33] = reduce_alter,

};

/**
 * parse_reduce - reduce production
 * @param sign_pool [IN]
 * @param sign_stack [IN]
 * @param node [OUT]
 * @param change [IO]
 */
void parse_reduce(dynapool_t sign_pool, deque_node_t sign_stack, sign_t *node, lr_item_t change) {
  int pdct = change->index;
  // reduce
  reduce_func[pdct](sign_pool, sign_stack, node);
  int nonid = lr_pdct2nonid[pdct];
  sign_t sign = deque_peek_front(sign_stack, sign_s, deque_elem);
  // after reduce, check goto table
  *change = lr_goto_table[sign->state][nonid];
}

ptrn_t parse_pattern0(stream_t stream) {
  deque_node_s sentinel0, sentinel1;
  deque_node_t sign_stack = &sentinel0;
  deque_node_t token_deque = &sentinel1;
  deque_init(sign_stack);
  deque_init(token_deque);

  dynapool_t sign_pool = dynapool_construct_with_type(sign_s);

  // tokenize
  int ch;
  dstr_t token;
  while ((ch = token_next(stream, &token)) != TOKEN_EOF) {
    if (ch == TOKEN_ERR) break;

    if (token != NULL) {
      sign_t node = dynapool_alloc_node(sign_pool);
      node->state = TOKEN_TEXT;
      node->data = token;
      deque_push_back(token_deque, node, sign_s, deque_elem);
    }

    if (ch != TOKEN_TEXT){
      sign_t node = dynapool_alloc_node(sign_pool);
      node->state = -ch;
      if (ch == TOKEN_DIST) {
        int max = token_max_dist();
        int min = token_min_dist();
        node->data = pint(((uptr_t) max & 0xFFFF) << 16 | ((uptr_t) min & 0xFFFF));
      } else {
        node->data = NULL;
      }
      deque_push_back(token_deque, node, sign_s, deque_elem);
    }
  }

  if (ch != TOKEN_ERR) {
    // TOKEN_EOF 入队
    sign_t node = dynapool_alloc_node(sign_pool);
    node->state = TOKEN_EOF;
    node->data = NULL;
    deque_push_back(token_deque, node, sign_s, deque_elem);

    // 初始状态进栈
    sign_t sign = dynapool_alloc_node(sign_pool);
    sign->state = 0;
    sign->data = NULL;
    deque_push_front(sign_stack, sign, sign_s, deque_elem);

    while (!deque_empty(token_deque)) {
      node = deque_pop_front(token_deque, sign_s, deque_elem);
      sign = deque_peek_front(sign_stack, sign_s, deque_elem);
      // check action table
      lr_item_s change = lr_action_table[sign->state][node->state];
lr_rdc:
      switch (change.action) {
        case deny:
          goto lr_err;
        case acpt:
          goto lr_acc;
        case shft:
          // node 进栈，无需释放内存
          node->state = change.index;
          deque_push_front(sign_stack, node, sign_s, deque_elem);
          break;
        case rduc:
          // node 放回 token_deque，规约产生式
          deque_push_front(token_deque, node, sign_s, deque_elem);
          parse_reduce(sign_pool, sign_stack, &node, &change);
          goto lr_rdc;
      }
    }
  } else {
    // tokenize error
    return NULL;
  }

lr_acc:
  return NULL;

lr_err:
  return NULL;
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
