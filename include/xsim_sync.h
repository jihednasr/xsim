#ifndef _XSIM_SYNC_H_
#define _XSIM_SYNC_H_

typedef struct xsim_cond      xsim_cond_t;
typedef struct xsim_barrier   xsim_barrier_t;

#include <pthread.h>

typedef pthread_mutex_t  xsim_lock_t;

struct xsim_cond {
  pthread_mutex_t lock;
  pthread_cond_t  cond;
};

struct xsim_barrier {
  int             count;
  int             current;
  pthread_mutex_t lock;
  pthread_cond_t  cond;
};

static inline int xsim_lock_init(xsim_lock_t *lock) {
  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, 1);
  return pthread_mutex_init(lock, &mattr);
}

static inline int xsim_lock_lock(xsim_lock_t *lock) {
  return pthread_mutex_lock(lock);
}

static inline int xsim_lock_unlock(xsim_lock_t *lock) {
  return pthread_mutex_unlock(lock);
}

static inline int xsim_lock_fini(xsim_lock_t *lock) {
  return pthread_mutex_destroy(lock);
}

int xsim_barrier_init(xsim_barrier_t *barrier, int needed);
int xsim_barrier_fini(xsim_barrier_t *barrier);
int xsim_barrier_wait(xsim_barrier_t *barrier);

int xsim_local_barrier_init(xsim_barrier_t *barrier, int needed);
int xsim_local_barrier_fini(xsim_barrier_t *barrier);
int xsim_local_barrier_wait(xsim_barrier_t *barrier);


int xsim_cond_init(xsim_cond_t *cond);
int xsim_cond_lock(xsim_cond_t *cond);
int xsim_cond_wait(xsim_cond_t *cond);
int xsim_cond_unlock(xsim_cond_t *cond);
int xsim_cond_sign(xsim_cond_t *cond);
int xsim_cond_fini(xsim_cond_t *cond);

#endif /* _XSIM_SYNC_H_ */
