/**
 * test_trie.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "../src/trie/acdat.h"
#include "../src/trie/actrie.h"
#include "../src/vocab.h"

trie_node_t trie_next_node_by_binary(trie_t self, trie_node_t pNode, unsigned char key);

size_t trie_match(trie_t self, unsigned char content[], size_t len) {
  size_t i, j, c = 0;
  for (i = 0; i < len; ++i) {
    trie_node_t pCursor = self->root;
    for (j = i; j < len; ++j) {
      trie_node_t pNext = trie_next_node_by_binary(self, pCursor, content[j]);
      if (pNext == self->root) {
        break;
      }
      pCursor = pNext;
      if (pNext->value != NULL) {
        c++;
      }
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
      if (pNext->value != NULL) {
        c++;
      }
      pNext = trie_access_node(self, pNext->trie_failed);
    }
  }
  return c;
}

int main() {
  bool enable_automation = false;

  vocab_t vocab = vocab_construct(stream_type_file, "company.txt");
  if (vocab == NULL) {
    exit(-1);
  }

  trie_t prime_trie = trie_alloc();
  if (prime_trie == NULL) {
    exit(-1);
  }

  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    trie_add_keyword(prime_trie, keyword.ptr, keyword.len, 1);
  }
  trie_sort_to_line(prime_trie);
  if (enable_automation) {
    trie_build_automation(prime_trie);
  }

  printf("use memory %zu\n", amalloc_used_memory());
  printf("use node %zu\n", segarray_size(prime_trie->node_array));

  FILE* fin = fopen("corpus1.txt", "r");
  if (fin == NULL) {
    exit(-1);
  }

  printf("load success!\n");

  char content[1500000];
  int count = 0, c = 0, t = 1;
  long long start = current_milliseconds();

  while (t--) {
    while (fscanf(fin, "%[^\n]\n", content) != EOF) {
      count++;
      size_t len = strlen(content);
      if (false) {
        c += trie_ac_match(prime_trie, content, len);
      } else {
        c += trie_match(prime_trie, content, len);
      }
    }

    fseek(fin, 0, SEEK_SET);
  }

  long long end = current_milliseconds();
  double time = (double)(end - start) / 1000;
  printf("time: %lfs\n", time);
  printf("line: %d\n", count);
  printf("match: %d\n", c);

  fclose(fin);

  return 0;
}
