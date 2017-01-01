#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

int cmp(char *s1, char *s2)
{
	while (*s1 != 0 && *s2 != 0) {
		if (*s1 != *s2) {
			return *s1 - * s2;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("arg count error!");
		exit(-1);
	}

	FILE *fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		printf("open file error!");
		exit(-1);
	}

	int count = 1;
	
	char *cur, *last, *t;
	cur = (char*) malloc(1000);
	last = (char*) malloc(1000);
	fscanf(fp, "%s", last);
	while (fscanf(fp, "%s", cur) != EOF) {
		if (strcmp(last, cur) >= 0) {
			printf("order error:\n  >>> %s\n  <<< %s\n", last, cur);
		}
		t = cur;
		cur = last;
		last = t;

		count++;
	}

	free(last);
	free(cur);
	fclose(fp);

	printf("check finish, line: %d \n", count);

	return 0;
}
