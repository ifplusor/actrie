//
// Created by james on 6/16/17.
//

#include <sys/timeb.h>
#include <common.h>

const size_t POOL_REGION_SIZE = REGION_SIZE;
const size_t POOL_POSITION_SIZE = POSITION_MASK + 1;

const char *str_empty = "";

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

bool striter_init(striter_t self, unsigned char *ptr, size_t len) {
  self->ptr = ptr;
  self->len = len;
  self->cur = 0;
  return true;
}

bool striter_reset(striter_t self) {
  self->cur = 0;
  return true;
}

int striter_cur(striter_t self) {
  if (self->cur >= self->len)
    return EOF;
  return self->ptr[self->cur];
}

int striter_next(striter_t self) {
  if (self->cur >= self->len)
    return EOF;
  return self->ptr[self->cur++];
}
