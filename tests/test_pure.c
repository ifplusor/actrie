//
// Created by james on 6/16/17.
//

#include <matcher.h>
#include "../src/matcher0.h"

int main() {
  matcher_t matcher;
  context_t ctx;
  char content[150000] = "12345123\00045";

#ifdef _WIN32
  system("chcp 65001");
#endif

  strlen_s str = {
      .ptr = "123\00045\n23\n12345\n45\n",
      .len = 19
  };
  matcher = matcher_construct_by_string(matcher_type_dat, &str);
//  matcher = matcher_construct_by_file(matcher_type_acdat, "testdata/rule_pure.txt");
  ctx = matcher_alloc_context(matcher);

//  FILE *fin = fopen("testdata/corpus.txt", "r");
//  if (fin == NULL) exit(-1);

  FILE *fout = stdout;
//  FILE *fout = fopen("match.out", "w");
//  if (fout == NULL) exit(-1);

  int count = 0, c = 0;

  long long start = current_milliseconds();
//  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;
    matcher_reset_context(ctx, content, 11);
    while (matcher_next(ctx)) {
      mdi_t idx = matcher_matched_index(ctx);
      fprintf(fout, "%.*s(%d) - %s\n",
              (int) idx->length, idx->keyword, idx->wlen, idx->extra);
//      fprintf(fout, "[%zu,%zu] %.*s(%d) - %s\n",
//              ctx->out_eo - idx->length, ctx->out_eo,
//              (int) idx->length, idx->keyword, idx->wlen, idx->extra);
      c++;
    }
//  }
  long long end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  matcher_free_context(ctx);

  matcher_destruct(matcher);

//  fclose(fin);
//  fclose(fout);

  return 0;
}