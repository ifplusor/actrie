//
// Created by james on 11/14/17.
//

#include <utf8.h>
#include "utf8ctx.h"

wctx_t alloc_context(matcher_t matcher) {
  context_t context;
  wctx_t wctx;

  do {
    context = matcher_alloc_context(matcher);
    if (context == NULL) break;

    wctx = malloc(sizeof(wctx_s));
    if (wctx == NULL) break;

    wctx->ctx = context;
    wctx->pos = NULL;

    return wctx;
  } while (0);

  matcher_free_context(context);

  return NULL;
}

bool free_context(wctx_t wctx) {
  if (wctx == NULL)
    return false;

  free(wctx->pos);
  context_t ctx = wctx->ctx;
  free(wctx);
  return matcher_free_context(ctx);
}

bool reset_context(wctx_t wctx, char *content, int length) {
  do {
    if (wctx == NULL)
      break;

    if (!matcher_reset_context(wctx->ctx, content, (size_t) length))
      break;

    wctx->pos = malloc((length + 1) * sizeof(size_t));
    if (wctx->pos == NULL) break;

    utf8_word_position(content, (size_t) length, wctx->pos);

    return true;
  } while (0);

  return false;
}

bool next(wctx_t wctx) {
  if (wctx == NULL)
    return false;

  return matcher_next(wctx->ctx);
}
