/*
 * Update the time when it is possible
 * and send a null message in order to inform the other nodes about
 * our local time each time the node updates its local time.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sched.h>

#include <xsim.h>
#include <xsim_targets.h>
#include <xsim_time.h>
#include <xsim_topology.h>
#include <xsim_time_model_common.h>
#include <xsim_time_model.h>
#include <xsim_performance_evaluation.h>


#define DBG_HDR "xsim:time_model4"
#ifdef XSIM_TIME_MODEL4_DEBUG
#define DEBUG
#endif /* XSIM_TIME_MODEL4_DEBUG */

#ifdef XSIM_TIME_MODEL4_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_TIME_MODEL4_HDEBUG */

#include <xsim_debug.h>


int son_body_model4(xsim_t *xsim, int node_id, sim_time limit_simulation_time, 
        const char *output_measure)
{
    xsim_node_t      *node                   = NULL;
    xsim_msg_list_t **to_send                = NULL;
    sim_time          next_time_for_resource = 0;
    int               stabilized             = 0;

	xsim_time_model_init_son_body(xsim, node_id, &node, &to_send);

    next_time_for_resource = limit_simulation_time;

    /* global measure */
    xsim_perf_begin_global_measure(global_processus_time, CLOCK_PROC);
    xsim_perf_begin_global_measure(global_simulation_time, CLOCK_OTHERS);

	/*
	 * Body here
	 */
    while (node->current_time < limit_simulation_time) {

        /* simulate the components of the node */
        if (xsim_time_model_simulate_composant(node, to_send, &next_time_for_resource,
                   &stabilized) == END_OF_SIMULATION)
            goto end_of_simulation;

        /* update the time if possible */
        if (xsim_update_local_time_if_possible(node, stabilized, next_time_for_resource) == 1) {
			broadcast(&node->msg_box->mark, node->iface[0], xsim_null_msg);
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

    return xsim_time_model_end_of_simulation(node, to_send, output_measure);
}



