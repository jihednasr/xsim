#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include <gnuplot_measure.h>
#include <gnuplot_read.h>
#include <gnuplot_systemC.h>

int nb_iface_total = NB_IFACE_MIN;
int nb_node_total  = NB_NODE_MIN;
void gnuplot_systemC_init(int proc, int node, int iface)
{
    systemC_time_table = malloc(sizeof(uint64_t) * proc * node * iface);
    memset(systemC_time_table, 0, sizeof(uint64_t)*proc*node*iface);
    nb_iface_total = iface;
    nb_node_total  = node;
    return;
}

void gnuplot_systemC_free()
{
    free(systemC_time_table);
    return;
}

void gnuplot_systemC_read_all_measure(char *name, int proc, int node, int iface)
{
    /* open the file */
    FILE* fptr = fopen(name, "r");
    if (fptr == NULL) {
        fprintf(stderr, "An error occurs when opening the file %s.\n", name);
        return;
    }

    int  line_nb = 0;
    int  current = 0;
    int64_t  i   = 0;
    int64_t  n   = 0;
    int64_t  p   = 0;
    char line[LINESIZE];
    while (fgets(line, LINESIZE, fptr) != NULL) {
        line_nb++;
        current = 0;
        SKIP_SPACE;
        if (line[current] == '#') 
            continue;

        p = get_int(&current, line, line_nb);
        SKIP_SPACE;
        n = get_int(&current, line, line_nb);
        SKIP_SPACE;
        i = get_int(&current, line, line_nb);
        SKIP_SPACE;
        systemC_time_table[SYSTEMC_INDICE(p, n, i)] += get_int(&current, line, line_nb);
    }

    /* close the file */
    if (fclose(fptr) != 0) {
        fprintf(stderr, "An error occurs when closing the file %s.\n", 
                name);
    }


    for (int k=0 ; k<proc*node*iface ; k++) {
        systemC_time_table[k] /= LOOP;
    }
    return;
}
