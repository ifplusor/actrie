/**
 * vocab.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "vocab.h"

vocab_t vocab_alloc() {
  return amalloc(sizeof(vocab_s));
}

bool vocab_free(vocab_t self) {
  if (self == NULL) {
    return false;
  }
  afree(self);
  return true;
}

vocab_t vocab_construct(stream_type_e type, void* src) {
  int ch, ch0;
  vocab_t vocab = vocab_alloc();
  do {
    if (src == NULL) {
      break;
    }

    vocab->_stream = stream_construct(type, src);
    if (vocab->_stream == NULL) {
      break;
    }

    // get line number and file size
    vocab->count = vocab->length = 0;
    ch0 = '\n';
    while ((ch = stream_getc(vocab->_stream)) != EOF) {
      if (ch == '\n' && ch0 != '\n') {
        vocab->count++;
      }
      vocab->length++;
      ch0 = ch;
    }
    if (ch0 != '\n') {
      vocab->count++;
    }

    dynabuf_init(&vocab->_buf, 200);

    return vocab;
  } while (0);

  vocab_free(vocab);
  return NULL;
}

bool vocab_destruct(vocab_t self) {
  if (self == NULL) {
    return false;
  }
  stream_destruct(self->_stream);
  dynabuf_clean(&self->_buf);
  afree(self);
  return true;
}

size_t vocab_count(vocab_t self) {
  return self ? self->count : 0;
}

size_t vocab_length(vocab_t self) {
  return self ? self->length : 0;
}

// iterator
bool vocab_reset(vocab_t self) {
  if (self == NULL) {
    return false;
  }
  stream_rewind(self->_stream);
  return true;
}

bool vocab_next_word(vocab_t self, strlen_t keyword, strlen_t extra) {
  strpos_s keyword_pos, extra_pos;
  int ch;
  if (self == NULL) {
    return false;
  }
  dynabuf_reset(&self->_buf);
  ch = dynabuf_consume_until(&self->_buf, self->_stream, "\n\t", &keyword_pos);
  if (ch == EOF && dynabuf_empty(&self->_buf)) {
    return false;
  } else {
    dynabuf_write_eos(&self->_buf, NULL, 0);
    *keyword = dynabuf_split(&self->_buf, keyword_pos);
    if (ch == '\t') {
      ch = dynabuf_consume_until(&self->_buf, self->_stream, "\n", &extra_pos);
      dynabuf_write_eos(&self->_buf, NULL, 0);
      *extra = dynabuf_split(&self->_buf, extra_pos);
      if (extra->len > 0 && extra->ptr[extra->len - 1] == '\r') {  // EOL is '\r\n'
        extra->len -= 1;
        extra->ptr[extra->len] = '\0';
      }
    } else {
      if (keyword->len > 0 && keyword->ptr[keyword->len - 1] == '\r') {  // EOL is '\r\n'
        keyword->len -= 1;
        keyword->ptr[keyword->len] = '\0';
      }
      *extra = strlen_empty;
    }
  }
  return true;
}
