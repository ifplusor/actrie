/**
 * vocab.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_VOCABULARY_H__
#define __ACTRIE_VOCABULARY_H__

#include <alib/acom.h>
#include <alib/io/stream.h>
#include <alib/string/dynabuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct vocab {
  stream_t _stream;
  size_t count, length;

  dynabuf_s _buf;
} vocab_s, *vocab_t;

vocab_t vocab_construct(stream_type_e type, void* src);
bool vocab_destruct(vocab_t self);

size_t vocab_count(vocab_t self);
size_t vocab_length(vocab_t self);

// iterator
bool vocab_reset(vocab_t self);
bool vocab_next_word(vocab_t self, strlen_t keyword, strlen_t extra);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_VOCABULARY_H__
