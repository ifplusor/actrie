//
// Created by james on 9/30/17.
//

#include "vocab.h"

vocab_t vocab_alloc() {
  return malloc(sizeof(vocab_s));
}

bool vocab_free(vocab_t self) {
  if (self == NULL) return false;
  free(self);
  return true;
}

vocab_t vocab_construct(vocab_type_e type, void *src) {
  size_t i;
  int ch, ch0;
  vocab_t vocab = vocab_alloc();
  do {
    if (src == NULL) break;

    vocab->count = vocab->length = 0;
    vocab->type = type;
    vocab->prop = vocab_prop_empty;

    dynabuf_init(&vocab->_buf, 200, false);

    if (type == vocab_type_file) {
      FILE *fp = fopen(src, "rb");
      if (fp == NULL) break;

      vocab->data = fp;

      // filter bom
      ch;
      if ((ch = getc(fp)) != 0xef) {
        if (ch != EOF) ungetc(ch, fp);
      } else if ((ch = getc(fp)) != 0xbb) {
        if (ch != EOF) ungetc(ch, fp);
        ungetc(0xef, fp);
      } else if ((ch = getc(fp)) != 0xbf) {
        if (ch != EOF) ungetc(ch, fp);
        ungetc(0xbb, fp);
        ungetc(0xef, fp);
      } else {
        vocab->prop |= vocab_prop_bom;
      }

      // get line number and file size
      ch0 = '\n';
      while ((ch = getc(fp)) != EOF) {
        if (ch == '\n' && ch0 != '\n')
          vocab->count++;
        vocab->length++;
        ch0 = ch;
      }
      if (ch0 != '\n')
        vocab->count++;
    } else if (type == vocab_type_string) {
      char *str = src;
      vocab->length = strlen(str);
      vocab->data = dynabuf_alloc();
      dynabuf_init(vocab->data, vocab->length + 1, false);

      if (vocab->length >= 3
          && str[0] == 0xef && str[1] == 0xbb && str[2] == 0xbf) {
        vocab->length -= 3;
        dynabuf_write_with_zero(vocab->data, str+3, vocab->length);
      } else {
        dynabuf_write_with_zero(vocab->data, str, vocab->length);
      }
      striter_init(&vocab->_cur, vocab->data, vocab->length);

      ch0 = '\n';
      for (i = 0; i < vocab->length; i++) {
        ch = str[i];
        if (ch == '\n' && ch0 != '\n')
          vocab->count++;
        ch0 = ch;
      }
      if (ch0 != '\n')
        vocab->count++;
    } else {
      break;
    }
    return vocab;
  } while(0);

  vocab_free(vocab);
  return NULL;
}

bool vocab_destruct(vocab_t self) {
  if (self == NULL) return false;
  if (self->type == vocab_type_file) {
    fclose(self->data);
  } else if (self->type == vocab_type_string) {
    dynabuf_free(self->data);
  }
  dynabuf_clean(&self->_buf);
  free(self);
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
  if (self == NULL) return false;
  if (self->type == vocab_type_file) {
    if (self->prop & vocab_prop_bom) {
      fseek(self->data, 3, SEEK_SET);
    } else {
      fseek(self->data, 0, SEEK_SET);
    }
  } else if (self->type == vocab_type_string) {
    striter_init(&self->_cur, self->data, self->length);
  }
  return true;
}

bool vocab_next_word(vocab_t self, strlen_t keyword, strlen_t extra) {
  size_t keyword_pos, extra_pos;
  char ch;
  if (self == NULL) return false;
  dynabuf_reset(&self->_buf);
  if (self->type == vocab_type_file) {
    keyword_pos = dynabuf_length(&self->_buf);
    ch = dynabuf_read_file_until(&self->_buf, self->data, "\n\t");
    if (ch == EOF && self->_buf._len == 0) {
      return false;
    } else if (ch == '\n' || ch == EOF) {
      keyword->len = dynabuf_length(&self->_buf);
      dynabuf_write_with_zero(&self->_buf, NULL, 0);
      keyword->ptr = dynabuf_buffer(&self->_buf, keyword_pos);
      extra->ptr = (char *) str_empty;
      extra->len = 0;
    } else {
      keyword->len = dynabuf_length(&self->_buf);
      dynabuf_write_with_zero(&self->_buf, NULL, 0);
      extra_pos = dynabuf_length(&self->_buf);
      dynabuf_read_file_until(&self->_buf, self->data, "\n");
      extra->len = dynabuf_length(&self->_buf) - keyword->len - 1;
      dynabuf_write_with_zero(&self->_buf, NULL, 0);
      keyword->ptr = dynabuf_buffer(&self->_buf, keyword_pos);
      extra->ptr = dynabuf_buffer(&self->_buf, extra_pos);
    }
  } else if (self->type == vocab_type_string) {
    // TODO:
    ;
  }
  return true;
}