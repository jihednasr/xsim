#ifndef _XSIM_TARGETS_H_
#define _XSIM_TARGETS_H_


#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif 

/*
 * First version is 64 bit .. larger later
 */
typedef uint64_t xsim_targets_t;

#define XSIM_TARGETS_PRINT_SIZE 19

static inline char *
xsim_targets_print(xsim_targets_t *targets) {

  char *buf = (char*) malloc(XSIM_TARGETS_PRINT_SIZE);

  //sprintf(buf,"0x%016"PRIx64, *targets);
  sprintf(buf,"0x%016lld", (unsigned long long)(*targets));

  return buf;
}

static inline void
xsim_targets_copy(xsim_targets_t *dst, xsim_targets_t *src) {

  *dst = *src;
  return;

}

static inline void
xsim_targets_set(xsim_targets_t *targets, int target) {
  
  *targets |= (1ULL << target);
  
  return;
}

static inline void
xsim_targets_clear(xsim_targets_t *targets, int target) {
  
  *targets &= ~(1ULL << target);
  
  return;
}

static inline void
xsim_targets_clearall(xsim_targets_t *targets) {
  
  *targets = 0;
  
  return;
}

static inline int
xsim_targets_is_set(xsim_targets_t *targets, int target) {
  
  return (*targets & ((xsim_targets_t)1 << target));
}

static inline void
xsim_targets_atomic_set(xsim_targets_t *targets, int target) {
  
  xsim_targets_t old, new_t;

  do{
    old = *targets;
    new_t = (old | ((xsim_targets_t)1 << target));
  }while(!__sync_bool_compare_and_swap(targets, old, new_t));

  return;
}

static inline void
xsim_targets_atomic_clear(xsim_targets_t *targets, int target) {
  
  xsim_targets_t old, new_t;

  do{
    old = *targets;
    new_t = (old & ~((xsim_targets_t)1 << target));
  }while(!__sync_bool_compare_and_swap(targets, old, new_t));

  return;
}



#ifdef __cplusplus
}
#endif
#endif /* _XSIM_TARGETS_H_ */
