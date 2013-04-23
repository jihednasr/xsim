#ifndef _GNUPLOT_SYSTEMC_H_ 
#define _GNUPLOT_SYSTEMC_H_

#define SYSTEMC_INDICE(p, n, i)     (i)-NB_IFACE_MIN + \
                                    ((n)-NB_NODE_MIN)*nb_iface_total + \
                                    ((p)-NB_PROC_MIN)*nb_node_total*nb_iface_total

extern int nb_iface_total;
extern int nb_node_total;

uint64_t *systemC_time_table;


void gnuplot_systemC_init(int proc, int node, int iface);
void gnuplot_systemC_free();
void gnuplot_systemC_read_all_measure(char *name, int proc, int node, int iface);

#endif
