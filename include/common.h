//
// Created by james on 6/16/17.
//

#ifndef _MATCH_COMMON_H_
#define _MATCH_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <stdbool.h>
#endif

//#define DEBUG 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __bool_true_false_are_defined
#define bool int
#define true 1
#define false 0
#endif /* __bool_true_false_are_defined */

#if defined(_WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

#define MAX_LINE_SIZE 1024*1024

#define REGION_SIZE 0x00001000
#define REGION_OFFSET 18
#define POSITION_MASK 0x0003FFFF

extern const size_t POOL_REGION_SIZE;
extern const size_t POOL_POSITION_SIZE;

long long system_millisecond();

#define offset_of(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({ \
            const typeof(((type *)0)->member) *__mptr = (ptr); \
            (type *)((char*)__mptr - offset_of(type, member)); })

char *strdup(const char *s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_COMMON_H_ */
