//
// Created by james on 6/22/17.
//

#include <matcher.h>
#include "../src/matcher0.h"

int main() {
  char content[150000] = "abcdefg";

//  matcher_t matcher = matcher_construct_by_file(matcher_type_ultimate, "rule");
  matcher_t matcher = matcher_construct_by_string(matcher_type_ultimate, "(f|(a|b).{0,5}(e(?&!ef)|g))");
  context_t context = matcher_alloc_context(matcher);

  FILE *fin = fopen("corpus.txt", "r");
  if (fin == NULL) exit(-1);

//  FILE *fout = fopen("match.out", "w");
  FILE *fout = stdout;

  int count = 0;

  sint64_t start = current_milliseconds();
//  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;

    matcher_reset_context(context, content, strlen(content));
    while (matcher_next(context)) {
      mdi_t idx = matcher_matched_index(context);
      fprintf(fout, "%.*s(%d) - %s\n",
              (int) idx->length, idx->mdi_keyword, idx->wlen, idx->mdi_extra);
//      fprintf(fout, "%d: [%zu,%zu] %.*s(%d) - %s\n",
//              count, context->out_eo - idx->length, context->out_eo,
//              (int) idx->length, idx->_keyword, idx->wlen, idx->_extra);
    };
//  }

  sint64_t end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);

  matcher_free_context(context);
  matcher_destruct(matcher);

  fclose(fin);
  fclose(fout);
}
