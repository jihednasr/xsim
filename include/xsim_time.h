#ifndef _XSIM_TIME_H_
#define _XSIM_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include <xsim_time_type.h>
#include <xsim_iface.h>
#include <xsim_node.h>

/*
 * Return the reception time of the next message to be handle
 * by the simulator node and already received.
 * If there is no message, return MAX_SIM_TIME;
 */
sim_time 	xsim_time_iface_next_message_recv(xsim_iface_t *iface);


/*
 * Update the current time of node if the node has nothing more to do 
 * in the current time.
 * Return 1 if the time was updated
 * <= 0 if the time was not updated.
 */
int 		xsim_update_local_time_if_possible(xsim_node_t *node, int stabilized, sim_time next_time_for_resource);

/*
 * Update the current time of node if the node has nothing more to do 
 * in the current time.
 * If the node can not go further in the future because of a lack of information
 * about the other nodes, it asks them about there local time.
 * Return 1 if the time was updated
 * <= 0 if the time was not updated.
 */
int         xsim_update_local_time_if_possible_else_ask_time(xsim_node_t *node, int stabilized, sim_time next_time_for_resource);

/* 
 * Update the simulation time known by the time table 
 * about the time the next message from msg->src_id node can arrive.
 */
int  		xsim_time_update(xsim_node_t *node, xsim_msg_t *msg);

/* 
 * Compute the simulation time when the first bit of the msg should arrive 
 * to the destination node
 * if there is no other message in the network. 
 */
sim_time    xsim_time_due_time(xsim_iface_t *iface, xsim_msg_t *msg);
extern int info_time_already_send;
extern int ask_our_time_info;
/* 
 * This both variable are used by the listener and the node 
 * but are not protected with lock or CAS when used.
 * Normally it is not a problem in their currently usage.
 */


#ifdef __cplusplus
}
#endif

#endif
