/* __HEADER_HERE__ */

#include <pthread.h>
#include <xsim_sync.h>
#include <xsim_error.h>

#define DBG_HDR "xsim:sync"
#ifdef XSIM_SYNC_DEBUG
#define DEBUG
#endif /* XSIM_SYNC_DEBUG */

#ifdef XSIM_SYNC_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_SYNC_HDEBUG */

#include <xsim_debug.h>

int
xsim_barrier_init(xsim_barrier_t *barrier, int needed) {

  pthread_mutexattr_t mattr;
  pthread_condattr_t  cattr;

  barrier->count   = needed;
  barrier->current = 0;

  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, 1);
  pthread_mutex_init(&barrier->lock, &mattr);

  pthread_condattr_init(&cattr);
  pthread_condattr_setpshared(&cattr, 1);
  pthread_cond_init(&barrier->cond, &cattr);
  
  return XSIM_SUCCESS;
}

int
xsim_barrier_fini(xsim_barrier_t *barrier) {

  pthread_mutex_destroy(&barrier->lock);
  pthread_cond_destroy(&barrier->cond);

  return XSIM_SUCCESS;
}

int
xsim_barrier_wait(xsim_barrier_t *barrier) {

  pthread_mutex_lock(&barrier->lock);
  barrier->current++;
  if (barrier->current == barrier->count) {
    barrier->current = 0;
    pthread_cond_broadcast(&barrier->cond);
  } else {
    pthread_cond_wait(&barrier->cond,&barrier->lock);
  }
  pthread_mutex_unlock(&barrier->lock);

  return XSIM_SUCCESS;

}


int
xsim_cond_init(xsim_cond_t *cond) {

  pthread_mutexattr_t mattr;
  pthread_condattr_t  cattr;

  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, 1);
  pthread_mutex_init(&cond->lock, &mattr);

  pthread_condattr_init(&cattr);
  pthread_condattr_setpshared(&cattr, 1);
  pthread_cond_init(&cond->cond, &cattr);

  return XSIM_SUCCESS;
}

int xsim_cond_lock(xsim_cond_t *cond) {

  pthread_mutex_lock(&cond->lock);

  return XSIM_SUCCESS;
}

int xsim_cond_wait(xsim_cond_t *cond) {

  pthread_cond_wait(&cond->cond,&cond->lock);

  return XSIM_SUCCESS;
}

int xsim_cond_unlock(xsim_cond_t *cond) {

  pthread_mutex_unlock(&cond->lock);

  return XSIM_SUCCESS;
}


int xsim_cond_sign(xsim_cond_t *cond) {

  pthread_cond_broadcast(&cond->cond);

  return XSIM_SUCCESS;
}

int xsim_cond_fini(xsim_cond_t *cond) {

  pthread_mutex_destroy(&cond->lock);
  pthread_cond_destroy(&cond->cond);

  return XSIM_SUCCESS;
}
