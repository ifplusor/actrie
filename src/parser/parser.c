/**
 * parser.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "parser.h"

#include <alib/object/pint.h>

#include "lr_table.h"
#include "tokenizer.h"

void reduce_only_pop(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  *node = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
}

void reduce_text2pure(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t sign = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct pure-pattern
  dstr_t text = sign->data;
  ptrn_t pure_ptrn = _alloc(ptrn, pure, text);
  _release(text);

  // reuse sign
  sign->data = pure_ptrn;
  *node = sign;
}

void reduce_unwrap(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t right = deque_pop_front(sign_stack, lr_sign_s, deque_elem);  // ')'
  lr_sign_t center = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t left = deque_pop_front(sign_stack, lr_sign_s, deque_elem);  // '(', '(?&!', '(?<!'

  // reuse center
  *node = center;

  // clean other
  _release(right->data);
  dynapool_free_node(sign_pool, right);
  _release(left->data);
  dynapool_free_node(sign_pool, left);
}

void reduce_ambi(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t ambi = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t center = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct ambi-pattern
  ptrn_t ambi_ptrn = _alloc(ptrn, ambi, center->data, ambi->data);

  // reuse center
  center->data = ambi_ptrn;
  *node = center;

  // clean other
  dynapool_free_node(sign_pool, ambi);
}

void reduce_anto(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t center = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t anto = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct anto-pattern
  ptrn_t anto_ptrn = _alloc(ptrn, anto, center->data, anto->data);

  // reuse center
  center->data = anto_ptrn;
  *node = center;

  // clean other
  dynapool_free_node(sign_pool, anto);
}

void reduce_join(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t tail = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t head = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct dist-pattern
  ptrn_t dist_ptrn = _alloc(ptrn, dist, head->data, tail->data, ptrn_dist_type_any, 0, 0);

  // reuse head
  head->data = dist_ptrn;
  *node = head;

  // clean other
  dynapool_free_node(sign_pool, tail);
}

void reduce_dist(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t tail = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t rept = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t char_set = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t head = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  uptr_t rept_data = (uptr_t)pint_get(rept->data);
  int min = (int)(rept_data & 0xFFFFU);
  int max = (int)((rept_data >> 16U) & 0xFFFFU);

  // construct dist-pattern
  ptrn_t dist_ptrn;
  int charset = (int)pint_get(char_set->data);
  if (charset == TOKEN_NUM) {
    dist_ptrn = _alloc(ptrn, dist, head->data, tail->data, ptrn_dist_type_num, min, max);
  } else {
    dist_ptrn = _alloc(ptrn, dist, head->data, tail->data, ptrn_dist_type_any, min, max);
  }

  // reuse head
  head->data = dist_ptrn;
  *node = head;

  // clean other
  dynapool_free_node(sign_pool, tail);
  _release(rept->data);
  dynapool_free_node(sign_pool, rept);
  _release(char_set->data);
  dynapool_free_node(sign_pool, char_set);
}

void reduce_alter(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node) {
  lr_sign_t after = deque_pop_front(sign_stack, lr_sign_s, deque_elem);
  lr_sign_t alter = deque_pop_front(sign_stack, lr_sign_s, deque_elem);  // "|"
  lr_sign_t before = deque_pop_front(sign_stack, lr_sign_s, deque_elem);

  // construct alter-pattern
  ptrn_t alter_ptrn = _alloc(ptrn, cat, before->data, after->data);

  // reuse before
  before->data = alter_ptrn;
  *node = before;

  // clean other
  dynapool_free_node(sign_pool, after);
  _release(alter->data);
  dynapool_free_node(sign_pool, alter);
}

/**
 * parse_reduce - reduce production
 * @param sign_pool [IN]
 * @param sign_stack [IN]
 * @param node [OUT]
 * @param change [IO]
 */
void parse_reduce(dynapool_t sign_pool, deque_node_t sign_stack, lr_sign_t* node, lr_item_t change) {
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

  deque_node_s sign_stack[1], token_deque[1];
  deque_init(sign_stack);
  deque_init(token_deque);

  dynapool_t sign_pool = dynapool_construct_with_type(lr_sign_s);

  // tokenize
  int ch;
  dstr_t token;
  while ((ch = token_next(stream, &token)) != TOKEN_EOF) {
    if (ch == TOKEN_ERR) {
      break;
    }

    if (token != NULL) {
      lr_sign_t node = dynapool_alloc_node(sign_pool);
      node->state = TOKEN_TEXT;
      node->data = token;
      deque_push_back(token_deque, node, lr_sign_s, deque_elem);
    }

    if (ch != TOKEN_TEXT) {  // sign
      lr_sign_t node = dynapool_alloc_node(sign_pool);
      node->state = -ch;  // every sign is negative
      if (ch == TOKEN_REPT) {
        int max = token_rept_max();
        int min = token_rept_min();
        node->data = pint(((uptr_t)max & 0xFFFFU) << 16U | ((uptr_t)min & 0xFFFFU));
      } else {
        node->data = pint(ch);
      }
      deque_push_back(token_deque, node, lr_sign_s, deque_elem);
    }
  }

  if (ch == TOKEN_ERR) {
    // tokenize error
    goto lr_error;
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
  lr_reduce:
    switch (change.action) {
      case deny:
        // node 放回 token_deque，等待后续清理
        deque_push_front(token_deque, node, lr_sign_s, deque_elem);
        goto lr_error;
      case acpt:
        // node 是 TOKEN_EOF, 放回 token_deque，等待后续清理
        deque_push_front(token_deque, node, lr_sign_s, deque_elem);
        goto lr_accept;
      case shft:
        // node 进栈，无需释放内存
        node->state = change.index;
        deque_push_front(sign_stack, node, lr_sign_s, deque_elem);
        break;
      case rduc:
        // node 放回 token_deque，规约产生式
        deque_push_front(token_deque, node, lr_sign_s, deque_elem);
        parse_reduce(sign_pool, sign_stack, &node, &change);
        goto lr_reduce;
    }
  }

lr_accept:
  sign = deque_peek_front(sign_stack, lr_sign_s, deque_elem);
  pattern = sign->data;
  _retain(pattern);

lr_error:
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

bool parse_vocab(vocab_t vocab,
                 have_pattern_f have_pattern,
                 void* arg,
                 bool all_as_plain,
                 bool ignore_bad_pattern,
                 bool bad_as_plain) {
  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    if (keyword.len <= 0) {
      continue;
    }
    // parse pattern, output syntax tree
    ptrn_t pattern;
    if (all_as_plain) {
      // construct pure-pattern
      dstr_t text = dstr(&keyword);
      pattern = _alloc(ptrn, pure, text);
      _release(text);
    } else {
      pattern = parse_pattern(&keyword);
      if (pattern == NULL) {
        fprintf(stderr, "bad pattern: '%.*s'\n", (int)keyword.len, keyword.ptr);
        if (!ignore_bad_pattern) {
          if (bad_as_plain) {
            // construct pure-pattern
            dstr_t text = dstr(&keyword);
            pattern = _alloc(ptrn, pure, text);
            _release(text);
          } else {
            return false;
          }
        } else {
          continue;
        }
      }
    }
    have_pattern(pattern, &extra, arg);
    _release(pattern);
  }
  return true;
}
