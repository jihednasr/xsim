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
#include <xsim_time_model_common.h>
#include <xsim_performance_evaluation.h>
#include <poisson_law_generator.h>


#define DBG_HDR "xsim:test_model"
#ifdef XSIM_MODEL_DEBUG
#define DEBUG
#endif

#ifdef XSIM_MODEL_HDEBUG
#define HUGE_DEBUG
#endif

#include <xsim_debug.h>

#include <xsim_time_model.h>

#define LAMBDA                          40
#define MEAN_PACKET_SIZE                20
#define WORK_FOR_RESOURCE               1
#define TIME_PERIOD          100

xsim_init_struct_t is;

void sc_son_body_model1(xsim_t *xsim, int node_id);

int main(int argc, char** argv) {
    read_argument(argc, argv, &is);

    void (**simulation)(xsim_t*, int) = (void(**)(xsim_t*, int)) malloc(sizeof(void(**)(xsim_t*, int)) * is.nb_nodes);
    for (int i=0 ; i<is.nb_nodes ; i++) {
        simulation[i] = sc_son_body_model1;
    }
    xsim_spawner(simulation, is.nb_nodes, is.nb_ifaces, is.topology);

    free(simulation);
    return 0;
}


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

	DMSG("[%d/%d] (%03"PRIu64" t) GOT A MESSAGE FROM %d: sent time: %03"PRIu64" t ; due_time: %03"PRIu64" t ; arrival_time: %03"PRIu64" t ; size_message: %03" PRIu32 " flits \n",
			node_id, msg->x_id, node->current_time, msg->src_id,
			msg->stamp_time, msg->due_time, msg->real_arrival_time, msg->size);

	return CONTINUE_SIMULATION;
}


int work = WORK_FOR_RESOURCE;
int resource_simulation(xsim_node_t *node, xsim_msg_list_t **NIC_msg_list_output, 
		sim_time *next_time_for_resource)
{
	if (work > 0) {
        xsim_msg_t *msg = xsim_msg_new();

        int target;
        do {
            target = rand()%(node->nb_nodes);
        } while (target == (int) node->node_id);
        xsim_targets_set(&msg->targets, target);

        msg->stamp_time = node->current_time;
        memset(msg->payload, 1, sizeof(uint8_t)*XSIM_MAX_PAYLOAD);
        msg->size = (rand() % MEAN_PACKET_SIZE) + 1;//XSIM_MAX_PAYLOAD) + 1;
        msg->type = xsim_has_payload;
        int send_iface = rand() % node->nb_iface;
        msg->x_id = send_iface;

        DMSG("[%d] (%03llu t) try to send a message on iface %d to node %d with size %"PRIu32"!\n", 
                node->node_id, (unsigned long long ) node->current_time, send_iface, target, msg->size);

        xsim_msg_list_add(NIC_msg_list_output[send_iface], msg);

        node->iface[send_iface]->msg_send = 1;

        sim_time jump = 1;
        do {
            jump = poisson_generator_knuth(LAMBDA);
        } while (jump <= 0);
        *next_time_for_resource = node->current_time + jump;
        HMSG("[%d] Next time for resource: %llu t\n",
                node->node_id, *next_time_for_resource);

        work = 0;
    }
    return work;
}

void resource_wake_up()
{
	work = WORK_FOR_RESOURCE;
	return;
}

void sc_son_body_model1(xsim_t *xsim, int node_id)
{
    /* Initialize xsim */
    xsim_node_t      *node                   = NULL;
    xsim_msg_list_t **to_send                = NULL;
    sim_time          next_time_for_resource = 0;
    sim_time          last_time_broadcast    = 0;
    int               stabilized             = 0;

	xsim_time_model_init_son_body(xsim, node_id, &node, &to_send);

    next_time_for_resource = is.simulation_time;

    /* global measure */
    xsim_perf_begin_global_measure(global_processus_time, CLOCK_PROC);
    xsim_perf_begin_global_measure(global_simulation_time, CLOCK_OTHERS);

	/*
	 * Body here
	 */
    while (node->current_time < (unsigned int) is.simulation_time) {

        /* simulate the components of the node */
        if (xsim_time_model_simulate_composant(node, to_send, &next_time_for_resource,
                   &stabilized) == END_OF_SIMULATION)
            goto end_of_simulation;

        /* update the time if possible */
        if (xsim_update_local_time_if_possible(node, stabilized, next_time_for_resource) == 1) {
            if ((last_time_broadcast + TIME_PERIOD >= node->current_time) || (last_time_broadcast == 0)) {
                broadcast(&node->msg_box->mark, node->iface[0], xsim_null_msg);
                last_time_broadcast = node->current_time;
            }
			if (node->current_time > next_time_for_resource) {
				EMSG("<%d> (%"PRIu64" t) Error: jump above the next time the resource should work: %"PRIu64"t\n",
						node->node_id, node->current_time, next_time_for_resource);
			}
			if (node->current_time == next_time_for_resource)
				resource_wake_up();
		} 
    }

end_of_simulation:
    /* global measure */
    xsim_perf_end_global_measure(global_processus_time, CLOCK_PROC);
    xsim_perf_end_global_measure(global_simulation_time, CLOCK_OTHERS);

    xsim_time_model_end_of_simulation(node, to_send, is.measures_output);
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


//int sc_son_body_model1(xsim_t *xsim __attribute__((__unused__)), 
//        int node_id __attribute__((__unused__)), 
//        sim_time limit_simulation_time __attribute__((__unused__)), 
//        const char *output_measure __attribute__((__unused__)))
//{
//    fprintf(stderr, "The time model called is not implemented in this programme.\nDo not use -m 6");
//    return 0;
//}

