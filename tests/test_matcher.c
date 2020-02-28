/**
 * test_matcher.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include <matcher.h>

int main() {
  char* str = "(好用|好吃(?&!吃的)|真棒).{3,6}(人|东西)\t123\n东西.{0,6}(?<!的|和)真棒\t2\n";
  strlen_s pattern = {.ptr = str, .len = strlen(str)};

  printf("use memory: %zu\n", amalloc_used_memory());
  matcher_t matcher = matcher_construct_by_string(&pattern);
  printf("use memory: %zu\n", amalloc_used_memory());
  if (matcher == NULL) {
    printf("build matcher failed!");
    return -1;
  }

  char* content = "好吃的东西给真棒的人";
  context_t context = matcher_alloc_context(matcher);
  matcher_reset_context(context, content, strlen(content));

  word_t matched = matcher_next(context);
  while (matched != NULL) {
    printf("%.*s: %.*s\n", (int)matched->keyword.len, matched->keyword.ptr, (int)matched->extra.len,
           matched->extra.ptr);
    matched = matcher_next(context);
  }

  printf("use memory: %zu\n", amalloc_used_memory());
  matcher_free_context(context);
  printf("use memory: %zu\n", amalloc_used_memory());

  matcher_destruct(matcher);
  printf("use memory: %zu\n", amalloc_used_memory());

  return 0;
}
