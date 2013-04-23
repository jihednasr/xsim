#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>


#include <xsim_sc_3dspin_config.h>
#include <soclib_dspin_interfaces.h>
#include <soclib_TG.h>
#include <soclib_3dspin_router.h>

#include <xsim_sc_clk.h>

#include <xsim_node.h>
#include <xsim_time_model.h>

void xsim_sc_node_main (xsim_t *xsim, int node_id);

xsim_init_struct_t is;

int sc_main(int argc, char** argv) {
    read_argument(argc, argv, &is);

    void (**simulation)(xsim_t*, int) = (void(**)(xsim_t*, int)) malloc(sizeof(void(**)(xsim_t*, int)) * is.nb_nodes);
    for (int i=0 ; i<is.nb_nodes ; i++) {
        simulation[i] = xsim_sc_node_main;
    }
    xsim_spawner(simulation, is.nb_nodes, is.nb_ifaces, is.topology);

    free(simulation);
    return 0;
}

void xsim_sc_node_main (xsim_t *xsim, int node_id)
{
    /* To remove the warning about the deprecated usage */
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING); 

    /* Initialize xsim */
    xsim_node_t      *node                   = NULL;
    xsim_msg_list_t **to_send                = NULL; /* Not used in this case */

	xsim_time_model_init_son_body(xsim, node_id, &node, &to_send);

    /* Variables */
    int NUM_X = node->nb_nodes;
    int NUM_Y = 1;
    int NUM_Z = 1;
    int x = 0;
    int y = 0;
    int z = 0;
    int i = 0;
    
    int xx = 0;
    int yy = 0;
    int zz = 0;

    char str[30];
    char cluster_name[30];
    char res[30];
    char wrapper_clk_str[30];
    char sender_clk_str[30];

    /* Instantiation */
    sc_signal<bool>		signal_clk("signal_clk");
    sc_signal<bool> 	signal_resetn("signal_resetn");
    sc_signal<bool>    *signal_wrapper_clk[node->nb_iface];
    sc_signal<bool>    *signal_sender_clk [node->nb_iface];

    Xsim_sc_clk clkGenerator("Global_CLK", signal_clk, node->node_id);
    //clkGenerator.signal_clk  (signal_clk);

    DSPIN_SIGNALS<DSPIN_DATA_SIZE> local_in[node->nb_iface];
    DSPIN_SIGNALS<DSPIN_DATA_SIZE> local_out[node->nb_iface];

    SOCLIB_3DSPIN_ROUTER<DSPIN_DATA_SIZE> *cluster[node->nb_iface];
    SOCLIB_PRODUCER <DSPIN_DATA_SIZE, MAX_PACKET_LENGTH, MIN_PACKET_LENGTH>//, NUM_Z*NUM_Y*NUM_X> 
        *producer[node->nb_iface];

    int   ADR [NUM_Z*NUM_Y*NUM_X];
    //float PER [NUM_Z*NUM_Y*NUM_X];

    /* Initialisation */
    for (zz=0; zz<NUM_Z ; zz++) {
        for (yy=0; yy<NUM_Y ; yy++) {
            for (xx=0; xx<NUM_X ; xx++) {
                ADR [zz*NUM_Y*NUM_X + yy*NUM_X + xx] = (zz<<10) | (yy<<5) | xx;
                //PER [zz*NUM_Y*NUM_X + yy*NUM_X + xx] = 1.0/(NUM_Z*NUM_Y*NUM_X);
            }
        }
    }

    /* x, y, z convertion */
    int addr = ADR[node->node_id]; 
    x =  addr & 0x0000001F;
    y = (addr & 0x000003e0) >> 5;
    z = (addr & 0x00007c00) >> 10;

    for (i=0 ; i<node->nb_iface ; i++) {
        sprintf (wrapper_clk_str, "wrapper_clk[%x][%x][%x][%x]",z,y,x,i);
        sprintf (sender_clk_str,  "sender_clk[%x][%x][%x][%x]", z,y,x,i);
        signal_wrapper_clk[i] = new sc_signal<bool>(wrapper_clk_str);
        signal_sender_clk[i]  = new sc_signal<bool>(sender_clk_str);

        sprintf (str, "producer[%x][%x][%x][%x]",z,y,x,i);
        producer[i] = new SOCLIB_PRODUCER <DSPIN_DATA_SIZE, MAX_PACKET_LENGTH, 
                 MIN_PACKET_LENGTH> //NUM_Z*NUM_Y*NUM_X> 
                     (str, node->node_id, ADR, 50, node->nb_nodes); //(z*NUM_Y*NUM_X)+(y*NUM_X)+x, ADR, 50);
        producer[i]->CLK           (signal_clk);
        producer[i]->RESETN        (signal_resetn);
        producer[i]->init_out      (local_out[i]);
        producer[i]->targ_in       (local_in[i]);
        producer[i]->SENDER_CLK   (*signal_sender_clk[i]);
        producer[i]->WRAPPER_CLK  (*signal_wrapper_clk[i]);

        sprintf(cluster_name, "cluster[%x][%x][%x][%x]", z, y, x, i);
        cluster[i] = new SOCLIB_3DSPIN_ROUTER<DSPIN_DATA_SIZE>
            (cluster_name, addr, node->iface[i], ADR);
        cluster[i]->CLK          (signal_clk);
        cluster[i]->RESETN       (signal_resetn);
        cluster[i]->OUT          (local_in[i]);
        cluster[i]->IN           (local_out[i]);
        cluster[i]->SENDER_CLK   (*signal_sender_clk[i]);
        cluster[i]->WRAPPER_CLK  (*signal_wrapper_clk[i]);
    }


    for (i=0 ; i<node->nb_iface ; i++) {
        producer[i]->unif();
    }

    /* for "localy" */
    //	int dist [NUM_Z*NUM_Y*NUM_X];
    //	for (int zz=0; zz<NUM_Z ; zz++)
    //		for (int yy=0; yy<NUM_Y ; yy++)
    //		        for (int xx=0; xx<NUM_X ; xx++)
    //				dist [zz*NUM_Y*NUM_X + yy*NUM_X + xx] = abs(zz-z) + abs(yy-y) + abs(xx-x);
    //	for (i=0 ; i<node->nb_iface ; i++) {
    //	    producer[i]->localize(dist);
    //	}

    ///////////////////

    //printf("\nHit ENTER to end start...\n");
    //char buf[1];
    //cin.getline(buf,2);

    float per;
    float Aver;



    FILE *f;
    sprintf (res, "res_node%d", node->node_id);
    f = fopen (res, "wt");

    for (per = MIN_PER; per < MAX_PER ; per+=1) {

        for (i=0 ; i<node->nb_iface ; i++) {
            producer[i]-> Percent = ((float)per/100) /* * PER[z*NUM_Y*NUM_X + y*NUM_X + x] * ((MAX_PACKET_LENGTH+MIN_PACKET_LENGTH)/2)*/;
        }

        /* global measure */
        xsim_perf_begin_global_measure(global_processus_time, CLOCK_PROC);
        xsim_perf_begin_global_measure(global_simulation_time, CLOCK_OTHERS);

        //printf("RESET.\n");
        signal_resetn = false;
        sc_start(BASE_TIME*2, TIMING);         

        //printf("Run the simulation.\n");
        signal_resetn = true;
        sc_start(2*is.simulation_time-BASE_TIME, TIMING);         

        /* global measure */
        xsim_perf_end_global_measure(global_processus_time, CLOCK_PROC);
        xsim_perf_end_global_measure(global_simulation_time, CLOCK_OTHERS);

        Aver = 0;
        int Count = 0;

        for (i=0 ; i<node->nb_iface ; i++) {
            if (producer[i]->A_COUNT) {
                Aver += producer[i]->Latency;
                Count += producer[i]->A_COUNT;
            }
        }
        Aver /= Count;
        printf ("%f \n", Aver);

        fprintf (f,"%f : %f\n", (per/*/100*/)/**MAX_PACKET_LENGTH*/, Aver);
        fflush (f);
        //	cout << "average is :" << Aver << endl;
    }
    fclose (f);


    for (i=0 ; i<node->nb_iface ; i++) {
        delete producer[i];
        delete cluster[i];
        delete signal_wrapper_clk[i];
        delete signal_sender_clk[i];
    }

    xsim_time_model_end_of_simulation(node, to_send, is.measures_output);
    return;
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


int  son_body_model4(xsim_t *xsim __attribute__((__unused__)), 
        int node_id __attribute__((__unused__)), 
        sim_time limit_simulation_time __attribute__((__unused__)), 
        const char *output_measure __attribute__((__unused__)))
{
    fprintf(stderr, "The time model called is not implemented in this programme.\nUse -m 6\n");
    return 0;
}

int  son_body_model1(xsim_t *xsim __attribute__((__unused__)), 
        int node_id __attribute__((__unused__)), 
        sim_time limit_simulation_time __attribute__((__unused__)), 
        const char *output_measure __attribute__((__unused__)))
{
    fprintf(stderr, "The time model called is not implemented in this programme.\nUse -m 6\n");
    return 0;
}

