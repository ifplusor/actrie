#ifndef _MATCH_COMMON_H_
#define _MATCH_COMMON_H_


#include <stdlib.h>


#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif


#define REGIONSIZE 0x00001000
#define REGIONOFFSET 18
#define POSITIONMASK 0x0003FFFF


extern const size_t POOLREGIONSIZE;
extern const size_t POOLPOSITIONSIZE;


long long getSystemTime();


#endif
