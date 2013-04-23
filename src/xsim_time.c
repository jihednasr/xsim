#include <xsim_time.h>
#include <xsim_error.h>
#include <xsim_topology.h>
#include <xsim_perf_measure.h>

#define DBG_HDR "xsim:time"
#ifdef XSIM_TIME_DEBUG
#define DEBUG
#endif /* XSIM_TIME_DEBUG */

#ifdef XSIM_TIME_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_TIME_HDEBUG */

#include <xsim_debug.h>

#define MIN(a, b)  ((a) < (b)) ? (a) : (b) 
#define MAX(a, b)  ((a) < (b)) ? (b) : (a)

uint32_t seq_save = 0;


sim_time xsim_time_iface_next_message_recv(xsim_iface_t *iface) 
{
	/* 
	 * No need to take the lock because:
	 * 	- only the simulator thread can remove an element 
	 * 	but we are the simulator thread. So we are not removing a msg element.
	 * 	- only the listener can add an element. But it doesn't create SEG_FAULT
	 * 	because no old link are set to NULL. And with a lock, we also don't read
	 * 	this new message because the lock prevent the listener to add the message.
	 */
	if (xsim_msg_list_is_empty(iface->node->recv_queue[iface->x_id])) {
		return MAX_SIM_TIME;
	} else {
		return (xsim_msg_list_first(iface->node->recv_queue[iface->x_id])->real_arrival_time);
	}
	
}


int xsim_compute_next_time(xsim_node_t *node, int stabilized, sim_time next_time_for_resource)
{
    int i = 0;
	xsim_iface_t **iface = node->iface;
    /* check if the resource is stabilized */
    if (stabilized > 0)
        return 0;

    /* check if there is no messages to send or it is not possible to send more */
    sim_time min_send_time = MAX_SIM_TIME;
    sim_time min_recv_time = MAX_SIM_TIME;
    for (i=0 ; i<node->nb_iface ; i++) {

        /* we have still messages to send */
        if ((iface[i]->msg_send) && (iface[i]->next_possible_send <= node->current_time))
            return -2;

        /* we have still messages to receive */
		if ((iface[i]->msg_recv) && 
				(xsim_time_iface_next_message_recv(iface[i]) <= node->current_time) &&
				(iface[i]->next_possible_recv <= node->current_time))
            return -3;

        /* update min */
        if ((iface[i]->msg_send) && (min_send_time > iface[i]->next_possible_send)) {
            min_send_time = iface[i]->next_possible_send;
        }

		/* 
		 * Use a lock in order to avoid we read an empty recv queue and
		 * then the listener put a message in it and update the min_table
		 * before we read the min_table.
		 */
		LOCK_NODE_RECV_QUEUE(node->recv_queue[i]);
		sim_time tmp = MAX(iface[i]->next_possible_recv, 
				MIN(xsim_time_iface_next_message_recv(iface[i]), iface[i]->min_table_recv));
		UNLOCK_NODE_RECV_QUEUE(node->recv_queue[i]);
		if (min_recv_time > tmp)
			min_recv_time = tmp;
    }

	sim_time last_time = node->current_time;

    /* we can update the time without risk */
	node->current_time = MIN(MIN(min_recv_time, min_send_time), next_time_for_resource);

	if (last_time != node->current_time) {
		if (last_time > node->current_time) {
			EMSG("<%d> Error: node goes back in the past !!\n",
					node->node_id);
            node->current_time = last_time;
		}
		DMSG("<%d/%d> COMPUTE NEXT TIME: next_poss_recv %llut ; next_recv %llut ; min_table %llut\n",
				node->node_id, 0, iface[0]->next_possible_recv,
				xsim_time_iface_next_message_recv(iface[0]), iface[0]->min_table_recv);
		DMSG("<%d/%d> COMPUTE NEXT TIME: min_recv_time %llut ; min_send_time %llut ; next_time_for_resource %llut\n",
				node->node_id, 0, min_recv_time, min_send_time, next_time_for_resource);
		return 1;
	} else {
		return -4;
	}
}


int xsim_update_local_time_if_possible(xsim_node_t *node, int stabilized, sim_time next_time_for_resource)
{
	return xsim_compute_next_time(node, stabilized, next_time_for_resource);
}

