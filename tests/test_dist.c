//
// Created by james on 6/22/17.
//

#include <matcher.h>

int main()
{
  char content[150000] = "发货，您验货，满意 满意 发货，您验货，满意";

  matcher_t matcher = matcher_construct_by_file(matcher_type_distance, "t3.txt");
  context_t context = matcher_alloc_context(matcher);

  FILE *fin = fopen("corpus.txt", "r");
  FILE *fout = fopen("match.out", "w");

  int count = 0;

  long long start = system_millisecond();
  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;

    matcher_reset_context(context, content, strlen(content));
    while (matcher_next(context)) {
      fprintf(fout, "%s - %zu\n", context->out_matched_index->keyword,
              context->out_matched_index->wlen);
    };

//        if (count % 100 == 0) {
//            printf("p: %d\n", count);
//        }
  }

  long long end = system_millisecond();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);

  matcher_free_context(context);
  matcher_destruct(matcher);

  fclose(fin);
  fclose(fout);

//	getchar();
}