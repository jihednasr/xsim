#ifndef _XSIM_H_
#define _XSIM_H_

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct xsim xsim_t;

#include <stdint.h>
#include <xsim_msg_box.h>

struct xsim {

  uint8_t          nb_x;
  uint16_t         nb_nodes;
  int              shm_id;
  void            *shm_addr;

  xsim_msg_box_t  *msg_box;

  char            *topology;
};

xsim_t *xsim_alloc(uint8_t nb_x, uint16_t nb_nodes, const char *topology);
int     xsim_init(xsim_t *xsim);
int     xsim_fini(xsim_t *xsim);
int     xsim_free(xsim_t *xsim);

#ifdef __cplusplus
}
#endif 
#endif /* _XSIM_H_ */
