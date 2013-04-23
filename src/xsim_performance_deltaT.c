#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include <xsim_sync.h>
#include <xsim_performance_evaluation.h>

#include <xsim_performance_deltaT.h>


#define DBG_HDR "xsim:time_performance_deltaT"
#ifdef XSIM_PERFORMANCE_DELTAT_DEBUG
#define DEBUG
#endif 

#ifdef XSIM_PERFORMANCE_DELTAT_HDEBUG
#define HUGE_DEBUG
#endif 

#include <xsim_debug.h>




#define NB_PROC_CMD     "cat /proc/cpuinfo | grep processor | wc -l"
#define LINESIZE        20
#define DELTA_LIMIT	    100


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int xsim_nb_proc()
{
    int nb_proc = 1;

    int size = snprintf(NULL, 0, NB_PROC_CMD);
    char *command = malloc(sizeof(char) * (size+1));
    sprintf(command, NB_PROC_CMD);

    /* execute the command */
    FILE *fptr = popen(command, "r");
	if (fptr == NULL) {
		fprintf(stderr, "An error occurs when executing the script %s.\n", command);
		return EXIT_FAILURE;
	}

    char line[LINESIZE] = "";
    if (fgets(line, LINESIZE, fptr) == NULL) {
		fprintf(stderr, "An error occurs when reading the output of the script %s.\n", command);
		return EXIT_FAILURE;
	} else {
        nb_proc = atoi(line);
    }

    /* close the file */
	if (fclose(fptr) != 0) {
		fprintf(stderr, "An error occurs when closing the file %s.\n", 
                command);
    }
    free(command);

    return nb_proc;
}

void measure_delta_t(int id, xsim_deltaT_t *table)
{
    int i = 0;
    int k = 0;
    /* selection its CPU */
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(id, &mask);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask) != 0) {
		EMSG("[%d] Got error when setting the affinity for deltaT\n", id);
	}
    sleep(1);
    if (sched_getcpu() != id) {
        fprintf(stderr, "Processus %d is not on the right processor.\n", id);
    }

    /* wait the other processes */
    xsim_barrier_wait(&table->barrier1);
    if (id == 0) 
        sleep(1);
    for (i=0 ; i<NB_PROG ; i++) {
        /* Measure its time */
        if (id == 0) {
            for (k=1 ; k<table->nb_proc ; k++) {
                table->time[0 + i*3 + (k-1)*NB_PROG*3] = Readtsc();
                while (table->time[1 + i*3 + (k-1)*NB_PROG*3] == 0); 
                table->time[2 + i*3 + (k-1)*NB_PROG*3] = Readtsc();
            }
        } else {
            while (table->time[0 + i*3 + (id-1)*NB_PROG*3] == 0); 
            table->time[1 + i*3 + (id-1)*NB_PROG*3] = Readtsc();
        }
    }

    /* detach the shared memory */ 
    if (shmdt(table) == -1) {
        fprintf(stderr, "shmdt for deltaT failed\n");
    }
    exit(EXIT_SUCCESS);
}

int xsim_evaluate_diff_time_between_proc()
{
    int nb_proc     = xsim_nb_proc();
    int spawned     = 0;
    int shm_id      = 0;
    int error       = 0;
    int terminated  = 0;
    int pid         = 0;
    int ret         = 0;
    int status      = 0;
    int64_t *deltaT = NULL;
    int k           = 0;
    int i           = 0;
    fprintf(stdout, "There is %d proc.\n", nb_proc);

    if (nb_proc == 1)
        return EXIT_SUCCESS;

    /*
     * Create shared memory
     */
    shm_id = shmget(IPC_PRIVATE, xsim_sizeof_xsim_deltaT_t(nb_proc), IPC_CREAT | 0640);
    if (shm_id == -1) {
        EMSG("shmget: shmget for deltaT failed");
        return EXIT_FAILURE;
    }

    /*
     * Attach shared memory
     */
    xsim_deltaT_t *common = (xsim_deltaT_t*)shmat(shm_id, (void *)0, 0);
    if (common == (void *)-1) {
        EMSG("shmat: shmat for deltaT failed");
        return EXIT_FAILURE;
    }

    /* initialize xsim_deltaT_t */
    xsim_barrier_init(&common->barrier1, nb_proc);
    common->nb_proc = nb_proc;
    common->time = &common->first_time;
    memset(common->time, 0, xsim_sizeof_table(nb_proc));

    while (spawned < nb_proc) {
        pid = fork();
        switch(pid){
            case -1: /* error */
                error = 1;
                goto on_fork_error;

            case 0:  /* son */
                measure_delta_t(spawned, common);
                EMSG("[%d] Must not be return in this function !!!!\n", spawned);
                exit(EXIT_FAILURE);

            default: /* father */
                spawned++;
                break;
        }
    }

on_fork_error:
    /* wait the childs end */
    do {
        ret = wait(&status);
        if( WEXITSTATUS(status) )
            EMSG("Got an error when measuring deltaT.\n");
        terminated++;
    } while (ret > 0); /* no problem: ID 0 is init */
    terminated--; /* one for the last loop */

    if( (errno == ECHILD) && (terminated == spawned) ) {
        DMSG("Got all my childs for deltaT measure.\n");
    } else {
        EMSG("Something went wrong in deltaT measure: handle it (%d / %d)\n", terminated, spawned);
    }

    deltaT = malloc(xsim_sizeof_table(nb_proc)/3);
    memset(deltaT, 0, xsim_sizeof_table(nb_proc)/3);
    if (!error) {
        for (k=1 ; k<nb_proc ; k++) {
            for (i=0 ; i<NB_PROG ; i++) {
                int64_t tmp = (common->time[2 + 3*i + (k-1)*NB_PROG*3] - 
                        common->time[0 + 3*i + (k-1)*NB_PROG*3])/2;
                deltaT[i]   = common->time[1 + 3*i + (k-1)*NB_PROG*3] - tmp - 
                    common->time[0 + 3*i + (k-1)*NB_PROG*3];
                // fprintf(stdout, "0 - 1 - 2 : %"PRId64" - %"PRId64" - %"PRId64"\n",
                //         common->time[0+3*i], common->time[1+3*i], common->time[2+3*i]);
                DMSG("essai %d - couple %d-%d: distance: %"PRId64" ; deltaT: %"PRId64"t\n", 
                        i, 0, k, tmp, deltaT[i]);
                if ((deltaT[i] > DELTA_LIMIT) || (-deltaT[i] > DELTA_LIMIT)) {
                    fprintf(stderr, "Warning: the difference between processor %d and %d is greater than %d: %"PRId64" !\n",
                            0, k, DELTA_LIMIT, deltaT[i]); 
                }
            }
        }

    }

    /* free xsim_deltaT_t */
    xsim_barrier_fini(&common->barrier1);
    if (shmdt(common) == -1) {
        EMSG("shmdt for deltatT failed\n");
    }
    if ((shmctl(shm_id, IPC_RMID, NULL)) == -1) {
        EMSG("shmctl: shmctl for deltaT failed");
    }
    free(deltaT);
    return EXIT_SUCCESS;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