int already_ask            = 0;
int info_time_already_send = 0;
int ask_our_time_info      = 0;
int xsim_update_local_time_if_possible_else_ask_time(xsim_node_t *node, int stabilized, sim_time next_time_for_resource)
{
	int ret = xsim_compute_next_time(node, stabilized, next_time_for_resource);

	if (!(already_ask) && ((ret == -4) || (ret == -3))) {
		for (int i=0 ; i<node->nb_iface ; i++) {
			if (node->iface[i]->min_table_recv <= node->current_time) {
				xsim_msg_t *msg = xsim_msg_new();
				msg->size = 0;
				msg->type = xsim_need_time_info;
				xsim_targets_clearall(&msg->targets);
				xsim_iface_send(node->iface[i], msg);
				DMSG("<%d/%d> (%llu t) Send a request for time information\n",
						node->node_id, i, node->current_time);
				already_ask = 1;
				break;
			}
		}
	} else if (ret == 1) {
		already_ask            = 0;
		info_time_already_send = 0;
		DMSG("<%d> NEW TIME: %llu t\n",
				node->node_id, node->current_time);
	}
	return ret;
}

sim_time xsim_time_min_table(sim_time *table, xsim_node_state_t *state, int size, int dest) 
{
    sim_time res = MAX_SIM_TIME;
    for (int i=0 ; i<size ; i++) {
        if (state[i] == simulating) {
            sim_time next_recv_time = table[i] + xsim_topology_travel_time(i, dest) + time_window;
            if (next_recv_time < res)
                res = next_recv_time;
        }
    }
    return res;
}

int  xsim_time_update(xsim_node_t *node, xsim_msg_t *msg)
{
	int i = 0;
	
    if ((msg->type != xsim_has_payload) && (msg->size != 0)) 
        EMSG("There is message with != 0 but should be 0: type %d, size: %d.\n", msg->type, msg->size);

	switch (msg->type) {
	case xsim_end_of_simulation:
        node->iface[msg->x_id]->state[msg->src_id] = not_simulating;
        break;

	case xsim_need_time_info:
		DMSG("<%d/%d> (%llu t) Receive a request for time information from node %d.\n", 
				node->node_id, msg->x_id, node->current_time, msg->src_id);
		if (!info_time_already_send) {
            ask_our_time_info = 1;
		}
		break;

	default:
		break;
	}


    for (i=0 ; i<msg->x_id ; i++) {
        if (node->iface[i]->table[msg->src_id] < msg->stamp_time) {
            LOCK_LISTENER_RECV_QUEUE(node->recv_queue[i]);
            node->iface[i]->table[msg->src_id] = msg->stamp_time;
			node->iface[i]->min_table_recv = 
				xsim_time_min_table(node->iface[i]->table, node->iface[i]->state, 
						node->nb_nodes, node->node_id);
            UNLOCK_LISTENER_RECV_QUEUE(node->recv_queue[i]);
        }
    }

	/* 
	 * the node which send this message can not send an other message 
	 * on the same interface before msg->stamp_time + msg->size
	 */
	sim_time tmp = msg->stamp_time + msg->size;
	if (node->iface[msg->x_id]->table[msg->src_id] < tmp) {
        LOCK_LISTENER_RECV_QUEUE(node->recv_queue[msg->x_id]);
		node->iface[msg->x_id]->table[msg->src_id] = tmp; 
		node->iface[msg->x_id]->min_table_recv = 
			xsim_time_min_table(node->iface[msg->x_id]->table, 
					node->iface[msg->x_id]->state, node->nb_nodes, node->node_id);
		DMSG("<%d> (%llu t) New min time for time table: %llut ; msg recv from %"PRIu32" with size %d; null_message: %d\n", 
				node->node_id, node->current_time, node->iface[msg->x_id]->min_table_recv, 
				msg->src_id, msg->size, msg->type);
        UNLOCK_LISTENER_RECV_QUEUE(node->recv_queue[msg->x_id]);
	}

    for (i=msg->x_id+1 ; i<node->nb_iface ; i++) {
        if (node->iface[i]->table[msg->src_id] < msg->stamp_time) {
            LOCK_LISTENER_RECV_QUEUE(node->recv_queue[i]);
            node->iface[i]->table[msg->src_id] = msg->stamp_time;
			node->iface[i]->min_table_recv = 
				xsim_time_min_table(node->iface[i]->table, node->iface[i]->state, 
						node->nb_nodes, node->node_id);
            UNLOCK_LISTENER_RECV_QUEUE(node->recv_queue[i]);
        }
    }
	
	return XSIM_SUCCESS;
}


sim_time xsim_time_due_time(xsim_iface_t *iface, xsim_msg_t *msg)
{
	sim_time res = msg->stamp_time;
	if (msg->stamp_time != MAX_SIM_TIME) {
        /* arrival time of the first flit */
		res += xsim_topology_travel_time(msg->src_id, iface->node->node_id);
	}
	if (res <= msg->stamp_time) {
		EMSG("<%d> Error: overflow when in the computation of the arrival time of a message from <%d> \n",
				iface->node->node_id, msg->src_id);
	}
	return res;
}


