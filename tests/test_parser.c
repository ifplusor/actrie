/**
 * test_parser.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "../src/parser/parser.h"

int main() {
  char* str = "(好用|好吃|真棒).{1,3}(人|东西)";
  strlen_s pattern = {.ptr = str, .len = strlen(str)};

  printf("before parser, use memory: %zu\n", amalloc_used_memory());
  ptrn_t ptrn = parse_pattern(&pattern);
  printf("after parser, use memory: %zu\n", amalloc_used_memory());
  _release(ptrn);
  printf("after release, use memory: %zu\n", amalloc_used_memory());

  return 0;
}