//
// Created by james on 6/22/17.
//

#include <matcher.h>
#include <utf8.h>

int main() {

  matcher_t matcher = matcher_construct_by_file(matcher_type_acdat, "/home/james/Downloads/dict.txt");
  printf("load success!\n");
  printf("use memory %zu\n", amalloc_used_memory());

  context_t context = matcher_alloc_context(matcher);

  FILE *fin = fopen("/home/james/Downloads/corpus.txt", "r");
  if (fin == NULL) exit(-1);

  FILE *fout = fopen("match2.out", "w");
  if (fout == NULL) exit(-1);

  char content[1000000];
  int count = 0, c = 0, t = 1;
  sint64_t start = current_milliseconds();

  while (t--) {
    while (fscanf(fin, "%[^\n]\n", content) != EOF) {
      count++;

      matcher_reset_context(context, content, strlen(content));
      while (matcher_next(context)) {
        mdi_t idx = matcher_matched_index(context);
        strpos_s pos = matcher_matched_pos(context);
        strlen_s str= matcher_matched_str(context);
//      fprintf(fout, "%.*s(%d) - %s\n", (int) str.len, str.ptr,
//              (int) utf8_word_len(str.ptr, str.len), idx->extra);
        fprintf(fout, "%d: [%zu,%zu] %.*s(%d) - %s\n",
                count, pos.so, pos.eo, (int) str.len, str.ptr,
                (int) utf8_word_len(str.ptr, str.len), idx->extra);
        c++;
      };
    }

    fseek(fin, 0, SEEK_SET);
  }

  sint64_t end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  matcher_free_context(context);
  matcher_destruct(matcher);

  fclose(fin);
  fclose(fout);
}
