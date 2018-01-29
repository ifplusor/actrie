//
// Created by james on 1/23/18.
//

#include "../src/parser.h"

int main() {
  char *str = "(?<!不)(好用|好吃|真棒.{1,3}(人|东西)";
  strlen_s pattern = {
      .ptr = str,
      .len = strlen(str)
  };

  tokenizer_init();

  printf("before parser, use memory: %zu\n", amalloc_used_memory());
  ptrn_t ptrn = parse_pattern(&pattern);
  printf("after parser, use memory: %zu\n", amalloc_used_memory());
  _release(ptrn);
  printf("after release, use memory: %zu\n", amalloc_used_memory());

  return 0;
}