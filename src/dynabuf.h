//
// Created by james on 9/28/17.
//

#ifndef _MATCH_DYNABUF_H_
#define _MATCH_DYNABUF_H_

#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !DYNABUF_EXTEND_SPACE_SIZE
#define DYNABUF_EXTEND_SPACE_SIZE 1024
#endif

typedef struct dynamic_buffer {
  char *_buffer;
  size_t _len, _size;
} dynabuf_s, *dynabuf_t;

dynabuf_t dynabuf_alloc();
bool dynabuf_init(dynabuf_t self, size_t size);
bool dynabuf_destroy(dynabuf_t self);
char *dynabuf_write(dynabuf_t self, const char *src, size_t len);
char *dynabuf_write_with_eow(dynabuf_t self, const char *src, size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_MATCH_DYNABUF_H_
