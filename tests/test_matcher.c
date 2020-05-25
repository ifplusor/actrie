/**
 * test_matcher.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include <matcher.h>
#include <utf8helper.h>

int main() {
  char* str = "不(好|会)好";
  strlen_s pattern = {.ptr = str, .len = strlen(str)};

  printf("use memory: %zu\n", amalloc_used_memory());
  matcher_t matcher = matcher_construct_by_string(&pattern, false, false, true);
  printf("use memory: %zu\n", amalloc_used_memory());
  if (matcher == NULL) {
    printf("build matcher failed!");
    return -1;
  }

  utf8_ctx_t utf8_ctx = alloc_utf8_context();
  context_t context = matcher_alloc_context(matcher);
  matcher_fix_pos(context, fix_utf8_pos, utf8_ctx);

  char* content = "真的不会好";
  reset_utf8_context(utf8_ctx, content, strlen(content));
  matcher_reset_context(context, content, strlen(content));

  word_t matched = matcher_next(context);
  while (matched != NULL) {
    printf("%.*s: %.*s\n", (int)matched->keyword.len, matched->keyword.ptr, (int)matched->extra.len,
           matched->extra.ptr);
    matched = matcher_next(context);
  }

  printf("use memory: %zu\n", amalloc_used_memory());
  matcher_free_context(context);
  free_utf8_context(utf8_ctx);
  printf("use memory: %zu\n", amalloc_used_memory());

  matcher_destruct(matcher);
  printf("use memory: %zu\n", amalloc_used_memory());

  return 0;
}
