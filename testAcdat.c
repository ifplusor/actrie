#include <time.h>

#include "acdat.h"


int main(int argc, char *argv[])
{
	FILE *fpdict = fopen("sdict", "rb");
	if (fpdict == NULL) return -1;

	initTrie();
	initDat();

	// 建立字典树
	time_t s0 = time(NULL);
	constructTrie(fpdict);
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %lds\n", s1-s0);
	fclose(fpdict);
	
	// 排序字典树节点
	sortTrieForBinarySearch();
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %lds\n", s2 -s1);

	// 建立 Double-Array Trie
	constructDat();
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %lds\n", s3-s2);
	
	// 建立 AC 自动机
	constructDatAutomation();
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %lds\n", s4-s3);
	
	closeTrie();

	// test match
	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		unsigned char company[100000];
		long long s5 = getSystemTime();
		while (fscanf(fpcorpus, "%s", company) != EOF) matchAcdat(company);
		long long s6 = getSystemTime();
		fprintf(stderr, "s5: %lldms\n", s6-s5);
		fclose(fpcorpus);
	}

	// 释放字典树
	closeDat();

	return 0;
}

