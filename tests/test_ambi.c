//
// Created by james on 6/16/17.
//

#include <matcher.h>
#include "../src/matcher0.h"

int main() {
  matcher_t pdat1;
  context_t ctx1;

  char content[150000] = "我们华人的祖国是中华人民共和国";

  printf("content: %s\n", content);

#ifdef _WIN32
  system("chcp 65001");
#endif

//  pdat1 = matcher_construct_by_file(matcher_type_ambi, "n.dict");
  pdat1 = matcher_construct_by_string(matcher_type_ambi,
                                      "华人(?&!人民|中华)\n中华|共和国(?&!中华人民共和国)\n");
  ctx1 = matcher_alloc_context(pdat1);
//  FILE *fin = fopen("corpus.txt", "r");
//  FILE *fout = fopen("match.out", "w");
  FILE *fout = stdout;

  int count = 0, c = 0;

  long long start = current_milliseconds();
//  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;
    matcher_reset_context(ctx1, content, strlen(content));
    while (matcher_next(ctx1)) {
      mdi_t idx = matcher_matched_index(ctx1);
      fprintf(fout, "[%zu,%zu] %.*s(%d) - %s\n",
              ctx1->out_eo - idx->length, ctx1->out_eo,
              (int) idx->length, idx->mdi_keyword, idx->wlen, idx->mdi_extra);
      c++;
    }
//  }
  long long end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  matcher_free_context(ctx1);

  matcher_destruct(pdat1);

//  fclose(fin);
  fclose(fout);

  return 0;
}