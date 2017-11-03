//
// Created by james on 6/22/17.
//

#include <matcher.h>
#include "../src/matcher0.h"

int main() {
  char content[150000] = "abcdefg";
//  char content[150000] = "我们华人的祖国是中华人民共和国";

  matcher_t matcher = matcher_construct_by_file(matcher_type_ultimate, "testdata/rule_dist.txt");
//  matcher_t matcher = matcher_construct_by_string(matcher_type_ultimate, "(f|(a|b).{0,5}(e(?&!ef)|g))");
//  matcher_t matcher = matcher_construct_by_string(matcher_type_ultimate, "华人(?&!人民|中华)\n中华|共和国(?&!中华人民共和国)\n");

  context_t context = matcher_alloc_context(matcher);

  FILE *fin = fopen("testdata/corpus.txt", "r");
  if (fin == NULL) exit(-1);

  FILE *fout = fopen("match.out", "w");
  if (fout == NULL) exit(-1);
//  FILE *fout = stdout;

  int count = 0;

  sint64_t start = current_milliseconds();
  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;

    matcher_reset_context(context, content, strlen(content));
    while (matcher_next(context)) {
      mdi_t idx = matcher_matched_index(context);
      fprintf(fout, "%.*s(%d) - %s\n",
              (int) idx->length, idx->keyword, idx->wlen, idx->extra);
//      fprintf(fout, "%d: [%zu,%zu] %.*s(%d) - %s\n",
//              count, context->out_eo - idx->length, context->out_eo,
//              (int) idx->length, idx->_keyword, idx->wlen, idx->_extra);
    };
  }

  sint64_t end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);

  matcher_free_context(context);
  matcher_destruct(matcher);

  fclose(fin);
  fclose(fout);
}
