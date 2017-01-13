#ifndef _MATCH_COMMON_H_
#define _MATCH_COMMON_H_


#include <stdlib.h>
#include <stdbool.h>


#ifndef __bool_true_false_are_defined
#define bool int
#define true 1
#define false 0
#endif


#define REGIONSIZE 0x00001000
#define REGIONOFFSET 18
#define POSITIONMASK 0x0003FFFF


extern const size_t POOLREGIONSIZE;
extern const size_t POOLPOSITIONSIZE;


long long getSystemTime();


#endif
