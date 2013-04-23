#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gnuplot_main_tools.h>
#include <gnuplot_measure.h>
#include <gnuplot_output.h>
#include <gnuplot_read.h>
#include <gnuplot_systemC.h>


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
#define EXEC                    "../tests/test_poisson"
#define SYSTEMC_REF             "../systemC/Platform_3DSPIN/simulation_2d_mesh"
#define SYSTEMC_TLM_REF         "../systemC_TLM_ref/simulation_systemC_ref"
#define SC_DSPIN_MODEL6         "../systemC/simulation_systemC_3DSPIN_model6"
#define SC_DSPIN_MODEL7         "../systemC/simulation_systemC_3DSPIN_model7"
#define SC_DSPIN_MODEL8         "../systemC_TLM/simulation_systemC_model8"
#define SC_DSPIN_MODEL9         "../systemC_TLM/simulation_systemC_model9"
#define TOPO_REP                "../topology"

#define COMMAND_POISSON         "%s -n %d -i %d -tl %d -m %d -om %s -tp %s/2d_mesh_%dnodes_%dx_%dy.topo> /dev/null", \
                                        EXEC, n, i, tl, model, measure_name, TOPO_REP, n, NB_X(n), NB_Y(n) 
#define COMMAND_SC_DSPIN_MODEL6 "%s -n %d -i %d -tl %d -m %d -om %s -tp %s/2d_mesh_%dnodes_%dx_%dy.topo > /dev/null", \
                                        SC_DSPIN_MODEL6, n, i, tl, model, measure_name, TOPO_REP, n, NB_X(n), NB_Y(n)
#define COMMAND_SC_DSPIN_MODEL7 "%s -n %d -i %d -tl %d -m %d -om %s -tp %s/2d_mesh_%dnodes_%dx_%dy.topo > /dev/null", \
                                        SC_DSPIN_MODEL7, n, i, tl, model-1, measure_name, TOPO_REP, n, NB_X(n), NB_Y(n)
#define COMMAND_SC_DSPIN_MODEL8 "%s -n %d -i %d -tl %d -m %d -om %s -tp %s/2d_mesh_%dnodes_%dx_%dy.topo > /dev/null", \
                                        SC_DSPIN_MODEL8, n, i, tl, model-2, measure_name, TOPO_REP, n, NB_X(n), NB_Y(n)
#define COMMAND_SC_DSPIN_MODEL9 "%s -n %d -i %d -tl %d -m %d -om %s -tp %s/2d_mesh_%dnodes_%dx_%dy.topo > /dev/null", \
                                        SC_DSPIN_MODEL9, n, i, tl, model-3, measure_name, TOPO_REP, n, NB_X(n), NB_Y(n)
#define COMMAND_SYSTEMC_REF     "%s_%dnodes_%dx_%dy %d %d %s > /dev/null", \
                                        SYSTEMC_REF, n, NB_X(n), NB_Y(n), tl, p, systemc_perf_measure 
#define COMMAND_SYSTEMC_TLM_REF "%s %d %d %d %d  %s/2d_mesh_%dnodes_%dx_%dy.topo %s > /dev/null", \
                                        SYSTEMC_TLM_REF, tl, p, n, i, TOPO_REP, n, NB_X(n), NB_Y(n), systemc_perf_measure 

#define MEASURE_NAME            "%s/%s_%dproc_%dnode_%diface_%dtime_%dmodel_%dtrial", \
                                        MEASURE_REP, gp_name, p, n, i, tl, model, k
#define SYSTEMC_MEASURE_NAME    "./%s/systemC_%s_%dproc_%dnode_%diface_%dtime.perf", \
                                        MEASURE_REP, gp_name, nb_proc, nb_node, \
                                        nb_iface, tl

#define SYSTEMC_INIT_CMD        "echo \"# proc node iface    execution time\" > %s",\
                                        systemc_perf_measure


#define OFFLINE_CMD(p)  "~/../fournel/bin/start_stop_cpu %d off", (p)
#define ONLINE_CMD(p)   "~/../fournel/bin/start_stop_cpu %d on",  (p)


#define CPU_PRESENT     "/sys/devices/system/cpu/present"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int nb_cpu_max       = 1;
int next_cpu_online  = 0; /* = last cpu online + 1 */

