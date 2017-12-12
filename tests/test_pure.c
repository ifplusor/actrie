//
// Created by james on 6/16/17.
//

#include <matcher.h>
#include <utf8.h>

int main() {

#ifdef _WIN32
  system("chcp 65001");
#endif

  matcher_t matcher = matcher_construct_by_file(matcher_type_dat, "/home/james/Downloads/dict.txt");
  printf("load success!\n");
  printf("use memory %zu\n", amalloc_used_memory());

  context_t ctx = matcher_alloc_context(matcher);

  FILE *fin = fopen("/home/james/Downloads/corpus.txt", "r");
  if (fin == NULL) exit(-1);

  FILE *fout = fopen("match.out", "w");
  if (fout == NULL) exit(-1);

  char content[1000000];
  char *contents[40100];
  size_t lens[40100];
  int len = 0;
  while (fscanf(fin, "%[^\n]\n", content) != EOF) {
    contents[len] = strdup(content);
    lens[len] = strlen(content);
    len++;
  }

  int count = 0, c = 0, t = 10;
  long long start = current_milliseconds();

  while (t--) {
    for (int i = 0; i < len; i++) {
//    while (fscanf(fin, "%[^\n]\n", content) != EOF) {
      count++;
      matcher_reset_context(ctx, contents[i], lens[i]);
      while (matcher_next(ctx)) {
        mdi_t idx = matcher_matched_index(ctx);
        strpos_s pos = matcher_matched_pos(ctx);
        strlen_s str = matcher_matched_str(ctx);
//      fprintf(fout, "%.*s(%d) - %s\n", (int) str.len, str.ptr,
//              (int) utf8_word_length(str.ptr, str.len), idx->extra);
//        fprintf(fout, "%d: [%zu,%zu] %.*s(%d) - %s\n",
//                count, pos.so, pos.eo, (int) str.len, str.ptr,
//                (int) utf8_word_length(str.ptr, str.len), idx->extra);
        c++;
      }
    }

//    fseek(fin, 0, SEEK_SET);
  }

  long long end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  matcher_free_context(ctx);
  matcher_destruct(matcher);

  fclose(fin);
  fclose(fout);

  for (int i = 0; i < len; i++) {
    free(contents[i]);
  }

  return 0;
}
