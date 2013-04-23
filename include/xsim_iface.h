#ifndef _XSIM_IFACE_H_
#define _XSIM_IFACE_H_

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct xsim_iface       xsim_iface_t;

#include <xsim_node.h>

enum xsim_node_state {
    simulating,
    not_simulating
};
typedef enum   xsim_node_state  xsim_node_state_t;

struct xsim_iface {

    uint8_t             x_id;
    xsim_node_t        *node;

    /* time management */
    sim_time           *table;
    sim_time            min_table_recv;
    xsim_node_state_t  *state;
    sim_time            next_possible_send;
    sim_time            next_possible_recv;
    int                 msg_recv;
    unsigned char       msg_send;

};

xsim_iface_t *xsim_iface_new(xsim_node_t *node, int x_id);
void          xsim_iface_free(xsim_iface_t *iface);
void          xsim_iface_clean(xsim_iface_t *iface);

int           xsim_iface_send(xsim_iface_t *iface, xsim_msg_t *msg);
xsim_msg_t   *xsim_iface_recv(xsim_iface_t *iface);

#ifdef __cplusplus
}
#endif
#endif /* _XSIM_IFACE_H_ */
