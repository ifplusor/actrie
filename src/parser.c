//
// Created by james on 1/16/18.
//

#include "tokenizer.h"
#include "parser.h"
#include <dynapool.h>

#include "lr_table.h"


typedef struct _sign_node {
  int state;
  void *data;
  deque_node_s deque_elem;
} sign_s, *sign_t;


typedef void (*lr_reduce_func)(deque_node_t, sign_t*);

void reduce_only_pop(deque_node_t sign_stack, sign_t *node) {
  *node = deque_pop_front(sign_stack, sign_s, deque_elem);
}

static const lr_reduce_func reduce_func[LR_PDCT_NUM] = {
    NULL,
    reduce_only_pop,

};

void parse_reduce(lr_item_t change, deque_node_t sign_stack, sign_t *node) {
  int pdct = change->index;
  // reduce
  reduce_func[pdct](sign_stack, node);
  int nonid = lr_pdct2nonid[pdct];
  sign_t sign = deque_peek_front(sign_stack, sign_s, deque_elem);
  // after reduce, check goto table
  *change = lr_goto_table[sign->state][nonid];
}

ptrn_t parse_pattern1(stream_t stream) {
  deque_node_s sentinel0, sentinel1;
  deque_node_t sign_stack = &sentinel0;
  deque_node_t token_deque = &sentinel1;
  deque_init(sign_stack);

  dynapool_t sign_pool = dynapool_construct_with_type(sign_s);

  // tokenize
  int ch;
  dstr_t token;
  while ((ch = token_next(stream, &token)) != TOKEN_EOF) {
    if (ch == TOKEN_ERR) break;

    sign_t node = dynapool_alloc_node(sign_pool);
    node->state = -ch;
    if (ch == TOKEN_TEXT) {
      node->data = token;
    } else if (ch == TOKEN_DIST) {
      int max = token_max_dist();
      int min = token_min_dist();
      node->data = (void*) (((uptr_t) max & 0xFFFF) << 16 | ((uptr_t) min & 0xFFFF));
    } else {
      node->data = NULL;
    }
    deque_push_back(token_deque, node, sign_s, deque_elem);
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
          node->state = change.action;
          deque_push_front(sign_stack, node, sign_s, deque_elem);
          break;
        case rduc:
          parse_reduce(&change, sign_stack, &node);
          goto lr_rdc;
      }
    }
  }

lr_acc:
  return NULL;

lr_err:
  return NULL;
}


ptrn_t parse_pattern0(stream_t stream, bool expect_sube);

ptrn_t parse_alter(stream_t stream, ptrn_t before, bool expect_sube) {
  ptrn_t after = parse_pattern0(stream, expect_sube);
  if (after != NULL)
    return _alloc(ptrn, cat, before, after);
  // error
  return NULL;
}

ptrn_t parse_ambi(stream_t stream, ptrn_t origin, bool expect_sube) {
  // get ambiguous pattern
  ptrn_t ambi = parse_pattern0(stream, true);
  if (ambi == NULL) return NULL;

  // check ambi type
  do {
    // only pure pattern is allowed as antonym prefix
    if (ambi->type == ptrn_type_pure) break; // type is pure, pass
    if (ambi->type == ptrn_type_alter) {
      list_t con = ambi->desc;
      while (con != NULL) {
        ptrn_t p = con->car;
        if (p->type != ptrn_type_pure)
          break;
        con = con->cdr;
      }
      if (con == NULL) break; // type of each item is pure, pass
    }
    // error
    _release(ambi);
    return NULL;
  } while (0);

  // create ambi-pattern
  return _alloc(ptrn, ambi, origin, ambi);
}

ptrn_t parse_anto(stream_t stream, bool expect_sube) {
  // get antonym pattern
  ptrn_t anto = parse_pattern0(stream, true);
  if (anto == NULL) return NULL;

  // check anto type
  do {
    // only pure pattern is allowed as antonym prefix
    if (anto->type == ptrn_type_pure) break; // type is pure, pass
    if (anto->type == ptrn_type_alter) {
      list_t con = anto->desc;
      while (con != NULL) {
        ptrn_t p = con->car;
        if (p->type != ptrn_type_pure)
          break;
        con = con->cdr;
      }
      if (con == NULL) break; // type of each item is pure, pass
    }
    // error
    _release(anto);
    return NULL;
  } while (0);

  // get origin pattern
  ptrn_t origin = parse_pattern0(stream, expect_sube);
  if (origin == NULL) {
    _release(anto);
    return NULL;
  }

  // create anto-pattern
  return _alloc(ptrn, anto, origin, anto);
}

ptrn_t parse_dist(stream_t stream, ptrn_t head, bool expect_sube) {
  int min = token_min_dist();
  int max = token_max_dist();

  ptrn_t tail = parse_pattern0(stream, expect_sube);
  if (tail != NULL)
    return _alloc(ptrn, dist, head, tail, min, max);

  return NULL;
}

/**
 * @param stream
 * @param expect_sube - terminator
 * @return pattern
 */
ptrn_t parse_pattern0(stream_t stream, bool expect_sube) {
  int ch;
  dstr_t token;
  ptrn_t pattern = NULL;

  while ((ch = token_next(stream, &token)) != TOKEN_EOF) {


    if (token != NULL) {
      // have token, generate pure pattern
      ptrn_t ptrn = _alloc(ptrn, pure, token);
      _release(token);

      if (ch == TOKEN_TEXT) { // pure pattern
        pattern = ptrn;
      } else if (ch == TOKEN_SUBE) {
        // empty-sube
        pattern = ptrn;
        break; // sub pattern is end
      } else if (ch == TOKEN_AMBI) {
        // empty-ambi
        pattern = parse_ambi(stream, ptrn, expect_sube);
      } else if (ch == TOKEN_DIST) {
        // empty-dist
        pattern = parse_dist(stream, ptrn, expect_sube);
      } else if (ch == TOKEN_ALT) {
        // empty-alt
        pattern = parse_alter(stream, ptrn, expect_sube);
      } else {
        ALOG_FATAL("parser error.");
      }
    } else {
      // have not token
      if (ch == TOKEN_SUBS) {
        // empty-subs
        pattern = parse_pattern0(stream, true);
      } else if (ch == TOKEN_SUBE) {

      } else if (ch == TOKEN_AMBI) {

      } else if (ch == TOKEN_ANTO) {
        // only empty-anto is allowed, so pattern must be empty in here
        pattern = parse_anto(stream, expect_sube);
      } else if (ch == TOKEN_DIST) {
        // pattern-dist
        pattern = parse_dist(stream, pattern, expect_sube);
      } else if (ch == TOKEN_ALT) {
        pattern = parse_alter(stream, pattern, expect_sube);
      } else {
        ALOG_FATAL("parser error.");
      }
    }
  }

  if (((ch == TOKEN_SUBE) && !expect_sube) ||
      ((ch == TOKEN_EOF) && expect_sube)) {
    // terminator is unexpected
    _release(pattern);
    return NULL;
  }

  return pattern;
}

/**
 * wrapper for convert strlen_t to stream_t
 */
ptrn_t parse_pattern(strlen_t pattern) {
  stream_t stream = stream_construct(stream_type_string, pattern);
  ptrn_t ptrn = parse_pattern0(stream, false);
  stream_destruct(stream);
  return ptrn;
}

void *parse_vocab(vocab_t vocab) {
  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    // TODO: parse pattern, output syntax tree
    ptrn_t syntax = parse_pattern(&keyword);

  }

  return NULL;
}
