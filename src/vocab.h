//
// Created by james on 9/30/17.
//

#ifndef _MATCH_VOCAB_H_
#define _MATCH_VOCAB_H_

#include <common.h>
#include "dynabuf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vocab_type {
  vocab_type_file,
  vocab_type_string
} vocab_type_e;

typedef enum vocab_prop {
  vocab_prop_empty = 0,
  vocab_prop_bom = 1,
} vocab_prop_f;

typedef struct vocab {
  void *data;
  size_t count, length;
  vocab_type_e type;
  vocab_prop_f prop;

  dynabuf_s _buf;
  striter_s _cur;
} vocab_s, *vocab_t;

vocab_t vocab_alloc();
bool vocab_free(vocab_t self);

vocab_t vocab_construct(vocab_type_e type, void *src);
bool vocab_destruct(vocab_t self);

size_t vocab_count(vocab_t self);
size_t vocab_length(vocab_t self);

// iterator
bool vocab_reset(vocab_t self);
bool vocab_next_word(vocab_t self, strlen_t keyword, strlen_t extra);

#ifdef __cplusplus
};
#endif

#endif //_MATCH_VOCAB_H_
