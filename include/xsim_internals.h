#ifndef _XSIM_INTERNALS_H_
#define _XSIM_INTERNALS_H_

typedef struct xsim_internals xsim_internals_t;

#include <xsim.h>

struct xsim_internals {

  int             shm_id;
  xsim_msg_box_t *box;

};

extern xsim_internals_t xsim_int;

#endif /* _XSIM_INTERNALS_H_ */
