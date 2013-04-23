#ifndef _GNUPLOT_OUTPUT_H_ 
#define _GNUPLOT_OUTPUT_H_

#define XSIM_VERSION    0
#define DATA_REP        "gnuplot_data"
#define CMD_REP         "gnuplot_command"
#define MEASURE_REP     "perf_measures"
#define PNG_REP         "png_gnuplot"
#define PDF_REP         "pdf_gnuplot"

typedef struct rapport rapport_t;

struct rapport {
    uint64_t time;
    float    rapport;
};

int gnuplot_out_all_measure(char *begin_name, int proc, int node, int iface, int tl);
int gnuplot_compute_output(int proc, int node, int iface);

int gnuplot_output_command_file(const char *begin_name, int spec, const char* data_name, 
        measure_type_t type, int node, int iface);

int gnuplot_calcul_rapport(int proc, int node, int iface, 
        rapport_t **table_iface, int type);


#define NORMAL                0
#define SPEEDUP               1
#define NODE_PROC_RAPPORT     2
#define SPEEDUP_SYSTEMC       3
#define MAX_SPEC              4
static inline char *spec_output_name(int i)
{
    switch (i) {
        case NORMAL:
            return "execution_time";
        case SPEEDUP:
            return "speedup";
        case NODE_PROC_RAPPORT:
            return "rapport";
        case SPEEDUP_SYSTEMC:
            return "speedup_systemC";
        default:
            return "undefined";
    }
}



#define NODE_NOT_USED(n)           ( ((n) == 2) || ((n) == 3) ||\
                                     ((n) == 5) || ((n) == 6) ||\
                                     ((n) == 7) || ((n) == 8) ||\
                                     ((n) == 10)|| ((n) == 11)||\
                                     ((n) == 12)|| ((n) == 13)||\
                                     ((n) == 14)|| ((n) == 15)||\
                                     ((n) == 17)|| ((n) == 18)||\
                                     ((n) == 19)|| ((n) == 20)||\
                                     ((n) == 21)|| ((n) == 22)||\
                                     ((n) == 23)|| ((n) == 24)||\
                                     ((n) == 26)|| ((n) == 27)||\
                                     ((n) == 28)|| ((n) == 29)||\
                                     ((n) == 30)|| ((n) == 31) )
 
#endif
