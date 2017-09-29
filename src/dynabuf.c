//
// Created by james on 9/28/17.
//

#include "dynabuf.h"

// dynamic buffer
// ========================================

const static size_t dynabuf_extend_space_size =
    ROUND_UP_16(DYNABUF_EXTEND_SPACE_SIZE);


static inline size_t dynabuf_extend_size(size_t old_size, size_t insert_size) {
  return old_size + (insert_size / dynabuf_extend_space_size + 1)
      * dynabuf_extend_space_size;
}

dynabuf_t dynabuf_alloc() {
  return malloc(sizeof(dynabuf_s));
}

bool dynabuf_init(dynabuf_t self, size_t size) {
  if (self == NULL) return false;
  if (size != 0) {
    // size 为 16 的倍数
    size = ROUND_UP_16(size);
    self->_buffer = malloc(size);
    if (self->_buffer == NULL) return false;
    self->_size = size;
  } else {
    self->_buffer = NULL;
    self->_size = 0;
  }
  self->_len = 0;
  return true;
}

bool dynabuf_destroy(dynabuf_t self) {
  if (self == NULL) return false;
  free(self->_buffer);
  self->_buffer = NULL;
  self->_len = self->_size = 0;
  return true;
}

char *dynabuf_write(dynabuf_t self, const char *src, size_t len) {
  if (self == NULL) return NULL;
  if (self->_len + len > self->_size) {
    size_t new_size = dynabuf_extend_size(self->_size, len);
    char *new_buffer = malloc(new_size);
    if (new_buffer == NULL) return NULL;
    free(self->_buffer);
    self->_buffer = new_buffer;
    self->_size = new_size;
  }
  char *ret = memcpy(self->_buffer + self->_len, src, len);
  self->_len += len;
  return ret;
}

char *dynabuf_write_with_eow(dynabuf_t self, const char *src, size_t len) {
  if (self == NULL) return NULL;
  if (self->_len + len + 1 > self->_size) {
    size_t new_size = dynabuf_extend_size(self->_size, len + 1);
    char *new_buffer = malloc(new_size);
    if (new_buffer == NULL) return NULL;
    free(self->_buffer);
    self->_buffer = new_buffer;
    self->_size = new_size;
  }
  char *ret = memcpy(self->_buffer + self->_len, src, len);
  self->_len += len;
  // append '\0'
  self->_buffer[self->_len] = '\0';
  self->_len++;
  return ret;
}
