#ifndef _GNUPLOT_MEASURE_h_
#define _GNUPLOT_MEASURE_h_

#include <inttypes.h>

#define LOOP         40

#define NB_PROC_MIN  1
#define NB_NODE_MIN  2
#define NB_IFACE_MIN 1

#define INDICE(node, iface)  ((iface)-NB_IFACE_MIN + ((iface_total)*((node)-NB_NODE_MIN+1)))

typedef enum   measure_type measure_type_t;
typedef struct cell         cell_t;
typedef struct measure      measure_t;

enum measure_type {
    lock_wait,
    in_lock,

    /* limit between measure done with RDTSC and global measure */
    global_processus_time,
    global_simulation_time,
    global_listener_time,

    nb_type
};

static inline char *type_name(measure_type_t type)
{
    switch (type) {
        case lock_wait:
            return "lock_wait";
        case in_lock:
            return "in_lock";
        case global_processus_time:
            return "global_processus_time";
        case global_simulation_time:
            return "global_simulation_time";
        case global_listener_time:
            return "global_listener_time";
        default:
            return "not_a_type";
    }
}

struct cell {
    uint64_t    time;
    long double variance;
    int         nb_measure;
    int         nb_iface;
    int         nb_node;
    measure_t  *next;
};

struct measure {
    int         global;
    uint64_t    time;
    int         benchmark;
    measure_t  *next;
};

cell_t **table_measure_type[nb_type];
extern int iface_total;



void gnuplot_init_measure(int proc, int node, int iface);
void gnuplot_free_measure(int proc, int node, int iface);

int  gnuplot_read_all_measure(const char *begin_name, int proc, int node, int iface);
int  gnuplot_read_a_measure(const char *name, int proc, int node, int iface);

void      add_measure(uint64_t time, int benchmark, measure_type_t type, int proc, int node, int iface);


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static inline int NB_X(int n) {
    switch (n) {
        case 2:
            return 2;
        case 4:
            return 2;
        case 6:
            return 3;
        case 8:
            return 4;
        case 9:
            return 3;
        case 10:
            return 5;
        case 12:
            return 4;
        case 15:
            return 5;
        case 16:
            return 4;
        case 18:
            return 6;
        case 20:
            return 5;
        case 21:
            return 7;
        case 24:
            return 6;
        case 25:
            return 5;
        case 28:
            return 7;
        case 30:
            return 6;
        case 32:
            return 8;
        default:
            return 0;
    }
}

static inline int NB_Y(int n) {
    switch (n) {
        case 2:
            return 1;
        case 4:
            return 2;
        case 6:
            return 2;
        case 8:
            return 2;
        case 9:
            return 3;
        case 10:
            return 2;
        case 12:
            return 3;
        case 15:
            return 3;
        case 16:
            return 4;
        case 18:
            return 3;
        case 20:
            return 4;
        case 21:
            return 3;
        case 24:
            return 4;
        case 25:
            return 5;
        case 28:
            return 4;
        case 30:
            return 5;
        case 32:
            return 4;
        default:
            return 0;
    }
}
 
#endif
