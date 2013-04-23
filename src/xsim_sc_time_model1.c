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


#define DBG_HDR "xsim:time_model1"
#ifdef XSIM_SC_TIME_MODEL1_DEBUG
#define DEBUG
#endif /* XSIM_TIME_MODEL1_DEBUG */

#ifdef XSIM_SC_TIME_MODEL1_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_TIME_MODEL1_HDEBUG */

#include <xsim_debug.h>


int sc_son_body_model1(xsim_t *xsim, int node_id, sim_time limit_simulation_time, 
        const char *output_measure)
{
    /* Initialize xsim */
    xsim_node_t      *node                   = NULL;
    xsim_msg_list_t **to_send                = NULL; /* Not used in this case */

	xsim_time_model_init_son_body(xsim, node_id, &node, &to_send);

    /* global measure */
    xsim_perf_begin_global_measure(global_processus_time, CLOCK_PROC);
    xsim_perf_begin_global_measure(global_simulation_time, CLOCK_OTHERS);

    //printf("Run the simulation.\n");
    xsim_sc_node_main(is.limit_simulation_time, node);

    /* global measure */
    xsim_perf_end_global_measure(global_processus_time, CLOCK_PROC);
    xsim_perf_end_global_measure(global_simulation_time, CLOCK_OTHERS);

    xsim_time_model_end_of_simulation(node, to_send, is.measures_output);
    return; 
}



