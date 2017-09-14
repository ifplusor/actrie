//
// Created by james on 6/16/17.
//

#include <sys/timeb.h>
#include <common.h>

const size_t POOL_REGION_SIZE = REGION_SIZE;
const size_t POOL_POSITION_SIZE = POSITION_MASK + 1;

long long system_millisecond() {
  struct timeb t;
  ftime(&t);
  return t.time * 1000 + t.millitm;
}

