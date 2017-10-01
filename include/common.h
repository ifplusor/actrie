//
// Created by james on 6/16/17.
//
/*
 * description:
 *   API 语义约定:
 *     1. alloc/free            分配、释放对象内存
 *     2. init/clean/reset      初始化、释放缓存、重置状态
 *     3. construct/destruct    构造、析构对象
 *     4. retain/release        增加、减少对象引用
 */

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

#ifdef _WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

#if defined(_WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

typedef long long ll;
typedef unsigned long long ull;

#if !MAX_LINE_SIZE
#define MAX_LINE_SIZE (1024 * 1024)
#endif

#if !REGION_SIZE
#define REGION_SIZE 0x00001000
#endif

#if !REGION_OFFSET
#define REGION_OFFSET 18
#endif

#define POSITION_MASK ((0x0001LL << REGION_OFFSET) - 1)

extern const size_t POOL_REGION_SIZE;
extern const size_t POOL_POSITION_SIZE;

#define ROUND_UP_16(num) \
  (((num) & 0x000F) ? ((num) | 0x000F) + 1 : (num))

#define max(a, b) ((a) >= (b) ? (a) : (b))

extern const char *str_empty;

long long system_millisecond();

#define offset_of(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({ \
            const typeof(((type *)0)->member) *__mptr = (ptr); \
            (type *)((char*)__mptr - offset_of(type, member)); })

#ifndef _WIN32
char *strdup(const char *s);
#endif

typedef struct str_len {
  char *ptr;
  size_t len;
} strlen_s, *strlen_t;

typedef struct str_pos {
  size_t so, eo;
} strpos_s, *strpos_t;

typedef struct str_iter {
  unsigned char *ptr;
  size_t len, cur;
} striter_s, *striter_t;

bool striter_init(striter_t self, unsigned char *ptr, size_t len);
bool striter_reset(striter_t self);
int striter_next(striter_t self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_COMMON_H_ */
