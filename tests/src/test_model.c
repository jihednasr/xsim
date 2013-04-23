#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include <xsim.h>
#include <xsim_node.h>
#include <xsim_iface.h>
#include <xsim_targets.h>
#include <xsim_time.h>
#include <xsim_topology.h>


#define DBG_HDR "xsim:test_model"
#ifdef XSIM_MODEL_DEBUG
#define DEBUG
#endif

#ifdef XSIM_MODEL_HDEBUG
#define HUGE_DEBUG
#endif

#include <xsim_debug.h>

#include <xsim_time_model.h>

#define PROBABILITY_TO_SEND_A_MESSAGE 	10   /* in percentage to send the message */
#define POSSIBLE_JUMP					20
#define WORK_FOR_RESOURCE               5

int resource_handle_msg(xsim_node_t *node, xsim_msg_t *msg, xsim_iface_t *iface)
{
	int node_id = iface->node->node_id;

	if (msg->type != xsim_has_payload) {
		EMSG("[%d/%d] (%03"PRIu64" t) Got an error on received message: not a message with payload !!!\n", 
				node_id, msg->x_id, node->current_time);
		return CONTINUE_SIMULATION;
	}
	if ( (msg->due_time > node->current_time) || (msg->real_arrival_time != node->current_time) ){
		EMSG("[%d/%d] (%03"PRIu64" t) Got an error on received message: TIME error !!!\n", 
				node_id, msg->x_id, node->current_time);
		return CONTINUE_SIMULATION;
	}

	DMSG("[%d/%d] (%03"PRIu64" t) GOT A MESSAGE FROM %d. \n",
			node_id, msg->x_id, node->current_time, msg->src_id);
	DMSG("[%d/%d] sent time: %03"PRIu64" t ; due_time: %03"PRIu64" t ; arrival_time: %03"PRIu64" t ; size_message: %03" PRIu32 " flits \n",
			node_id, msg->x_id, msg->stamp_time, msg->due_time, msg->real_arrival_time, msg->size);

	return CONTINUE_SIMULATION;
}


int work = WORK_FOR_RESOURCE;
int resource_simulation(xsim_node_t *node, xsim_msg_list_t **NIC_msg_list_output, 
		sim_time *next_time_for_resource)
{
	if (work > 0) {
		//HMSG("[%d] work !\n", node_id);
		if (rand()%100 < PROBABILITY_TO_SEND_A_MESSAGE) {
			xsim_msg_t *msg = xsim_msg_new();

			int target;
			do {
				target = rand()%(node->nb_nodes);
			} while (target == (int) node->node_id);
			xsim_targets_set(&msg->targets, target);

			msg->stamp_time = node->current_time;
			memset(msg->payload, 1, sizeof(uint8_t)*XSIM_MAX_PAYLOAD);
			msg->size = (rand() % XSIM_MAX_PAYLOAD) + 1;
			msg->type = xsim_has_payload;
			int send_iface = rand() % node->nb_iface;
			msg->x_id = send_iface;

			HMSG("[%d] (%03llu t) try to send a message on iface %d to node %d with size %"PRIu32"!\n", 
					node->node_id, node->current_time, send_iface, target, msg->size);

			xsim_msg_list_add(NIC_msg_list_output[send_iface], msg);

			node->iface[send_iface]->msg_send = 1;
		}
		if (work == 1) {
			*next_time_for_resource = node->current_time + (rand()%(POSSIBLE_JUMP-1)) + 10;
			HMSG("[%d] Next time for resource: %llu t\n",
					node->node_id, *next_time_for_resource);
		}
		work--;
	}
	return work;
}

void resource_wake_up()
{
	work = WORK_FOR_RESOURCE;
	return;
}

void resource_init_simulation(xsim_node_t *node __attribute__((__unused__)), xsim_iface_t **iface __attribute__((__unused__)), int nb_n __attribute__((__unused__)) )
{
	return;
}

void resource_end_simulation(xsim_node_t *node __attribute__((__unused__)), xsim_iface_t **iface __attribute__((__unused__)), int nb_nodes __attribute__((__unused__)) )
{
	return;
}

int sc_son_body_model1(xsim_t *xsim __attribute__((__unused__)), 
        int node_id __attribute__((__unused__)), 
        sim_time limit_simulation_time __attribute__((__unused__)), 
        const char *output_measure __attribute__((__unused__)))
{
    fprintf(stderr, "The time model called is not implemented in this programme.\nDo not use -m 6");
    return 0;
}
