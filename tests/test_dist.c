//
// Created by james on 6/22/17.
//

#include <matcher.h>
//#include "../src/distance.h"

int main() {
  char content[150000] = "苹果的果实134较饱满 较饱满";

  matcher_t matcher = matcher_construct_by_file(matcher_type_distance,
                                                "/home/james/Downloads/rule");
//                                                "n.dict");
//  FILE *fp = fopen("split.dict","w");
//  for (int i = 0; i < ((dist_matcher_t)matcher)->_dict->idx_count; i++) {
//    match_dict_index_t idx = ((dist_matcher_t)matcher)->_dict->index + i;
////    if (idx->prop & mdi_prop_reserve)
//      fprintf(fp, "%s %d\n", idx->_keyword, idx->prop);
//  }
//  fclose(fp);

  context_t context = matcher_alloc_context(matcher);

  FILE *fin = fopen("corpus.txt", "r");
  FILE *fout = fopen("match.out", "w");

  int count = 0;

  long long start = system_millisecond();
  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    count++;

    matcher_reset_context(context, content, strlen(content));
    while (matcher_next(context)) {
      match_dict_index_t idx = matcher_matched_index(context);
      fprintf(fout, "%.*s(%zu) - %s\n",
              (int) idx->length, idx->_keyword, idx->wlen, idx->_extra);
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

int main2() {
  char content[] = "012345678";

  matcher_t matcher = matcher_construct_by_string(matcher_type_acdat,
                                                  "123\n5\n3456");
  context_t context = matcher_alloc_context(matcher);
  matcher_reset_context(context, content, strlen(content));
  while (matcher_next(context)) {
    match_dict_index_t idx = matcher_matched_index(context);
    fprintf(stdout, "%.*s(%zu) - %s\n",
            (int) idx->length, idx->_keyword, idx->wlen, idx->_extra);
  };
  matcher_free_context(context);
  matcher_destruct(matcher);
}