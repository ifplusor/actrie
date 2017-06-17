//
// Created by james on 6/16/17.
//

#include <matcher.h>


//const int HISTORY_SIZE = 5000;
#define HISTORY_SIZE 5000


int main()
{
	struct _context history[HISTORY_SIZE];  /* 循环表 */
	int hcount = 0, hidx = 0;

	matcher_t pdat1, pdat2;
	context_t ctx1, ctx2;

	char content[150000];

	pdat1 = matcher_construct_by_file(matcher_type_acdat, "t1.txt");
	pdat2 = matcher_construct_by_file(matcher_type_acdat, "t2.txt");

	ctx1 = matcher_alloc_context(pdat1);
	ctx2 = matcher_alloc_context(pdat2);

	FILE *fin = fopen("corpus.txt", "r");
	FILE *fout = fopen("match.out", "w");


	int count = 0;

	long long start = system_millisecond();
	while (fscanf(fin, "%[^\n]\n", content) != EOF) {
		count++;

		hcount = 0, hidx = 0;

		matcher_reset_context(ctx1, content, strlen(content));
		matcher_reset_context(ctx2, content, strlen(content));

		while (matcher_next(ctx1)) {
			// check history
			for (int i = (HISTORY_SIZE + hidx - hcount) % HISTORY_SIZE; i != hidx; i = (i + 1) % HISTORY_SIZE) {
				if (history[i].out_e <= ctx1->out_e) {
					hcount--;
				} else {
					long diff_pos = history[i].out_e - history[i].out_matched_index->length - ctx1->out_e;
					if (diff_pos < 0) {
						continue;
					} else if (diff_pos > 45) {
						goto next_round;
					}
					if (strcmp(history[i].out_matched_index->extra, ctx1->out_matched_index->extra) == 0 &&
						diff_pos <= (*ctx1->out_matched_index->tag - '0')) {
						char c = content[history[i].out_e];
						content[history[i].out_e] = '\0';
						fprintf(fout, "%s - %s\n", &content[ctx1->out_e - ctx1->out_matched_index->length],
								ctx1->out_matched_index->extra);
						content[history[i].out_e] = c;
						//printf("hit: %zu, %zu\n", ctx1.out_e - ctx1.out_matched->depth, history[i].out_e);
					}
				}
			}
			if (hcount == 0) {  // locate w2
				while (matcher_next(ctx2)) {
					if (ctx2->out_e > ctx1->out_e) goto check_w2;
				}
				break;  // none w2
			}
			while (matcher_next(ctx2)) {
check_w2:
				history[hidx] = *ctx2;
				hidx = (hidx + 1) % HISTORY_SIZE;
				hcount++;
				long diff_pos = ctx2->out_e - ctx2->out_matched_index->length - ctx1->out_e;
				if (diff_pos < 0) {
					continue;
				} else if (diff_pos > 45) {
					goto next_round;
				}
				if (strcmp(ctx2->out_matched_index->extra, ctx1->out_matched_index->extra) == 0 &&
					diff_pos <= (*ctx1->out_matched_index->tag - '0')) {
					char c = content[ctx2->out_e];
					content[ctx2->out_e] = '\0';
					fprintf(fout, "%s - %s\n", &content[ctx1->out_e - ctx1->out_matched_index->length],
							ctx1->out_matched_index->extra);
					content[ctx2->out_e] = c;
					//printf("hit: %zu, %zu\n", ctx1.out_e - ctx1.out_matched->depth, ctx2.out_e);
				}
			}
next_round:;
		}

//        if (count % 100 == 0) {
//            printf("p: %d\n", count);
//        }
	}

	long long end = system_millisecond();
	double time = (double) (end - start) / 1000;
	printf("time: %lfs\n", time);
	printf("line: %d\n", count);

	matcher_free_context(ctx1);
	matcher_free_context(ctx2);

	matcher_destruct(pdat1);
	matcher_destruct(pdat2);

	fclose(fin);
	fclose(fout);

	getchar();

	return 0;
}