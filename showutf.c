#include <stdio.h>

int main(int argc, char *argv[])
{
	char content[1000];
	while (scanf("%s", content) != EOF) {
		for (int i=0; content[i]!='\0'; i++) {
			printf("%d,", content[i]);
		}
		printf("\n");
	}
	return 0;
}