int get_cpu_info()
{
    /* open the file */
	FILE* fptr = fopen(CPU_PRESENT, "r");
	if (fptr == NULL) {
		fprintf(stderr, "An error occurs when opening the file %s.\n", CPU_PRESENT);
		return EXIT_FAILURE;
	}

    nb_cpu_max      = get_nb_cpu(fptr);
    next_cpu_online = nb_cpu_max;

    /* close the file */
	if (fclose(fptr) != 0) {
		fprintf(stderr, "An error occurs when closing the file %s.\n", 
                CPU_PRESENT);
	}
    fprintf(stdout, "nb cpu: %d\n", nb_cpu_max);
    return EXIT_SUCCESS;
}

/* 
 * by convention if the argument is 0,
 * all the cpu are set offline or online
 * (but CPU0 stay always online)
 */
void cpu_online(int add_nb_cpu)
{
    int p = 0;
    int limit = 0;
    int size = 0;
    char *command = NULL;

    if (add_nb_cpu == 0) {
        limit = nb_cpu_max;
    } else {
        limit = next_cpu_online + add_nb_cpu;
        if (limit > nb_cpu_max) {
            limit =  nb_cpu_max;
        }
    }
    //fprintf(stdout, "Limit for online: %d ; add_nb_cpu: %d ; nb_cpu_max: %d ; next_cpu_online: %d.\n", 
      //      limit, add_nb_cpu, nb_cpu_max, next_cpu_online);
    for (p=next_cpu_online ; p<limit ; p++) {
        fprintf(stdout, "**** Try: online cpu %d.\n", p);
        size = snprintf(NULL, 0, ONLINE_CMD(p));
        command = malloc(sizeof(char) * (size+1));
        sprintf(command, ONLINE_CMD(p));
        system(command);
        sleep(5);
        free(command);
    }
    next_cpu_online = limit;
}

#define MIN_CPU     1
void cpu_offline(int sub_nb_cpu)
{
    int p = 0;
    int limit = 0;
    int size = 0;
    char *command = NULL;

    if (sub_nb_cpu == 0) {
        limit = MIN_CPU;
    } else {
        limit = next_cpu_online - sub_nb_cpu;
        if (limit < MIN_CPU) {
            limit =  MIN_CPU;
        }
    }
    //fprintf(stdout, "Limit for offline: %d ; sub_nb_cpu: %d ; nb_cpu_max: %d ; next_cpu_online: %d.\n",
      //      limit, sub_nb_cpu, nb_cpu_max, next_cpu_online);
    for (p=next_cpu_online-1 ; p>=limit ; p--) {
        fprintf(stdout, "**** Try: offline cpu %d.\n", p);
        size = snprintf(NULL, 0, OFFLINE_CMD(p));
        command = malloc(sizeof(char) * (size+1));
        sprintf(command, OFFLINE_CMD(p));
        system(command);
        sleep(5);
        free(command);
    }
    next_cpu_online = limit;
}




