#include <time.h>
#include <string.h>

#include "acdat.h"


int main(int argc, char *argv[])
{
	FILE *fpdict = fopen("sdict", "rb");
	if (fpdict == NULL) return -1;

	trie dict;
	datrie dat;

	initTrie(&dict);
	initDat(&dat);

	// 建立字典树
	time_t s0 = time(NULL);
	constructTrie(&dict, fpdict);
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %lds\n", s1-s0);
	fclose(fpdict);
	
	// 排序字典树节点
	sortTrieToLine(&dict);
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %lds\n", s2 -s1);

	// 建立 Double-Array Trie
	constructDat(&dat, &dict);
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %lds\n", s3-s2);
	
	// 建立 AC 自动机
	constructDatAutomation(&dat, &dict);
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %lds\n", s4-s3);
	
	closeTrie(&dict);

	// test match
	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		unsigned char company[100000], buffer[1000];
		datcontext context;
		long long s5 = getSystemTime();
		while (fscanf(fpcorpus, "%s", company) != EOF) {
			initAcdatContext(&context, &dat, company, strlen(company));
			while (nextWithAcdat(&context)) {
				int len = getMatchedInAcdat(&context, buffer, 1000);
				if (len > 0) {
					printf("%zu,%zu,%d: %s\n", context._i-len, context._i, len, buffer);
				} else {
					fprintf(stderr, "size is small.\n");
				}
			}
			//matchAcdat(&dat, company, strlen(company));
		}
		long long s6 = getSystemTime();
		fprintf(stderr, "s5: %lldms\n", s6-s5);
		fclose(fpcorpus);
	}

	// 释放字典树
	closeDat(&dat);

	return 0;
}

