#ifndef _XSIM_LISTENER_H_
#define _XSIM_LISTENER_H_


void *xsim_listener(void *arg);

void xsim_listener_post_msg(xsim_node_t *node);
void xsim_listener_read_msg(xsim_node_t *node);
void xsim_listener_free_msg(xsim_node_t *node);

int  xsim_listener_node_incoming_msg(xsim_node_t *node, xsim_msg_t *msg);

#endif
