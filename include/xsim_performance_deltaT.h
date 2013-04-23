#ifndef _XSIM_PERFORMANCE_DELTAT_H_
#define _XSIM_PERFORMANCE_DELTAT_H_

typedef struct xsim_deltaT xsim_deltaT_t;

struct xsim_deltaT {
    xsim_barrier_t  barrier1;

    int             nb_proc;
    int64_t         *time;
    int64_t        first_time;
};


#define NB_PROG         7
static inline size_t xsim_sizeof_table(int nb_proc)
{
    return (3 * sizeof(int64_t) * NB_PROG * (nb_proc-1));
}
static inline size_t xsim_sizeof_xsim_deltaT_t(int nb_proc) // __attribute__((__unused__)))
{
    return (sizeof(xsim_barrier_t) + sizeof(int) + sizeof(int64_t*) + 
           xsim_sizeof_table(nb_proc)); 
}

int xsim_nb_proc();
void measure_delta_t(int id, xsim_deltaT_t *table);
int xsim_evaluate_diff_time_between_proc();

#endif
