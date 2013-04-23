#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gnuplot_measure.h>
#include <gnuplot_read.h>

#define MEASURE_FILE        "%s_node%02d.perf", begin_name, n

int iface_total = 1;

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

void gnuplot_init_measure(int proc, int node, int iface)
{
    iface_total = iface;
    for (int i=0 ; i<nb_type ; i++) {
        table_measure_type[i] = malloc(sizeof(cell_t*) * proc);
        for (int j=0 ; j<proc ; j++) {
            table_measure_type[i][j] = malloc(sizeof(cell_t) * (node*iface));
            for (int k=0 ; k<node*iface ; k++) {
                table_measure_type[i][j][k].next       = NULL;
                table_measure_type[i][j][k].nb_node    = (k/iface) + 1;
                table_measure_type[i][j][k].nb_iface   = (k%iface) + 1;
                table_measure_type[i][j][k].nb_measure = 0;
                table_measure_type[i][j][k].time       = 0;
            }
        }
    }
}

void gnuplot_free_measure(int proc, int node, int iface)
{
    for (int i=0 ; i<nb_type ; i++) {
        for (int j=0 ; j<proc ; j++) {
            cell_t *table = table_measure_type[i][j];
            for (int k=0 ; k<node*iface ; k++) {
                while (table[k].next != NULL) {
                    measure_t *tmp = table[k].next;
                    table[k].next  = tmp->next;
                    free(tmp);
                }
            }
            free(table_measure_type[i][j]);
        }
        free(table_measure_type[i]);
    }
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

int gnuplot_compute_result_for_a_measure(int proc, int node, int iface)
{
    for (int t=0 ; t<nb_type ; t++) {
        cell_t    *c    = &(table_measure_type[t][proc-1][INDICE(node, iface)]);
        measure_t *m    = c->next;
        measure_t *last = m;
        uint64_t   res  = 0;
        /* first measure */
        if ((m != NULL) && !(m->global)) {
            if (t == global_processus_time) {
                if (m->time > res) {
                    res = m->time;
                }
            } else {
                res += m->time;
            }
            last = m;
            m    = m->next;
        }
        /* others measures */
        while ((m != NULL) && !(m->global)) {
            free(last);
            if (t == global_processus_time) {
                if (m->time > res) {
                    res = m->time;
                }
            } else {
                res += m->time;
            }
            last = m;
            m    = m->next;
        }

        /* set the result and link the list */
        if ((last != NULL) && !(last->global)) {
            last->global = 1;
            last->time   = res;
            c->next      = last;
            c->nb_measure++;
        }
    }

    return EXIT_SUCCESS;
}    

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

int gnuplot_read_all_measure(const char* begin_name, int proc, int node, int iface)
{
    for (int n=0 ; n<node ; n++) {
        int size = snprintf(NULL, 0, MEASURE_FILE);
        char *name = malloc(sizeof(char) * (size+1));
        sprintf(name, MEASURE_FILE);

        gnuplot_read_a_measure(name, proc, node, iface);

        free(name);
    }
    gnuplot_compute_result_for_a_measure(proc, node, iface);
    return 0;
}


int gnuplot_read_a_measure(const char *name, int proc, int node, int iface)
{
    /* open the file */
	FILE* fptr = fopen(name, "r");
	if (fptr == NULL) {
		fprintf(stderr, "An error occurs when opening the file %s.\n", name);
		return EXIT_FAILURE;
	}


    /* variable for the file reading */
    char     title[LINESIZE];
    int      line_nb      = 0;

    /* other variable for all the reading file */
    int benchmark = 100;
    int global_benchmark = 100;

    while (get_title(fptr, &line_nb, title) == 0){

        if (strcmp(title, "benchmark") == 0) {
            benchmark = get_cycle(fptr, &line_nb, MEAN);
            if (benchmark < 0)
                goto end_error;
        }

        if (strcmp(title, "global_benchmark") == 0) {
            global_benchmark = get_ns(fptr, &line_nb, MEAN);
            if (global_benchmark < 0)
                goto end_error;
        }

        if (strstr(title, "lock_wait") != NULL) {
            uint64_t time = get_cycle(fptr, &line_nb, TOTAL);
            if (time == 0)
                goto end_error;

            add_measure(time, benchmark, lock_wait, proc, node, iface);
        }

        if ((strstr(title, "lock") != NULL) && 
                (strstr(title, "wait") == NULL)) {
            uint64_t time = get_cycle(fptr, &line_nb, TOTAL);
            if (time == 0)
                goto end_error;

            add_measure(time, benchmark, in_lock, proc, node, iface);
        }

       /******************************************************************/ 

        if (strcmp(title, "global_simulation_time") == 0) {
            uint64_t time = get_ns(fptr, &line_nb, TOTAL);
            if (time == 0)
                goto end_error;

            add_measure(time, global_benchmark, global_simulation_time, proc, node, iface);
        }

        if (strcmp(title, "global_processus_time") == 0) {
            uint64_t time = get_ns(fptr, &line_nb, TOTAL);
            if (time == 0)
                goto end_error;

            add_measure(time, global_benchmark, global_processus_time, proc, node, iface);
        }

        if (strcmp(title, "global_listener_time") == 0) {
            uint64_t time = get_ns(fptr, &line_nb, TOTAL);
            if (time == 0)
                goto end_error;

            add_measure(time, global_benchmark, global_listener_time, proc, node, iface);
        }

        /* TODO completer avec les autres types */
    }


    /* close the file */
	if (fclose(fptr) != 0) {
		fprintf(stderr, "An error occurs when closing the file %s.\n", 
                name);
	}
    return EXIT_SUCCESS;

end_error:
    fprintf(stderr, "Error: file not writen as expected line: %d\n", line_nb);
	/* close the file */
	if (fclose(fptr) != 0) {
		fprintf(stderr, "An error occurs when closing the gnuplot output file: %s.\n", 
                name);
	}
    return EXIT_FAILURE;
}


void add_measure(uint64_t time, int benchmark, measure_type_t type, int proc, int node, int iface)
{
    cell_t    *c = &(table_measure_type[type][proc-1][INDICE(node, iface)]);
    measure_t *m = malloc(sizeof(measure_t));
    m->time      = time;
    m->benchmark = benchmark;
    m->global    = 0;

    m->next      = c->next;
    c->next      = m;
}


