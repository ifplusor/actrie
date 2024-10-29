/**
 * config.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_CONFIG_H__
#define __ACTRIE_CONFIG_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _segarray_config_ {
  size_t seg_blen;
  size_t region_size;
} segarray_config_s, *segarray_config_t;

segarray_config_s hint_segarray(size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_CONFIG_H__
