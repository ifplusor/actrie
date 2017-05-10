#include <time.h>
#include <acdat.h>


int main(int argc, char *argv[])
{
	FILE *fpdict = fopen("sdict", "rb");
	if (fpdict == NULL) return -1;

	// 建立字典树
	time_t s0 = time(NULL);
	trie_ptr prime_trie = trie_construct_by_file(fpdict);
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %lds\n", s1-s0);
	fclose(fpdict);
	
	// 排序字典树节点
	trie_sort_to_line(prime_trie);
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %lds\n", s2 -s1);

	// 建立 Double-Array Trie
	dat_trie_ptr dat = dat_construct(prime_trie);
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %lds\n", s3-s2);
	
	// 建立 AC 自动机
	dat_construct_automation(dat, prime_trie);
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %lds\n", s4-s3);
	
	trie_release(prime_trie);

	// test match
	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		char company[100000];
		dat_context context;
		long long s5 = system_millisecond();
		while (fgets(company, 100000, fpcorpus) != NULL) {
			dat_init_context(&context, dat, company, strlen(company));
			while (dat_ac_next_on_index(&context)) {
				int len = context.out_matched->dat_depth;
				if (len > 0) {
					printf("%zu,%zu,%d: %s\n", context.out_e-len, context.out_e, len,
						   context.out_matched_index->keyword);
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
	dat_release(dat);

	return 0;
}

