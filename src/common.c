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

size_t strnlen(const char *s, size_t n) {
  size_t i;
  for (i = 0; i < n; i++) {
    if (s[i] == '\0')
      return i;
  }
  return n;
}

char *strnstr(const char *s1, const char *s2, size_t n) {
  int i;
  size_t needle_len;

  if (0 == (needle_len = strnlen(s2, n)))
    return (char *) s1;

  for (i = 0; i <= (int) (n - needle_len); i++) {
    if ((s1[0] == s2[0]) && (0 == strncmp(s1, s2, needle_len)))
      return (char *) s1;
    s1++;
  }
  return NULL;
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
