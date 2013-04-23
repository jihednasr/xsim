#ifndef _GNUPLOT_MAIN_TOOLS_H_ 
#define _GNUPLOT_MAIN_TOOLS_H_

void print_help(const char *name);
int read_argument(int argc, char **argv, char **gp_name, int *tl,
        int *nb_iface, int *nb_node, int *nb_proc, int *model);

#endif
