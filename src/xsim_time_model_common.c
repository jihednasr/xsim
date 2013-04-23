#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sched.h>

#include <xsim.h>
#include <xsim_time.h>
#include <xsim_time_model_common.h>
#include <xsim_time_model.h>
#include <xsim_topology.h>
#include <xsim_perf_measure.h>
#include <xsim_performance_output.h>
#include <xsim_performance_deltaT.h>


#define DBG_HDR "xsim:time_model_common"
#ifdef XSIM_TIME_MODEL_COMMON_DEBUG
#define DEBUG
#endif /* XSIM_TIME_MODEL_COMMON_DEBUG */

#ifdef XSIM_TIME_MODEL_COMMON_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_TIME_MODEL_COMMON_HDEBUG */

#include <xsim_debug.h>

/* variable for the correctness tests */
sim_time *last_send      = NULL;
sim_time *last_theo_send = NULL;
sim_time *next_send      = NULL;

xsim_msg_t **last_msg = NULL;
/*************************************/
	
//
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/* Broadcast and message sent */

/* The payload of the messages sent with broadcast must not be taken into account */
void broadcast(xsim_targets_t *targets, xsim_iface_t *iface, xsim_msg_type_t type)
{
    if (type == xsim_has_payload) {
        EMSG("<%d> Error: broadcast not used in the expected wait.\n", iface->node->node_id);
    }
	/* the targets are not really useful because all the nodes will read the message */
	xsim_msg_t *msg = xsim_msg_new();
	xsim_targets_copy(&msg->targets, targets); 
	msg->type = type;
	msg->size = 0;
	switch (type) {
	case xsim_null_msg:
		DMSG("<%d> (%03"PRIu64" t) I will now send a NULL message !!!\n", 
				iface->node->node_id, iface->node->current_time);
		break;
	case xsim_end_of_simulation:
		DMSG("<%d> (%03"PRIu64" t) I will now send a message to signal I finished my simulation!!!\n", 
				iface->node->node_id, iface->node->current_time);
		break;
	default:
		DMSG("<%d> (%03"PRIu64" t) broadcast not used in the expected way.\n",
				iface->node->node_id, iface->node->current_time);
		return;
	}
	xsim_iface_send(iface, msg);
}

