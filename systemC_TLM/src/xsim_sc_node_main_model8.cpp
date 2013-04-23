#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>

#include <systemc.h>

#include <xsim_sc_node.h>

#include <xsim_time_model.h>

//#define DBG_HDR "xsim_sc_main"
//#ifdef XSIM_SC_MAIN_DEBUG
//#define DEBUG
//#endif 
//
//#ifdef XSIM_SC_MAIN_HDEBUG
//#define HUGE_DEBUG
//#endif 
//
//#include <xsim_debug.h>

#ifdef __cplusplus
extern "C" {
#endif 

// int xsim_sc_node_main (int ncycles, xsim_node_t *node)
// {
//     char *argv[3];
//     argv[0] = (char*) malloc(sizeof(char)*30);
//     argv[1] = (char*) malloc(sizeof(char)*30);
//     sprintf(argv[0], "sc_main");
//     sprintf(argv[1], "%d", ncycles);
//     argv[2] = (char*) node;
// 
//     int ret = sc_elab_and_sim(3, argv); // should call sc_main
//     printf("It worked well for node %d.\n", node->node_id);
// 
//     free(argv[0]);
//     free(argv[1]);
// 
//     return ret;
// }


//int sc_main(int argc, char* argv[])
//{
//    if (argc != 3) {
//        EMSG("Internal error in the call to sc_main.\n");
//        return EXIT_FAILURE;
//    }
//    int ncycles;
//    sscanf(argv[1],"%d",&ncycles);
//    xsim_node_t *node = (xsim_node_t*) argv[2];

int xsim_sc_node_main (int ncycles, xsim_node_t *node)
{
    // char name[30] = "helloooo";
    // xsim_sc_wrapper_send<DSPIN_DATA_SIZE> wrapper_send_test(name, node->iface[0]);

    /* To remove the warning about the deprecated usage */
//    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING); 

    char str[30];
    char res[30];
    srandom(time(NULL));

    sprintf(str, "node[%x]", node->node_id);
    xsim_sc_node<DSPIN_DATA_SIZE> sc_node(str, node); 
    printf("Initialization successful\n");

    float per;
    float aver;

    FILE *f;
    sprintf (res, "res_node%d", node->node_id);
    f = fopen (res, "wt");

    for (per = MIN_PER; per < MAX_PER ; per+=1) {

        sc_node.set_percent((float)per/100);
        //sc_node.set_percent((float)per/100) * PER[z*NUM_Y*NUM_X + y*NUM_X + x] * ((MAX_PACKET_LENGTH+MIN_PACKET_LENGTH)/2);

        printf("Run the simulation.\n");

        /* global measure */
        xsim_perf_begin_global_measure(global_processus_time, CLOCK_PROC);
        xsim_perf_begin_global_measure(global_simulation_time, CLOCK_OTHERS);

        sc_start(2*ncycles, TIMING);

        /* global measure */
        xsim_perf_end_global_measure(global_processus_time, CLOCK_PROC);
        xsim_perf_end_global_measure(global_simulation_time, CLOCK_OTHERS);

        aver = sc_node.get_average();
        printf ("%f \n", aver);

        fprintf (f,"%f : %f\n", (per/*/100*/)/**MAX_PACKET_LENGTH*/, aver);
        fflush (f);
        //	cout << "average is :" << Aver << endl;
    }
    fclose (f);

    return EXIT_SUCCESS;
};


/******************************************************************************/
/********* Empty functions to respect some previous definition ****************/
/******************************************************************************/

void resource_init_simulation(xsim_node_t *node __attribute__((__unused__)), 
        xsim_iface_t **iface __attribute__((__unused__)), 
        int nb_nodes __attribute__((__unused__)))
{
    return;
}

void resource_end_simulation(xsim_node_t *node __attribute__((__unused__)), 
        xsim_iface_t **iface __attribute__((__unused__)), 
        int nb_nodes __attribute__((__unused__)))
{
    return;
}

int  resource_handle_msg(xsim_node_t *node __attribute__((__unused__)), 
        xsim_msg_t *msg __attribute__((__unused__)), 
        xsim_iface_t *iface __attribute__((__unused__)))
{
    return 0;
}

int  resource_simulation(xsim_node_t *node __attribute__((__unused__)), 
        xsim_msg_list_t **NIC_msg_list_output __attribute__((__unused__)),
		sim_time *next_time_for_resource __attribute__((__unused__)))
{
    return 0;
}

void resource_wake_up()
{
    return;
}


// int  son_body_model4(xsim_t *xsim __attribute__((__unused__)), 
//         int node_id __attribute__((__unused__)), 
//         sim_time limit_simulation_time __attribute__((__unused__)), 
//         const char *output_measure __attribute__((__unused__)))
// {
//     fprintf(stderr, "The time model called is not implemented in this programme.\nUse -m 6\n");
//     return 0;
// }
// 
// int  son_body_model1(xsim_t *xsim __attribute__((__unused__)), 
//         int node_id __attribute__((__unused__)), 
//         sim_time limit_simulation_time __attribute__((__unused__)), 
//         const char *output_measure __attribute__((__unused__)))
// {
//     fprintf(stderr, "The time model called is not implemented in this programme.\nUse -m 6\n");
//     return 0;
// }

#ifdef __cplusplus
}
#endif 
