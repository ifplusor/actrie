#include <time.h>

#include "actrie.h"


int main(int argc, char *argv[])
{
	FILE *fpdict = fopen("sdict", "rb");
	if (fpdict == NULL) return -1;

	trie dict;
	trie_init(&dict);

	match_dict_ptr pdict = (match_dict_ptr) malloc(sizeof(match_dict));
	pdict->buff = NULL;

	// 建立字典树
	time_t s0 = time(NULL);
	trie_construct(&dict, fpdict, pdict);
	fclose(fpdict);
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %lds\n", s1-s0);

	// 按bfs顺序排序节点
    trie_sort_to_line(&dict);
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %lds\n", s2-s1);
	
	// 建立父指针关系
	trie_rebuild_parent_relation(&dict);
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %lds\n", s3-s2);
	
	// 构建自动机
	trie_construct_automation(&dict);
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %lds\n", s4-s3);

	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		unsigned char company[100000];
		while(fscanf(fpcorpus, "%s", company) != EOF)
			trie_ac_match(&dict, company, strlen(company));
		time_t s5 = time(NULL);
		fprintf(stderr, "s5: %lds\n", s5-s4);
		fclose(fpcorpus);
	}

	trie_close(&dict);
	dict_release(pdict);
	free(pdict);

	return 0;
}