void send_message(int dest, xsim_iface_t *iface, uint8_t *payload, uint32_t size)
{
    xsim_msg_t *msg = xsim_msg_new();

    xsim_targets_set(&msg->targets, dest); 
    if (size <= XSIM_MAX_PAYLOAD) {
        memcpy(msg->payload, payload, size);
    } else {
        EMSG("<%d> Error in send_message: size of the payload is too high for a message.\n",
                iface->node->node_id);
    }
    msg->size = size;
    msg->type = xsim_has_payload;

	DMSG("[%d] (%03"PRIu64" t) I will now send a message to %d!!!\n", 
			iface->node->node_id, iface->node->current_time, dest);
	xsim_iface_send(iface, msg);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/* Simulation of the components */

/* NIC reception */
int xsim_time_model_message_to_recv(xsim_node_t *node, int iface_nb, xsim_msg_t **msg_recv)
{
    xsim_iface_t *iface = node->iface[iface_nb];
    int node_id         = node->node_id;
    int res             = CONTINUE_SIMULATION;
    *msg_recv           = NULL;

    if (iface->msg_recv) {
        DMSG("<%d/%d> (%03"PRIu64" t) RECEIVE A MESSAGE!\n", 
                node_id, iface->x_id, node->current_time); 
        LOCK_NODE_RECV_QUEUE(node->recv_queue[iface_nb]);
        sim_time time_next_message = xsim_time_iface_next_message_recv(iface);
        if (time_next_message < node->current_time) {
            //EMSG("<%d/%d> (%"PRIu64" t) Time error: message in our past forgotten: real_arrival_time: %"PRIu64" t from %d. next_possible_recv: %lld, min_table_recv: %lld, arrival_time: %lld, min_table_recv_when_recv: %lld, last_wake_up_nic: %lld\n", 
            //        node_id, iface_nb, node->current_time, time_next_message, 
            //        xsim_msg_list_first(node->recv_queue[iface_nb])->src_id,
            //        (long long int)(iface->next_possible_recv),
            //        (long long int)(iface->min_table_recv),
            //        (long long int)(xsim_msg_list_first(node->recv_queue[iface_nb])->arrival_time),
            //        (long long int)(xsim_msg_list_first(node->recv_queue[iface_nb])->min_table_recv),
            //        (long long int)(iface->last_wake_up_nic));
            xsim_msg_update_list_time(node->recv_queue[iface_nb], node->current_time);
            time_next_message = xsim_time_iface_next_message_recv(iface);
        }

        if ((time_next_message >= iface->next_possible_recv) && 
                (time_next_message == node->current_time) &&
                (iface->min_table_recv >= node->current_time)) {

            xsim_msg_t *msg = xsim_iface_recv(iface);

            UNLOCK_NODE_RECV_QUEUE(node->recv_queue[iface_nb]);
            
            if (msg != NULL) {
                if (last_msg[msg->x_id] == NULL) {
                    last_msg[msg->x_id] = msg;
                } else {
                    test_correctness_recv(node, node_id, msg);
                    xsim_msg_free(last_msg[msg->x_id]);
                    last_msg[msg->x_id] = msg;
                }
                
                *msg_recv = msg;

            } else {
                EMSG("<%d/%d> Error: call xsim_iface_recv but no message to recv\n", 
                        node->node_id, iface_nb);
            }
        } else {
            UNLOCK_NODE_RECV_QUEUE(node->recv_queue[iface_nb]);
        }
        if (iface->msg_recv < 0) {
            EMSG("<%d/%d> Error: receive more message than realy received.\n",
                    node->node_id, iface_nb);
            iface->msg_recv = 0;
        }
    }
    return res;
}

/* NIC emission */
void xsim_time_model_message_to_send(xsim_node_t *node, int iface_nb, xsim_msg_list_t *to_send)
{
    xsim_iface_t *iface = node->iface[iface_nb];
    int node_id         = node->node_id;

    if (iface->msg_send) {
        xsim_msg_t* msg = xsim_msg_list_first(to_send);
        if ((node->current_time > iface->next_possible_send) && 
                (msg->stamp_time < node->current_time)) {
            EMSG("<%d/%d> Error: a message must be send earlier.\n",
                    node_id, iface->x_id);
        }

        if (msg->stamp_time <= node->current_time && 
                node->current_time >= iface->next_possible_send) {
            char *tar = xsim_targets_print(&msg->targets);
            DMSG("<%d/%d> (%03"PRIu64" t) SEND A MESSAGE TO NODE %s WITH SIZE %"PRIu32"!\n", 
                    node_id, iface->x_id, node->current_time, 
                    tar, msg->size);
            free(tar);

            xsim_msg_t *tmp   = xsim_msg_list_del(to_send);

            /* correctness for sent message */
            test_correctness_send(node, tmp, iface_nb);
            last_send[iface_nb]      = node->current_time;
            last_theo_send[iface_nb] = tmp->stamp_time;
            next_send[iface_nb]      = node->current_time + tmp->size;

            xsim_iface_send(iface, tmp);
        }
        if (xsim_msg_list_first(to_send) == NULL) {
            iface->msg_send = 0;
        }
    }
    return;
}

/* NICs and other components simulation */
int xsim_time_model_simulate_composant(xsim_node_t *node, xsim_msg_list_t **to_send, 
        sim_time *next_time_for_resource, int *stabilized)
{	
    int i   = 0;
    int res = CONTINUE_SIMULATION;
    xsim_msg_t *msg_tmp = NULL; 

    /* other components of the resource which could send messages*/
    *stabilized = resource_simulation(node, to_send, next_time_for_resource);

    /* execute the NICs */
    for (i = 0 ; i<node->nb_iface ; i++) {
        /* message to send ? */
        xsim_time_model_message_to_send(node, i, to_send[i]);

        /* message to recv ? */
        res = xsim_time_model_message_to_recv(node, i, &msg_tmp);
        if (msg_tmp != NULL) {
            xsim_perf_begin_global_measure(global_useful_time, CLOCK_OTHERS);
            res = resource_handle_msg(node, msg_tmp, node->iface[i]);
            xsim_perf_end_global_measure(global_useful_time, CLOCK_OTHERS);
            msg_tmp = NULL;
        }
    }
    return res;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/* Init and end for son_body */ 

void xsim_time_model_init_son_body(xsim_t *xsim, int node_id, xsim_node_t **node, xsim_msg_list_t ***to_send)
{
    int i=0; 
    DMSG("<%d> Init son body.\n", node_id);

    install_handler();
//    fprintf(stderr, "Test seg fault\n");
//    xsim_t *test = NULL;
//    test->nb_nodes = xsim->nb_nodes;

    //pid_t pid = syscall(SYS_getpid);
    //fprintf(stderr, "pid: %d\n", pid);
    //sleep(15);

#ifdef AFFINITY
	/* Set affinity */
	int cpu;
	if ((cpu = sched_getcpu()) < 0) {
		EMSG("<%d> Error when getting the CPU id of the running process\n", node_id);
	}
	DMSG("<%d> Run on %d\n", node_id, cpu);

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask) != 0) {
		EMSG("<%d> Got error when setting the affinity\n", node_id);
	}

    fprintf(stdout, "Node %d is on cpu %d.\n", node_id, sched_getcpu());
#endif

    /* init Rand */
    srand(time(NULL));
    srandom(time(NULL));

    /* Reinitialize the performance evaluation */
    xsim_perf_fini();

	/* init node + ifaces */
	*node  = xsim_node_init(xsim, node_id);

    *to_send = malloc(sizeof(xsim_msg_list_t*) * (*node)->nb_iface);
    for (i = 0 ; i<(*node)->nb_iface ; i++) {
        (*to_send)[i] = xsim_msg_list_new();
    }

	/* correctness for sent message */
	last_send      = malloc(sizeof(sim_time) * (*node)->nb_iface);
	last_theo_send = malloc(sizeof(sim_time) * (*node)->nb_iface);
	next_send      = malloc(sizeof(sim_time) * (*node)->nb_iface);
	memset(last_send,      0, sizeof(sim_time) * (*node)->nb_iface);
	memset(last_theo_send, 0, sizeof(sim_time) * (*node)->nb_iface);
	memset(next_send,      0, sizeof(sim_time) * (*node)->nb_iface);

	last_msg = malloc(sizeof(xsim_msg_t) * (*node)->nb_iface);
	for (int i=0 ; i<(*node)->nb_iface ; i++) {
		last_msg[i] = NULL;
	}

//	/* global measure */
//	xsim_perf_begin_global_measure(global_processus_time, CLOCK_PROC);
//	xsim_perf_begin_global_measure(global_simulation_time, CLOCK_OTHERS);

    DMSG("<%d> End init son.\n", node_id);
	return;
}

