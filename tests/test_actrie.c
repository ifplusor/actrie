#include <time.h>
#include <actrie.h>


int main(int argc, char *argv[])
{
	// 建立字典树
	trie_ptr prime_trie = trie_construct_by_file("sdict", true);
	if (prime_trie == NULL) return -1;

	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		unsigned char company[100000];
		time_t s4 = time(NULL);
		while(fscanf(fpcorpus, "%s", company) != EOF)
			trie_ac_match(prime_trie, company, strlen(company));
		time_t s5 = time(NULL);
		fprintf(stderr, "s5: %lds\n", s5-s4);
		fclose(fpcorpus);
	}

	trie_destruct(prime_trie);

	return 0;
}

