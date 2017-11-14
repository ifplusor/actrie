//
// Created by james on 11/14/17.
//

#ifndef _ACTRIE_UTF8POS_H_
#define _ACTRIE_UTF8POS_H_

#include <matcher.h>

typedef struct context_wrapper {
  context_t ctx;
  size_t *pos;
} wctx_s, *wctx_t;

wctx_t alloc_context(matcher_t matcher);
bool free_context(wctx_t wctx);
bool reset_context(wctx_t wctx, char *content, int length);
bool next(wctx_t wctx);

#endif //_ACTRIE_UTF8POS_H_