int xsim_time_model_end_of_simulation(xsim_node_t *node, xsim_msg_list_t **to_send, 
        const char *output_measure)
{
    int i       = 0;
    int node_id = node->node_id;

    /* signal t  all that I am at the end of my simulation */
    for (i=0 ; i<node->nb_iface ; i++) {
        broadcast(&node->msg_box->mark, node->iface[i], xsim_end_of_simulation);
    }

    ///* global measure */
    //xsim_perf_end_global_measure(global_processus_time, CLOCK_PROC);
    //xsim_perf_end_global_measure(global_simulation_time, CLOCK_OTHERS);

    DMSG("<%d> (%03"PRIu64" t) end of simulation\n", node_id, node->current_time);
    //fprintf(stderr, "<%d> end of simulation\n", node_id);

    /* free send lists of the NICs */
    for (i = 0 ; i<node->nb_iface ; i++) {
        xsim_msg_list_free(to_send[i]);
    }
    free(to_send);

	/* correctness for sent message */
    if (last_send != NULL)
        free(last_send);
    if (last_theo_send != NULL)
        free(last_theo_send);
    if (next_send != NULL)
        free(next_send);

	for (int i=0 ; i<node->nb_iface ; i++) {
		if (last_msg[i] != NULL)
			free(last_msg[i]);
	}
	free(last_msg);

    /* free node */
    xsim_node_free(node);
    
    //fprintf(stderr, "<%d> Will output measure.\n", node_id);
    xsim_perf_output(output_measure, node_id);
    xsim_perf_fini();

    uninstall_handler();
    return 0;

}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/* Tests for correctness of the messages order */

void test_correctness_send(xsim_node_t *node, xsim_msg_t *msg, int iface_id)
{
	int node_id = node->node_id;
	int x_id    = msg->x_id;
	if (x_id != iface_id) {
		EMSG("[%d/%d] Error: message not send on the right network.\n",
				node_id, x_id);
		return;
	}

	if (msg->stamp_time > node->current_time) {
		EMSG("[%d/%d] Error: a message is sent before it can in theory be sent.\n",
				node_id, x_id);
		return;
	}
	if (msg->stamp_time < last_theo_send[x_id]) {
		EMSG("[%d/%d] Error: a message should be sent before the last message send.\n",
				node_id, x_id);
		return;
	}
	if ((msg->stamp_time == last_theo_send[x_id]) ||
			(msg->stamp_time <= next_send[x_id])){
		if (node->current_time > next_send[x_id]) {
			EMSG("[%d/%d] Error: a message should be sent earlier.\n",
					node_id, x_id);
		} else if (node->current_time < next_send[x_id]) {
			EMSG("[%d/%d] Error: 2 messages sent are overlappping.\n",
					node_id, x_id);
		}
	} else { /* msg->stamp_time > last_theo_send[x_id]  &&  msg->stamp_time > next_send[x_id]*/
	   	if (msg->stamp_time != node->current_time) {
	   	/* msg->stamp_time < node->current_time */
			EMSG("[%d/%d] Error: this message should be sent at its stamp_time but was not.\n",
					node_id, x_id);
		}
	}
	return;
}


