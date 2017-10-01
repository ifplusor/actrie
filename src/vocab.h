//
// Created by james on 9/30/17.
//

#ifndef _MATCH_VOCAB_H_
#define _MATCH_VOCAB_H_

#include <common.h>
#include "dynabuf.h"
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vocab {
  stream_t _stream;
  size_t count, length;

  dynabuf_s _buf;
} vocab_s, *vocab_t;

vocab_t vocab_alloc();
bool vocab_free(vocab_t self);

vocab_t vocab_construct(stream_type_e type, const char *src);
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
