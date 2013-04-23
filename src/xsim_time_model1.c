/*
 * Update the time when it is possible
 * but send a null message in order to inform the other nodes about
 * our local time only when they ask it and they do not know already
 * our local time.
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
#include <xsim_perf_measure.h>
#include <xsim_performance_output.h>
#include <xsim_time_model_common.h>
#include <xsim_time_model.h>
#include <xsim_performance_evaluation.h>


#define DBG_HDR "xsim:time_model1"
#ifdef XSIM_TIME_MODEL1_DEBUG
#define DEBUG
#endif /* XSIM_TIME_MODEL1_DEBUG */

#ifdef XSIM_TIME_MODEL1_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_TIME_MODEL1_HDEBUG */

#include <xsim_debug.h>


int son_body_model1(xsim_t *xsim, int node_id, sim_time limit_simulation_time, 
        const char *output_measure)
{
    xsim_node_t      *node                   = NULL;
    xsim_msg_list_t **to_send                = NULL;
    sim_time          next_time_for_resource = 0;
    int               stabilized             = 0;
    xsim_msg_t       *msg_time               = NULL;


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

        if (ask_our_time_info && !info_time_already_send) {
            msg_time        = xsim_msg_new();
            msg_time->type  = xsim_null_msg;
            msg_time->size  = 0;
            xsim_targets_clearall(&msg_time->targets);
            xsim_iface_send(node->iface[0], msg_time);
            DMSG("<%d/%d> (%"PRIu64" t) Answer to a time info request.\n",
                    node->node_id, 0, node->current_time);
            ask_our_time_info = 0;
        }

        if (xsim_update_local_time_if_possible_else_ask_time(node, stabilized, next_time_for_resource) == 1) {
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



