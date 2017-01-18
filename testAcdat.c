#include <time.h>

#include "acdat.h"


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
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %lds\n", s1-s0);
	fclose(fpdict);
	
	// 排序字典树节点
	trie_sort_to_line(&dict);
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %lds\n", s2 -s1);

	dat_trie dat;
	dat_init(&dat, pdict);

	// 建立 Double-Array Trie
	dat_construct(&dat, &dict);
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %lds\n", s3-s2);
	
	// 建立 AC 自动机
	dat_construct_automation(&dat, &dict);
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %lds\n", s4-s3);
	
	trie_close(&dict);

	// test match
	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		char company[100000];
		dat_context context;
		long long s5 = system_millisecond();
		while (fgets(company, 100000, fpcorpus) != NULL) {
			dat_init_context(&context, &dat, company, strlen(company));
			while (dat_ac_next(&context)) {
				int len = context.out_matched->dat_depth;
				if (len > 0) {
					printf("%zu,%zu,%d: %s\n", context._i-len, context._i, len,
						   context.out_matched->dat_keyword);
				} else {
					fprintf(stderr, "size is small.\n");
				}
			}
			//matchAcdat(&dat, company, strlen(company));
		}
		long long s6 = system_millisecond();
		fprintf(stderr, "s5: %lldms\n", s6-s5);
		fclose(fpcorpus);
	}

	// 释放字典树
	dat_close(&dat);

	return 0;
}

