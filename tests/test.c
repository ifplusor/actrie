//
// Created by james on 6/16/17.
//

#include <matcher.h>

int main() {
  matcher_t pdat1;
  context_t ctx1;

  char content[150000];

  pdat1 = matcher_construct_by_file(matcher_type_acdat, "n.dict");
  ctx1 = matcher_alloc_context(pdat1);
  FILE *fin = fopen("corpus.txt", "r");
  FILE *fout = fopen("match.out", "w");

  int count = 0, c = 0;

  long long start = system_millisecond();
  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;
    matcher_reset_context(ctx1, content, strlen(content));
    while (matcher_next(ctx1)) {
//        fprintf(fout, "%s\n", ctx1->out_matched_index->keyword);
      c++;
    }
  }
  long long end = system_millisecond();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  matcher_free_context(ctx1);

  matcher_destruct(pdat1);

  fclose(fin);
  fclose(fout);

  return 0;
}