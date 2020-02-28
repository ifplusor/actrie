/**
 * utf8ctx.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "utf8ctx.h"

#include <alib/string/utf8.h>

wctx_t alloc_context(matcher_t matcher) {
  context_t context;
  wctx_t wctx;

  do {
    context = matcher_alloc_context(matcher);
    if (context == NULL) {
      break;
    }

    wctx = malloc(sizeof(wctx_s));
    if (wctx == NULL) {
      break;
    }

    wctx->ctx = context;
    wctx->pos = NULL;

    return wctx;
  } while (0);

  matcher_free_context(context);

  return NULL;
}

void free_context(wctx_t wctx) {
  if (wctx != NULL) {
    matcher_free_context(wctx->ctx);
    free(wctx->pos);
    free(wctx);
  }
}

bool reset_context(wctx_t wctx, char* content, int length) {
  do {
    if (wctx == NULL) {
      break;
    }

    if (!matcher_reset_context(wctx->ctx, content, (size_t)length)) {
      break;
    }

    if (wctx->pos != NULL) {
      free(wctx->pos);
    }
    wctx->pos = malloc((length + 1) * sizeof(size_t));
    if (wctx->pos == NULL) {
      break;
    }

    utf8_word_pos(content, (size_t)length, wctx->pos);

    return true;
  } while (0);

  return false;
}

word_t next(wctx_t wctx) {
  if (wctx == NULL) {
    return NULL;
  }

  return matcher_next(wctx->ctx);
}
