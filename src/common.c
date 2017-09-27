//
// Created by james on 6/16/17.
//

#include <sys/timeb.h>
#include <common.h>

const size_t POOL_REGION_SIZE = REGION_SIZE;
const size_t POOL_POSITION_SIZE = POSITION_MASK + 1;

long long system_millisecond() {
  struct timeb t;
  ftime(&t);
  return t.time * 1000 + t.millitm;
}

char *strdup(const char *s) {
  char *dup = (char *) malloc(sizeof(char) * (strlen(s) + 1));
  if (dup != NULL)
    strcpy(dup, s);
  return dup;
}

dynabuf_t dynabuf_alloc() {
  return malloc(sizeof(dynabuf_s));
}

bool dynabuf_init(dynabuf_t buf, size_t size) {
  if (buf == NULL) return false;
  if (size != 0) {
    // size 为 16 的倍数
    size = (size & 0x000FLL) ? (size | 0x000FLL) + 1 : size;
    buf->buffer = malloc(size);
    if (buf->buffer == NULL) return false;
    buf->size = size;
  } else {
    buf->buffer = NULL;
    buf->size = 0;
  }
  buf->len = 0;
  return true;
}

bool dynabuf_destroy(dynabuf_t buf) {
  if (buf == NULL) return false;
  free(buf->buffer);
  buf->buffer = NULL;
  buf->len = buf->size = 0;
  return true;
}

const size_t extend_space_size = (EXTEND_SPACE_SIZE & 0x000FLL)
                                 ? (EXTEND_SPACE_SIZE | 0x000FLL) + 1
                                 : EXTEND_SPACE_SIZE;

char *dynabuf_write(dynabuf_t buf, const char *src, size_t len) {
  if (buf == NULL) return NULL;
  if (buf->len + len > buf->size) {
    size_t new_size = buf->size
        + (len / extend_space_size + 1) * extend_space_size;
    char *new_buffer = malloc(new_size);
    if (new_buffer == NULL) return NULL;
    free(buf->buffer);
    buf->buffer = new_buffer;
    buf->size = new_size;
  }
  char *ret = memcpy(buf->buffer + buf->len, src, len);
  buf->len += len;
  return ret;
}

char *dynabuf_write_with_eow(dynabuf_t buf, const char *src, size_t len) {
  if (buf == NULL) return NULL;
  if (buf->len + len + 1 > buf->size) {
    size_t new_size = buf->size
        + ((len + 1) / extend_space_size + 1) * extend_space_size;
    char *new_buffer = malloc(new_size);
    if (new_buffer == NULL) return NULL;
    free(buf->buffer);
    buf->buffer = new_buffer;
    buf->size = new_size;
  }
  char *ret = memcpy(buf->buffer + buf->len, src, len);
  buf->len += len;
  // append '\0'
  buf->buffer[buf->len] = '\0';
  buf->len++;
  return ret;
}


