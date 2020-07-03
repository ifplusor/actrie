/**
 * utf8ctx.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "utf8ctx.h"

utf8ctx_t utf8ctx_alloc_context(matcher_t matcher) {
  context_t context;
  utf8ctx_t utf8ctx;

  do {
    context = matcher_alloc_context(matcher);
    if (context == NULL) {
      break;
    }

    utf8ctx = amalloc(sizeof(utf8ctx_s));
    if (utf8ctx == NULL) {
      break;
    }

    utf8ctx->matcher_ctx = context;

    utf8ctx->utf8_ctx.pos = NULL;
    utf8ctx->utf8_ctx.len = 0;
    matcher_fix_pos(utf8ctx->matcher_ctx, fix_utf8_pos, &utf8ctx->utf8_ctx);

    utf8ctx->return_byte_pos = false;

    return utf8ctx;
  } while (0);

  matcher_free_context(context);

  return NULL;
}

void utf8ctx_free_context(utf8ctx_t utf8ctx) {
  if (utf8ctx != NULL) {
    matcher_free_context(utf8ctx->matcher_ctx);
    afree(utf8ctx->utf8_ctx.pos);
    afree(utf8ctx);
  }
}

bool utf8ctx_reset_context(utf8ctx_t utf8ctx, char* content, int len, bool return_byte_pos) {
  do {
    if (utf8ctx == NULL) {
      break;
    }

    utf8ctx->return_byte_pos = return_byte_pos;

    if (!reset_utf8_context(&utf8ctx->utf8_ctx, content, len)) {
      break;
    }

    matcher_reset_context(utf8ctx->matcher_ctx, content, len);

    return true;
  } while (0);

  return false;
}

word_t utf8ctx_next(utf8ctx_t utf8ctx) {
  if (utf8ctx == NULL) {
    return NULL;
  }

  word_t matched_word = matcher_next(utf8ctx->matcher_ctx);

  if (matched_word != NULL && !utf8ctx->return_byte_pos) {
    matched_word->pos.so = utf8ctx->utf8_ctx.pos[matched_word->pos.so];
    matched_word->pos.eo = utf8ctx->utf8_ctx.pos[matched_word->pos.eo];
  }

  return matched_word;
}

word_t utf8ctx_next_prefix(utf8ctx_t utf8ctx) {
  if (utf8ctx == NULL) {
    return NULL;
  }

  word_t matched_word = matcher_next_prefix(utf8ctx->matcher_ctx);

  if (matched_word != NULL && !utf8ctx->return_byte_pos) {
    matched_word->pos.so = utf8ctx->utf8_ctx.pos[matched_word->pos.so];
    matched_word->pos.eo = utf8ctx->utf8_ctx.pos[matched_word->pos.eo];
  }

  return matched_word;
}
