/**
 * utf8ctx.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "utf8ctx.h"

wctx_t alloc_context(matcher_t matcher) {
  context_t context;
  wctx_t wctx;

  do {
    context = matcher_alloc_context(matcher);
    if (context == NULL) {
      break;
    }

    wctx = amalloc(sizeof(wctx_s));
    if (wctx == NULL) {
      break;
    }

    wctx->matcher_ctx = context;

    wctx->utf8_ctx.pos = NULL;
    wctx->utf8_ctx.len = 0;
    matcher_fix_pos(wctx->matcher_ctx, fix_utf8_pos, &wctx->utf8_ctx);

    wctx->return_byte_pos = false;

    return wctx;
  } while (0);

  matcher_free_context(context);

  return NULL;
}

void free_context(wctx_t wctx) {
  if (wctx != NULL) {
    matcher_free_context(wctx->matcher_ctx);
    afree(wctx->utf8_ctx.pos);
    afree(wctx);
  }
}

bool reset_context(wctx_t wctx, char* content, int len, bool return_byte_pos) {
  do {
    if (wctx == NULL) {
      break;
    }

    wctx->return_byte_pos = return_byte_pos;

    if (!reset_utf8_context(&wctx->utf8_ctx, content, len)) {
      break;
    }

    matcher_reset_context(wctx->matcher_ctx, content, len);

    return true;
  } while (0);

  return false;
}

word_t next(wctx_t wctx) {
  if (wctx == NULL) {
    return NULL;
  }

  return matcher_next(wctx->matcher_ctx);
}

word_t next_prefix(wctx_t wctx) {
  if (wctx == NULL) {
    return NULL;
  }

  return matcher_next_prefix(wctx->matcher_ctx);
}
