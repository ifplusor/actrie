//
// Created by james on 9/28/17.
//

#ifndef _MATCH_DYNABUF_H_
#define _MATCH_DYNABUF_H_

#include <common.h>
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !DYNABUF_EXTEND_SPACE_SIZE
#define DYNABUF_EXTEND_SPACE_SIZE 1024
#endif

typedef enum dynabuf_prop {
  dynabuf_prop_empty = 0,
  dynabuf_prop_fixed = 1,
} dynabuf_prop_f;

typedef struct dynamic_buffer {
  char *_buffer;
  size_t _size, _cur; // start from 1, reserve 0
  dynabuf_prop_f _prop;
} dynabuf_s, *dynabuf_t;

dynabuf_t dynabuf_alloc();
bool dynabuf_free(dynabuf_t self);

bool dynabuf_init(dynabuf_t self, size_t size, bool fixed);
bool dynabuf_clean(dynabuf_t self);
bool dynabuf_reset(dynabuf_t self);

char *dynabuf_content(dynabuf_t self, strcur_s cursor);
strlen_s dynabuf_split(dynabuf_t self, strpos_s pos);
strcur_s dynabuf_related(dynabuf_t self, strcur_s base, char *related);
strcur_s dynabuf_cur2ptr(dynabuf_t self, strcur_s cursor);
size_t dynabuf_length(dynabuf_t self);
size_t dynabuf_cursor(dynabuf_t self);
strcur_s dynabuf_empty_cur(dynabuf_t self);
bool dynabuf_empty(dynabuf_t self);

strcur_s dynabuf_write(dynabuf_t self, const char *src, size_t len);
strcur_s dynabuf_write_with_zero(dynabuf_t self,
                                    const char *src,
                                    size_t len);

int dynabuf_consume_until(dynabuf_t self,
                          stream_t stream,
                          const char *delim,
                          strpos_t out_pos);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_MATCH_DYNABUF_H_
