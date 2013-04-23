#ifndef _XSIM_NODE_H_
#define _XSIM_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct xsim_node xsim_node_t;

#include <pthread.h>

#include <xsim.h>
#include <xsim_msg.h>
#include <xsim_msg_box.h>
#include <xsim_iface.h>
#include <inttypes.h>
#include <xsim_FIFO_list.h>

#define LIST_STATE_STARTING 1
#define LIST_STATE_RUNNING  2
#define LIST_STATE_STOPPING 3
#define LIST_STATE_DEAD     4


struct xsim_node {
  
  uint32_t           node_id;
  uint16_t		     nb_nodes;
  int                nb_iface;
  uint32_t           seq_id;

  uint32_t          *seq_msg_table;

  xsim_iface_t     **iface;

  xsim_msg_box_t    *msg_box;
  xsim_msg_t        *current;

  xsim_msg_list_t **recv_queue; /* one per X   */
  xsim_FIFO_list_t *send_queue; /* one for all */

  pthread_t         listener;

  xsim_barrier_t    list_barrier;
  volatile int      list_state;

  sim_time          current_time;
};

/*
 * Public functions
 */
xsim_node_t *xsim_node_init(xsim_t *xsim, int node_id);
int          xsim_node_free(xsim_node_t *node);

/*
 * Internal functions
 */
int          xsim_node_send(xsim_node_t *node, uint8_t x_id, xsim_msg_t *msg);
/*
 * Need that the lock on the recv_queue of the interface
 * is taken before to call xsim_node_recv
 */
xsim_msg_t  *xsim_node_recv(xsim_node_t *node, uint8_t x_id);


#ifdef __cplusplus
}
#endif

#endif /* _XSIM_NODE_H_ */
