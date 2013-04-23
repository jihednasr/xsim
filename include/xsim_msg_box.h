#ifndef _XSIM_MSG_BOX_H_
#define _XSIM_MSG_BOX_H_

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct xsim_msg_box xsim_msg_box_t;

#define XSIM_MSG_BUFFER_SIZE 65536 //32768 //16384 //8192 4096 2048//1024

#include <xsim_msg.h>
#include <xsim_sync.h>
#include <xsim_perf_config.h>
#include <xsim_garbage_list.h>

struct xsim_msg_box {
  xsim_barrier_t       init_barrier;
  xsim_cond_t          cond;

  xsim_targets_t       mark;
  //xsim_targets_t       read_mark;

  xsim_msg_list_t      msg_queue;
  xsim_msg_list_t      garbage_queue;

#ifdef BENCHMARK
  xsim_lock_t          benchmark_lock;
#endif
};

static inline int xsim_msg_box_size(void) {
  /* TODO: WARNING get rid of alignment */
  return (sizeof(xsim_msg_box_t) + XSIM_MSG_BUFFER_SIZE*sizeof(xsim_msg_list_elt_t));
}

int         xsim_msg_box_init(xsim_msg_box_t *box, int nb_nodes);
int         xsim_msg_box_node_init(xsim_msg_box_t *box, int node_id);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static inline void xsim_msg_box_add_empty(xsim_msg_box_t *box, xsim_msg_t *msg)
{
    xsim_garbage_list_add(&box->garbage_queue, msg, msg);
}
xsim_msg_t *xsim_msg_box_get_empty(xsim_msg_box_t *box);
int         xsim_msg_box_add_central_queue(xsim_msg_box_t *box, xsim_msg_t *msg);
	   
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

int         xsim_msg_box_node_fini(xsim_msg_box_t *box);
int         xsim_msg_box_fini(xsim_msg_box_t *box);

#ifdef __cplusplus
}
#endif 
#endif 