void test_correctness_recv(xsim_node_t* node, int node_id, xsim_msg_t *msg)
{
	uint8_t x_id = msg->x_id;
	if (msg->real_arrival_time != node->current_time) {
		EMSG("[%d/%d] Error: invalid set of arrival time.\n",
				node_id, x_id);
		return;
	}
	if (msg->due_time != msg->stamp_time + xsim_topology_travel_time(msg->src_id, node_id)) {
		EMSG("[%d/%d] Error: invalid computation of due_time.\n",
				node_id, x_id);
		return;
	}
	if (msg->due_time > msg->real_arrival_time) {
		EMSG("[%d/%d] Error: receive a message before it is possible in theory.\n",
				node_id, x_id);
		return;
	}

	if (msg->due_time + time_window < last_msg[x_id]->due_time) {
		EMSG("[%d/%d] Error: Message receive to late compared to other message.\n",
				node_id, x_id);
		return;
	}
    if (msg->real_arrival_time < last_msg[x_id]->real_arrival_time + last_msg[x_id]->size) {
        EMSG("[%d/%d] Error: 2 messages received are overlapping.\n",
                node_id, x_id);
        return;
    }

    /* can not check with time_window */
	//if ((msg->due_time == last_msg[x_id]->due_time) ||
	//		(msg->due_time <= last_msg[x_id]->real_arrival_time + last_msg[x_id]->size)) {
	//	if (msg->real_arrival_time > last_msg[x_id]->real_arrival_time + last_msg[x_id]->size) {
	//		EMSG("[%d/%d] Error: can receive a message earlier.\n",
	//			node_id, x_id);
    //        return;
	//	} 
	//} else { /* msg->due_time > last_msg[x_id]->due_time && msg->due_time > last_msg[x_id]->real_arrival_time + last_msg[x_id]->size */
    if (msg->due_time > last_msg[x_id]->real_arrival_time + last_msg[x_id]->size) { 
		//if (msg->due_time != msg->real_arrival_time) {
			/* msg->due_time < msg->real_arrival_time */
        if (msg->due_time + time_window < msg->real_arrival_time) {
			EMSG("[%d/%d] Error: a message should arrive at its due_time but did not.\n",
				node_id, x_id);
		}
		return;
	}
	return;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#define HAS_ARG             0x0001

/* Parser */
enum
{
    CMDLINE_OPTION_nb_nodes,
    CMDLINE_OPTION_nb_ifaces,
    CMDLINE_OPTION_topology,
    CMDLINE_OPTION_measures_output,
    CMDLINE_OPTION_gnuplot_output,
    CMDLINE_OPTION_simulation_time,
    CMDLINE_OPTION_help,
    CMDLINE_OPTION_null,
};

typedef struct
{
    const char      *name;
    int             flags;
    int             index;
    const char      *usage;
} cmdline_option;

const cmdline_option cmdline_options[] = 
{
    {"n",       HAS_ARG, CMDLINE_OPTION_nb_nodes,       "number of nodes"},
    {"i",       HAS_ARG, CMDLINE_OPTION_nb_ifaces,      "number of interfaces"},
    {"tp",      HAS_ARG, CMDLINE_OPTION_topology,       "topology file"},
    {"mo",      HAS_ARG, CMDLINE_OPTION_measures_output,"output file for the measure"},
    {"go",      HAS_ARG, CMDLINE_OPTION_gnuplot_output, "gnuplot output file"},
    {"time",    HAS_ARG, CMDLINE_OPTION_simulation_time,"maximum simulation time"},
    {"h",             0, CMDLINE_OPTION_help,           "print the help"},
    {"help",          0, CMDLINE_OPTION_help,           "print the help"},
    {NULL,            0, CMDLINE_OPTION_null,           NULL},
};

void print_help()
{
    printf("Option: \n");
    const cmdline_option *popt = cmdline_options;
    for (;;)
    {
        if (!popt->name)
            return;
        printf("-%s\t\t%s\n", popt->name, popt->usage);
        popt++;
    }
}

#define DEFAULT_OUTPUT              "output"

void read_argument(int argc, char **argv, xsim_init_struct_t *is)
{
	/* default value for the simulation */
	is->nb_nodes   = 2;
	is->nb_ifaces  = 1;
	is->topology   = NULL;
    is->measures_output = DEFAULT_OUTPUT;
    is->gnuplot_output = NULL;
    is->simulation_time = 100;

    int                 optind;
    const char          *r, *optarg;

    optind = 1;
    for (; ;)
    {
        if (optind >= argc)
            break;
        r = argv[optind];

        const cmdline_option *popt;

        optind++;
        /* Treat --foo the same as -foo.  */
        if (r[1] == '-')
            r++;
        popt = cmdline_options;
        for (;;)
        {
            if (!popt->name)
            {
                fprintf (stderr, "%s: invalid option -- '%s'\n", argv[0], r);
                exit (1);
            }
            if (!strcmp (popt->name, r + 1))
                break;
            popt++;
        }
        if (popt->flags & HAS_ARG)
        {
            if (optind >= argc)
            {
                fprintf (stderr, "%s: option '%s' requires an argument\n",
                        argv[0], r);
                exit (1);
            }
            optarg = argv[optind++];
        }
        else
            optarg = NULL;

        switch (popt->index)
        {
        case CMDLINE_OPTION_nb_nodes:
            is->nb_nodes = atoi(optarg);
            break;
        case CMDLINE_OPTION_nb_ifaces:
            is->nb_ifaces = atoi(optarg);
            break;
        case CMDLINE_OPTION_topology:
            is->topology = optarg;
            break;
        case CMDLINE_OPTION_measures_output:
            is->measures_output = optarg;
            break;
        case CMDLINE_OPTION_gnuplot_output:
            is->gnuplot_output = optarg;
            break;
        case CMDLINE_OPTION_simulation_time:
            is->simulation_time = atoi(optarg);
            break;
        case CMDLINE_OPTION_help:
            print_help();
            exit (0);
            break;
        }
    }

    return;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


int xsim_spawner(void (**simulation)(xsim_t*, int), int nb_nodes, int nb_ifaces, const char* topology)
{
    xsim_t  *xsim           = NULL;
    int      pid            = 0;
    int      status         = 0;
    int      spawned        = 0;
    int      terminated     = 0;
    int      ret            = 0;

    /* Compute deltaT */
    //xsim_evaluate_diff_time_between_proc();

    /* Initialise performance measures */
    xsim_perf_init();

    /*
     * Create the structure
     */
    xsim = xsim_alloc(nb_ifaces, nb_nodes, topology);

    /*
     * Setup comm
     */
    xsim_init(xsim);

    /*
     * Body here
     */
    while( spawned < nb_nodes) {
        pid = fork();
        switch(pid){
            case -1: /* error */
                goto on_fork_error;

            case 0:  /* son */
                //fprintf(stderr, "Child %d born\n", spawned);
                simulation[spawned](xsim, spawned);
                goto child_end;

            default: /* father */
                spawned++;
                break;
        }
    }

    do{
        ret = wait(&status);

        if( WEXITSTATUS(status) )
            fprintf(stderr, "Got an error\n");

        terminated++;
    }while(ret > 0); /* no problem: ID 0 is init :-) */
    terminated--; /* one for the last loop */

    if( (errno == ECHILD) && (terminated == spawned) ) {
        fprintf(stderr, "Got all my childs\n");
    } else {
        fprintf(stderr, "Something went wrong : handle it (%d / %d)\n", terminated, spawned);
    }
    xsim_fini(xsim);  
    xsim_perf_fini();

child_end:
    xsim_free(xsim);  
    return EXIT_SUCCESS;

    /*
     * Error handling
     */
on_fork_error:
    /* send a signal here */
    while( terminated < spawned ){
        wait(&status);

        if( WEXITSTATUS(status) )
            fprintf(stderr, "Got an error\n");

        terminated++;
    }
    xsim_fini(xsim);  
    xsim_free(xsim);  
    return EXIT_FAILURE;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

struct sigaction old_act;

void install_handler()
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = handler;
    sigaction(SIGSEGV, &act, &old_act);
    return;
}

void uninstall_handler()
{
    sigaction(SIGSEGV, &old_act, 0);
}

void handler(int signo)
{
    int cnt = 0;
    pid_t pid = syscall(SYS_getpid);
    if (signo == SIGSEGV) {
        EMSG("SIGSEGV pid: %d\n", pid);
    }
    while (1) {
        cnt++;
        if ((cnt % 100000000) == 0)
            EMSG("SIGSEGV pid: %d\n", pid);
    }
}

