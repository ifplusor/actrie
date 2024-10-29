/**
 * utf8helper.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "actrie/utf8helper.h"

#include <alib/string/utf8.h>

utf8_ctx_t alloc_utf8_context(void) {
  utf8_ctx_t utf8_ctx = amalloc(sizeof(utf8_ctx_s));
  if (utf8_ctx != NULL) {
    utf8_ctx->pos = NULL;
    utf8_ctx->len = 0;
  }
  return utf8_ctx;
}

void free_utf8_context(utf8_ctx_t context) {
  if (context != NULL) {
    afree(context->pos);
    afree(context);
  }
}

bool reset_utf8_context(utf8_ctx_t context, char content[], size_t len) {
  void* ptr = arealloc(context->pos, (len + 1) * sizeof(size_t));
  if (ptr == NULL) {
    return false;
  }

  context->pos = ptr;
  context->len = len;
  utf8_word_pos(content, context->len, context->pos);

  return true;
}

size_t fix_utf8_pos(size_t pos, size_t diff, bool plus_or_subtract, void* arg) {
  if (diff == 0) {
    return pos;
  }
  utf8_ctx_t utf8_ctx = (utf8_ctx_t)arg;
  size_t diff_pos;
  if (plus_or_subtract) {
    if (utf8_ctx->len - pos <= diff * 3) {
      diff_pos = utf8_ctx->len;
    } else {
      diff_pos = pos + diff * 3;
    }
    while (utf8_ctx->pos[diff_pos] - utf8_ctx->pos[pos] > diff) {
      diff_pos--;
    }
  } else {
    if (diff * 3 >= pos) {
      diff_pos = 0;
    } else {
      diff_pos = pos - diff * 3;
    }
    while (utf8_ctx->pos[pos] - utf8_ctx->pos[diff_pos] > diff) {
      diff_pos++;
    }
  }
  return diff_pos;
}
