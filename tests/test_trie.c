//
// Created by james on 6/16/17.
//


#include "../src/actrie.h"
#include "../src/acdat.h"


trie_node_t trie_next_node_by_binary(trie_t self, trie_node_t pNode, unsigned char key);

size_t trie_match(trie_t self, unsigned char content[], size_t len) {
  size_t i, j, c = 0;
  for (i = 0; i < len; ++i) {
    trie_node_t pCursor = self->root;
    for (j = i; j < len; ++j) {
      trie_node_t pNext = trie_next_node_by_binary(self, pCursor, content[j]);
      if (pNext == self->root) break;
      pCursor = pNext;
      if (pNext->idxlist != NULL) c++;
    }
  }
  return c;
}

size_t trie_ac_match(trie_t self, unsigned char content[], size_t len) {
  size_t i, c = 0;
  trie_node_t pCursor = self->root;
  for (i = 0; i < len; ++i) {
    trie_node_t pNext = trie_next_node_by_binary(self, pCursor, content[i]);
    while (pCursor != self->root && pNext == self->root) {
      pCursor = trie_access_node(self, pCursor->trie_failed);
      pNext = trie_next_node_by_binary(self, pCursor, content[i]);
    }
    pCursor = pNext;
    while (pNext != self->root) {
      if (pNext->idxlist != NULL) c++;
      pNext = trie_access_node(self, pNext->trie_failed);
    }
  }
  return c;
}


int main() {

  vocab_t vocab = vocab_construct(stream_type_file, "raw_company.txt");
  aobj acdat_conf = matcher_root_conf(1);
  acdat_conf = dat_matcher_conf(1, matcher_type_dat, acdat_conf);

  match_dict_t dict = dict_alloc();
  if (dict == NULL) exit(-1);
  if (!dict_parse(dict, vocab, acdat_conf)) exit(-1);

  trie_conf_s trie_conf = {
      .filter = 1,
      .enable_automation = true
  };
  trie_t prime_trie = trie_construct(dict, &trie_conf);
  if (prime_trie == NULL) exit(-1);

  printf("use memory %llu\n", amalloc_used_memory());

  FILE *fin = fopen("corpus.txt", "r");
  if (fin == NULL) exit(-1);

  printf("load success!\n");

  char content[1500000];
  int count = 0, c = 0, t = 5;
  long long start = current_milliseconds();

  while (t--) {
    while (fscanf(fin, "%[^\n]\n", content) != EOF) {
      count++;
      size_t len = strlen(content);
      if (trie_conf.enable_automation)
        c += trie_ac_match(prime_trie, content, len);
      else
        c += trie_match(prime_trie, content, len);
    }

    fseek(fin, 0, SEEK_SET);
  }

  long long end = current_milliseconds();
  double time = (double) (end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  fclose(fin);

  return 0;
}