/************** main **********************/
int main(int argc, char **argv) 
{
    /* arguments of the program */
    char    *gp_name      = NULL;
    int      tl           = 100;
    int      nb_iface     = 1;
    int      nb_node      = 2;
    int      nb_proc      = 1;
    int      model        = 4;
    
    /* other variables */
    int p    = 0;
    int n    = 0;
    int i    = 0;
    int k    = 0;
    int size = 0;
    char *systemc_perf_measure = NULL;
    char *command              = NULL; 

    if (read_argument(argc, argv, &gp_name, &tl,
                &nb_iface, &nb_node, &nb_proc, &model) == EXIT_FAILURE) 
        return EXIT_FAILURE;

    fprintf(stdout, "gp_name: %s ; tl: %d ; nb_iface: %d ; nb_node: %d ; nb_proc: %d ; model: %d.\n", 
            gp_name, tl, nb_iface, nb_node, nb_proc, model);

    if (nb_node < NB_NODE_MIN) {
        fprintf(stderr, "Error, get %d nodes but the minimum must be %d\n", nb_node, NB_NODE_MIN);
        free(gp_name);
        return EXIT_FAILURE;
    }

    get_cpu_info();
    if (nb_cpu_max < nb_proc) {
        fprintf(stderr, "Warning: ask for %d processors but the computer has only %d processors. The number of processors is set to %d.\n\n",
                nb_proc, nb_cpu_max, nb_cpu_max);
        nb_proc = nb_cpu_max;
    }

    /* Init structure for storing the measures */
    gnuplot_init_measure(nb_proc, nb_node, nb_iface);
    gnuplot_systemC_init(nb_proc, nb_node, nb_iface);

    /* Name for the systemC measures output */
    size                 = snprintf(NULL, 0, SYSTEMC_MEASURE_NAME);
    systemc_perf_measure = malloc(sizeof(char) * (size+1));
    sprintf(systemc_perf_measure, SYSTEMC_MEASURE_NAME);
    
    /* Initialise the output file for SystemC measure */
    size = snprintf(NULL, 0, SYSTEMC_INIT_CMD);
    command = malloc(sizeof(char) * (size+1));
    sprintf(command, SYSTEMC_INIT_CMD);
    system(command);
    free(command);

    cpu_offline(0);
    for (p=NB_PROC_MIN ; p<=nb_proc ; p++) {
        /* Execute and read measures */
        for (n=NB_NODE_MIN ; n<=nb_node ; n++) {
           if (NODE_NOT_USED(n)) 
               continue;
           for (i=NB_IFACE_MIN ; i<=nb_iface ; i++) {
                for (k=0 ; k<LOOP ; k++) {
                    fprintf(stdout, "proc: %d ; node: %d ; iface: %d ; Measure n°%d\n", 
                            p, n, i, k);

                    /* xsim execution + measures */
                    size = snprintf(NULL, 0, MEASURE_NAME);
                    char *measure_name = malloc(sizeof(char) * (size+1));
                    sprintf(measure_name, MEASURE_NAME);

                    if ((model == 1) || (model == 4)) {
                        size = snprintf(NULL, 0, COMMAND_POISSON);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_POISSON);
                    } else if (model == 6) {
                        size = snprintf(NULL, 0, COMMAND_SC_DSPIN_MODEL6);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_SC_DSPIN_MODEL6);
                    } else if (model == 7) {
                        size = snprintf(NULL, 0, COMMAND_SC_DSPIN_MODEL7);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_SC_DSPIN_MODEL7);
                    } else if (model == 8) {
                        size = snprintf(NULL, 0, COMMAND_SC_DSPIN_MODEL8);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_SC_DSPIN_MODEL8);
                    } else if (model == 9) {
                        size = snprintf(NULL, 0, COMMAND_SC_DSPIN_MODEL9);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_SC_DSPIN_MODEL9);
                    } else {
                        fprintf(stderr, "Model %d not known.\n", model);
                        cpu_online(0);
                        gnuplot_free_measure(nb_proc, nb_node, nb_iface);
                        gnuplot_systemC_free();
                        free(systemc_perf_measure);
                        free(gp_name);
                    } 


                    system(command);
                    gnuplot_read_all_measure(measure_name, p, n, i);

                    free(command);
                    free(measure_name);

                    /* systemC execution + measures */
                    if ((model == 6) || (model == 7)) {
                        size    = snprintf(NULL, 0, COMMAND_SYSTEMC_REF);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_SYSTEMC_REF);
                        system(command);
                        free(command);
                    } else if ((model == 8) || (model == 9)) {
                        size    = snprintf(NULL, 0, COMMAND_SYSTEMC_TLM_REF);
                        command = malloc(sizeof(char) * (size+1));
                        sprintf(command, COMMAND_SYSTEMC_TLM_REF);
                        system(command);
                        free(command);
                    }

                }
            }
        }

        /* add a CPU online */
        cpu_online(1);
    }
    cpu_online(0);

    gnuplot_systemC_read_all_measure(systemc_perf_measure, nb_proc, nb_node, nb_iface);
    gnuplot_out_all_measure(gp_name, nb_proc, nb_node, nb_iface, tl);

    gnuplot_free_measure(nb_proc, nb_node, nb_iface);
    gnuplot_systemC_free();
    free(systemc_perf_measure);
    free(gp_name);

    return EXIT_SUCCESS;

}










