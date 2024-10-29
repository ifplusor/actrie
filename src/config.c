/**
 * matcher.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "actrie/config.h"

#include <stdio.h>

segarray_config_s hint_segarray(size_t len) {
  segarray_config_s config = {.seg_blen = 0};

  size_t hint = len >> 3;
  while (hint != 0) {
    hint >>= 1;
    config.seg_blen++;
  }
  if (config.seg_blen < 10) {
    config.seg_blen = 10;
  }

  config.region_size = (len >> config.seg_blen) + 1;
  if (config.region_size < 8) {
    config.region_size = 8;
  }

  fprintf(stderr, "seg_blen: %zu, region_size: %zu\n", config.seg_blen, config.region_size);

  return config;
}
