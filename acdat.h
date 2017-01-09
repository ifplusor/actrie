#ifndef _MATCH_ACDAT_H_
#define _MATCH_ACDAT_H_


#include "actrie.h"


typedef struct datnode {
	size_t base,check;
	union {
		size_t next;
		size_t failed;
	} nf;
	union {
		size_t last;
		struct {
			unsigned char flag;
			unsigned char key;
		} f;
	} lf;
} datnode, *datnode_ptr;


void initDat();
void closeDat();
void constructDat();
void matchDat(unsigned char *content);
void constructDatAutomation();
void matchAcdat(unsigned char *content);


#endif
