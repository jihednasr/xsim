#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <systemc.h>

#include <xsim_sc_node.h>
#include <xsim_sc_FIFO_time.h>

#include <xsim_sc_topology.h>

#define DIFF_TIME(begin, end)       ((long long int)(end.tv_sec - begin.tv_sec)*1000000000 + (long long int)(end.tv_nsec - begin.tv_nsec))
#define COMMAND                     "echo \"     %2d   %2d     %d       %lld \" >> %s", \
                                                nb_proc, nb_nodes, nb_iface,\
                                            DIFF_TIME(begin,end), output_file

int sc_main(int argc, char* argv[])
{
    if (argc != 7) {
        EMSG("Internal error in the call to sc_main.\nExpected: %s nb_cycles nb_proc nb_nodes nb_ifaces topology_file output_file\n", argv[0]);
        return EXIT_FAILURE;
    }
    int ncycles, nb_proc, nb_nodes, nb_iface;
    sscanf(argv[1],"%d",&ncycles);
    sscanf(argv[2],"%d",&nb_proc);
    sscanf(argv[3],"%d",&nb_nodes);
    sscanf(argv[4],"%d",&nb_iface);
    char *topo_file   = argv[5];
    char *output_file = argv[6];

    struct timespec begin;
    struct timespec end;

    xsim_topology_init(topo_file, nb_nodes);

    /* To remove the warning about the deprecated usage */
//    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING); 

    char str[30];
    char fifo_str[30];
    xsim_sc_node<DSPIN_DATA_SIZE> * sc_node[nb_nodes];
    xsim_sc_fifo_time<DSPIN_DATA_SIZE> * fifo[nb_nodes][nb_nodes][nb_iface];

    int n = 0;
    int m = 0;
    int i = 0;
    for (n = 0 ; n<nb_nodes ; n++) {
        sprintf(str, "node[%x]", n);
        sc_node[n] = new xsim_sc_node<DSPIN_DATA_SIZE>(str, nb_nodes, nb_iface, n); 
    }
    for (n = 0 ; n<nb_nodes ; n++) {
        for (m = 0 ; m<nb_nodes ; m++) {
            for (i=0 ; i<nb_iface ; i++) {
                sprintf(fifo_str, "fifo[%d]->[%d]_%dif", n, m, i);
                fifo[n][m][i] = new xsim_sc_fifo_time<DSPIN_DATA_SIZE>(fifo_str, xsim_topology_travel_time(n, m)+1);
            }
        }
    }
    for (n = 0 ; n<nb_nodes ; n++) {
        for (m = 0 ; m<nb_nodes ; m++) {
            /* n indicates the source ; m indicates the destination */
            for (i=0 ; i<nb_iface ; i++) {
                sc_node[n]->wrapper_send[i]->out[m](*fifo[n][m][i]);
                sc_node[n]->wrapper_recv[i]->in[m] (*fifo[m][n][i]);
                fifo[m][n][i]->event_register(sc_node[n]->wrapper_recv[i]->event_receive_a_msg);
            }
        }
    }
    
    xsim_topology_free(nb_nodes);
    printf("Initialization successful\n");


    float per = MIN_PER;

    for (n = 0 ; n<nb_nodes ; n++) {
        sc_node[n]->set_percent((float)per/100);
        //sc_node.set_percent((float)per/100) * PER[z*NUM_Y*NUM_X + y*NUM_X + x] * ((MAX_PACKET_LENGTH+MIN_PACKET_LENGTH)/2);
    }

    printf("Run the simulation.\n");
    /* begin measure */
    clock_gettime(CLOCK_REALTIME, &(begin));
    sc_start(ncycles, TIMING);

    /* end measure */
    clock_gettime(CLOCK_REALTIME, &(end));
    /* output the measures */
    int size = snprintf(NULL, 0, COMMAND);
    char *command = (char*) malloc(sizeof(char) * (size+1));
    sprintf(command, COMMAND);

    system(command);
    free(command);

    fprintf(stderr, "End of simulation\n");

    for (n = 0 ; n<nb_nodes ; n++) {
        delete sc_node[n];
        for (m = 0 ; m<nb_nodes ; m++) {
            for (i=0 ; i<nb_iface ; i++) {
                delete fifo[n][m][i];
            }
        }
    }

    return EXIT_SUCCESS;
};

