#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gnuplot_measure.h>
#include <gnuplot_output.h>
#include <gnuplot_systemC.h>

#define OUTPUT_FILE(str)        "%s/%s_%s_%s.gp",         DATA_REP, begin_name, str, type_name(t)
#define CMD_OUTPUT_FILE(str)    "%s/%s_%s_%s.plot",        CMD_REP, begin_name, str, type_name(type)            

#define INDICE_RAPPORT(n, p)    ((n)-NB_NODE_MIN) + ((p)-NB_PROC_MIN)*(node-NB_NODE_MIN+1)
#define RAPPORT(k)              \
    ((((k)%(node-NB_NODE_MIN+1))+NB_NODE_MIN)(((k)/(node-NB_NODE_MIN+1))+NB_PROC_MIN))


#define LINEWIDTH               2

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


int gnuplot_out_all_measure(char *begin_name, int proc, int node, int iface, int tl)
{
    if (gnuplot_compute_output(proc, node, iface) == EXIT_FAILURE) 
        return EXIT_FAILURE;

    int t       = 0;
    int j       = 0;
    int p       = 0;
    int i       = 0;
    int k       = 0;
    int nb_case = 0;
    rapport_t **table_iface; 

    for (t=0 ; t<nb_type ; t++) {
        /* look if there is data to output */
        if (table_measure_type[t][0][INDICE(NB_NODE_MIN+2, NB_IFACE_MIN)].next == NULL) {
            fprintf(stderr, "Warning, no data corresponding to %s.\n",
                    type_name(t));
            continue; 
        }

        for (int spec=0 ; spec<MAX_SPEC ; spec++) {
            if (((spec == NODE_PROC_RAPPORT) && (t != global_processus_time)) ||
                    ((spec == SPEEDUP_SYSTEMC) && (t != global_processus_time)))
                continue;

            /* build the name of the output file */
            int size = snprintf(NULL, 0, OUTPUT_FILE(spec_output_name(spec)));
            char *name= malloc(sizeof(char) * (size+1));
            sprintf(name, OUTPUT_FILE(spec_output_name(spec)));

            /* open the file */
            FILE* fptr = fopen(name, "w");
            if (fptr == NULL) {
                fprintf(stderr, "An error occurs when opening the gnuplot output file: %s.\n", name);
                return EXIT_FAILURE;
            }

            /* writen the header */
            cell_t *first = table_measure_type[t][NB_PROC_MIN-1];
            fprintf(fptr, "# Version of xsim: %d\n", XSIM_VERSION);
            fprintf(fptr, "# simulation time: %d\n", tl);
            switch (spec) {
            case NODE_PROC_RAPPORT:
                fprintf(fptr, "# nb_node/nb_proc   nb_iface\n");
                fprintf(fptr, "#                   ");
                for (i=NB_IFACE_MIN ; i<=iface ; i++) {
                    fprintf(fptr, "         %02d        ", i);
                }
                break;
            default:
                fprintf(fptr, "# nb_proc   (nb_node, nb_iface)\n");
                fprintf(fptr, "#           ");
                for (j=(NB_NODE_MIN-1)*iface ; j<node*iface ; j++) {
                    if (!NODE_NOT_USED(first[j].nb_node)) {
                        fprintf(fptr, "     (%02d, %02d)      ", first[j].nb_node, first[j].nb_iface);
                    }
                }
                break;
            }
            fprintf(fptr, "\n");

            /* writen the data */
            switch (spec) {
            case NODE_PROC_RAPPORT:
                /* 
                 * A case of table_iface is a liste of measures 
                 * corresponding to a colon in gnuplot 
                 */
                table_iface = malloc(sizeof(rapport_t*) * (iface-NB_IFACE_MIN+1));
                nb_case = gnuplot_calcul_rapport(proc, node, iface, table_iface, t);

                for (k=0 ; k<nb_case ; k++) {
                    fprintf(fptr, "  %2f      ", table_iface[0][k].rapport);
                    for (i=0 ; i<iface-NB_IFACE_MIN+1 ; i++) {
                        fprintf(fptr, "%15"PRIu64"    ", table_iface[i][k].time);
                    }
                    fprintf(fptr, "\n");
                }
                for (i=0 ; i<iface-NB_IFACE_MIN+1 ; i++) {
                    free(table_iface[i]);
                }
                free(table_iface);
                break;
            case SPEEDUP:
                for (p=0 ; p<proc ; p++) {
                    fprintf(fptr, "  %2d      ", (p+1));
                    cell_t *table = table_measure_type[t][p];
                    for (j=(NB_NODE_MIN-1)*iface ; j<node*iface ; j++) {
                        if (!NODE_NOT_USED(first[j].nb_node)) {
                            fprintf(fptr, "%15f    ", (float)first[j].time/(float)table[j].time);
                        }
                    }
                    fprintf(fptr, "\n");
                }
                fprintf(fptr, "\n");
                break;
            case NORMAL:
                for (p=0 ; p<proc ; p++) {
                    fprintf(fptr, "  %2d      ", (p+1));
                    cell_t *table = table_measure_type[t][p];
                    for (j=(NB_NODE_MIN-1)*iface ; j<node*iface ; j++) {
                        if (!NODE_NOT_USED(first[j].nb_node)) {
                            fprintf(fptr, "%15"PRIu64"    ", table[j].time);
                        }
                    }
                    fprintf(fptr, "\n");
                }
                fprintf(fptr, "\n");
                break;
            case SPEEDUP_SYSTEMC:
                for (p=0 ; p<proc ; p++) {
                    fprintf(fptr, "  %2d      ", (p+1));
                    cell_t *table = table_measure_type[t][p];
                    for (j=(NB_NODE_MIN-1)*iface ; j<node*iface ; j++) {
                        if (NODE_NOT_USED(first[j].nb_node)) {
                            continue;
                        }
                        if (table[j].time != 0) {
                            fprintf(fptr, "%15f    ", 
                                    (float)systemC_time_table[SYSTEMC_INDICE(p+NB_PROC_MIN, table[j].nb_node, table[j].nb_iface)]/
                                    (float)table[j].time);
                        } 
//                        else {
//                            fprintf(stdout, "There is no measure if there is %d nodes.\n",
//                                    first[j].nb_node);
//                        }
                    }
                    //fprintf(fptr, "\n#");
                    //for (j=(NB_NODE_MIN-1)*iface ; j<node*iface ; j++) {
                    //    if (NODE_NOT_USED(first[j].nb_node)) {
                    //        continue;
                    //    }
                    //    if (table[j].time != 0) {
                    //        fprintf(fptr, "%15f/%15f   ", 
                    //                (float)systemC_time_table[SYSTEMC_INDICE(p+NB_PROC_MIN, table[j].nb_node, table[j].nb_iface)],
                    //                (float)table[j].time);
                    //    } 
//                  //      else {
//                  //          fprintf(stdout, "There is no measure if there is %d nodes.\n",
//                  //                  first[j].nb_node);
//                  //      }
                    //}
                    fprintf(fptr, "\n");
 
                }
                fprintf(fptr, "\n");

                break;
            default:
                fprintf(stderr, "Error: this type of data to output is not define: %s, nb: %d.\n",
                        spec_output_name(spec), spec);
                break;

            }

            /* close the file */
            if (fclose(fptr) != 0) {
                fprintf(stderr, "An error occurs when closing the gnuplot output file: %s.\n", 
                        name);
            }

            gnuplot_output_command_file(begin_name, spec, name, t, node, iface);

            free(name);
        }
    }

    return EXIT_SUCCESS;
}




int gnuplot_compute_output(int proc, int node, int iface)
{
    int t = 0;
    int p = 0;
    int j = 0;
    for (t=0 ; t<nb_type ; t++) {
        for (p=0 ; p<proc ; p++) {
            cell_t *table = table_measure_type[t][p];
            for (j=(NB_NODE_MIN-1)*iface ; j<node*iface ; j++) {
                uint64_t time = 0;
                uint64_t values_square = 0;
                uint64_t mean_square = 0;
                int cnt       = 0;
                measure_t *m = table[j].next;
                while (m != NULL) {
                    if (!(m->global)) {
                        fprintf(stderr, "Error, a measure was not correctly treated.\n");
                        return EXIT_FAILURE;
                    }
                    time += m->time;
                    values_square = m->time * m->time;
                    m     = m->next;
                    cnt++;
                }
                if (cnt != table[j].nb_measure) {
                    fprintf(stderr, "Error, read %d measures, count %d.\n", 
                            cnt, table[j].nb_measure);
                    return EXIT_FAILURE;
                }

                if (cnt != 0) {
                    time /= cnt;
                    /* compute the standart deviation */
                    mean_square = time*time;
                    values_square = values_square/cnt;
                }
                table[j].time = time;
                table[j].variance = sqrt((long double)values_square - mean_square);


            }
        }
    }

    return EXIT_SUCCESS;
}




int gnuplot_output_command_file(const char *begin_name, int spec, const char* data_name, 
        measure_type_t type, int node, int iface)
{
    /* build the name of the output file */
    int size = snprintf(NULL, 0, CMD_OUTPUT_FILE(spec_output_name(spec)));
    char *name= malloc(sizeof(char) * (size+1));
    sprintf(name, CMD_OUTPUT_FILE(spec_output_name(spec))); 

    /* open the file */
    FILE* fptr = fopen(name, "w");
    if (fptr == NULL) {
        fprintf(stderr, "An error occurs when opening the file: %s.\n", name);
        return EXIT_FAILURE;
    }

    /* writen the header */
    fprintf(fptr, "# Gnuplot script. Launch with \"gnuplot file.plot\"\n");
    fprintf(fptr, "# Uncomment to generate png file\n");
    fprintf(fptr, "# set terminal png\n");
    fprintf(fptr, "# set output '%s/%s_%s_%s.png'\n\n", 
            PNG_REP, begin_name, spec_output_name(spec), type_name(type));

    fprintf(fptr, "# Uncomment to generate pdf file\n");
    fprintf(fptr, "# set terminal postscript enhanced color\n");
    fprintf(fptr, "# set output '| ps2pdf - %s/%s_%s_%s.pdf'\n\n",
            PDF_REP, begin_name, spec_output_name(spec), type_name(type));

    fprintf(fptr, "set title \"%s_%s\"\n", type_name(type), spec_output_name(spec));
    fprintf(fptr, "set style data linespoints\n");
    fprintf(fptr, "set pointsize 1\n"); 
    fprintf(fptr, "set mouse\n");
    //fprintf(fptr, "set size 1.2, 1.2\n");
    switch (spec) {
    case NODE_PROC_RAPPORT:
        fprintf(fptr, "set xlabel \"number of nodes/number of processors\"\n"); 
        break;
    default:
        fprintf(fptr, "set xlabel \"number of processors\"\n"); 
        fprintf(fptr, "set xtics 1, 1\n");
        break;
    }
    if ((spec == NORMAL) || (spec == NODE_PROC_RAPPORT)) {
        if (type >= global_processus_time) { 
            fprintf(fptr, "set ylabel \"total simulation time (ns)\"\n");
        } else {
            fprintf(fptr, "set ylabel \"total simulation time (nb cycles)\"\n");
        }
        fprintf(fptr, "\n\n");
    } else if ((spec == SPEEDUP) || (spec == SPEEDUP_SYSTEMC)) {
        fprintf(fptr, "set ylabel \"Speed-up\"\n");
    }

    /* writen the data */
    fprintf(fptr, "plot ");
    int i = 0;
    int n = 0;
    int skip_column = 0;
    switch (spec) {
    case NODE_PROC_RAPPORT:
        for (i=NB_IFACE_MIN ; i<=iface ; i++) {
            int column = i+1;
            fprintf(fptr,  "\"%s\" using 1:%d title \"nb_iface: %d\" with linespoints linetype %d linewidth %d, \\\n", 
                    data_name, column, i, column, LINEWIDTH);
        }
        break;
    case SPEEDUP_SYSTEMC:
        skip_column = 0;
        for (n=NB_NODE_MIN ; n<=node ; n++) {
            if (NODE_NOT_USED(n)) {
                skip_column++;
                continue;
            }
            for (i=NB_IFACE_MIN ; i<=iface; i++) {
                int column = INDICE(n, i)+1-skip_column;
                fprintf(fptr, "\"%s\" using 1:%d title \"nb_node: %d, nb_iface: %d\" with linespoints linetype %d linewidth %d, \\\n", 
                        data_name, column, n, i, column, LINEWIDTH);
            }
        }
        break;
    default:
        skip_column = 0;
        for (n=NB_NODE_MIN ; n<=node ; n++) {
            if (NODE_NOT_USED(n)) {
                skip_column++;
                continue;
            }
            for (i=NB_IFACE_MIN ; i<=iface; i++) {
                int column = INDICE(n, i)+1-skip_column;
                fprintf(fptr, "\"%s\" using 1:%d title \"nb_node: %d, nb_iface: %d\" with linespoints linetype %d linewidth %d, \\\n", 
                        data_name, column, n, i, column, LINEWIDTH);
            }
        }
        break;
    }
    fseek(fptr, -4, SEEK_CUR);
    fprintf(fptr, " \n\n");
    fprintf(fptr, "pause -1\n");


    /* close the file */
    if (fclose(fptr) != 0) {
        fprintf(stderr, "An error occurs when closing the file: %s.\n", 
                name);
    }

    free(name);
    return EXIT_SUCCESS;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

 
int gnuplot_calcul_rapport(int proc, int node, int iface, 
        rapport_t **table_iface, int type)
{
    int i = 0;
    int p = 0;
    int n = 0;
    int k = 0;
    int l = 0;
    int j = 0;

    int nb_case = (proc-NB_PROC_MIN+1)*(node-NB_NODE_MIN+1); 
    for (i=0 ; i<iface ; i++) {
        table_iface[i] = malloc(sizeof(rapport_t) * nb_case);
    }

    /* Present the measure in an other way */
    for (p=NB_PROC_MIN ; p<=proc ; p++) {
        for (n=NB_NODE_MIN ; n<=node ; n++) {
            for (i=NB_IFACE_MIN ; i<=iface ; i++) {
                table_iface[i-NB_IFACE_MIN][INDICE_RAPPORT(n, p)].time = 
                    table_measure_type[type][p-NB_PROC_MIN][INDICE(n, i)].time;
                table_iface[i-NB_IFACE_MIN][INDICE_RAPPORT(n, p)].rapport = 
                    n/(float)p;
            }
        }
    }

    /* if there is same value for n/p, do the mean and reduce from one case*/
    for (k=0 ; k<nb_case ; k++) {
        int nb_same_rapport = 0;
        /* see if there is an other rapport with the same value in the rest of the table */
        for (l=k+1 ; l<nb_case ; l++) {
            if (table_iface[0][l].rapport == table_iface[0][k].rapport) {
                nb_same_rapport++;
                /* sum */
                for (i=0 ; i<iface-NB_IFACE_MIN+1 ; i++) {
                    table_iface[i][k].time += table_iface[i][l].time;
                }
                /* Shift the tables */
                for (i=0 ; i<iface-NB_IFACE_MIN+1 ; i++) {
                    for (int tmp=l ; tmp<nb_case-1 ; tmp++) {
                        table_iface[i][tmp].time = table_iface[i][tmp+1].time;
                        table_iface[i][tmp].rapport = table_iface[i][tmp+1].rapport;
                    }
                }
                nb_case--;
                l--;
            }
        }
        /* mean */
        if (nb_same_rapport > 0) {
            for (i=0 ; i<iface-NB_IFACE_MIN+1 ; i++) {
                table_iface[i][k].time /= nb_same_rapport;
            }
        }
    }

    /* sort the result from their rapport */
    /* insert in a sorted table */
    for (k=1 ; k<nb_case ; k++) {
        /* insert table_iface[i][k] in table_iface[i][0..k-1] */
        l = 0;
        while ((l<k) && (table_iface[0][l].rapport<table_iface[0][k].rapport)) {
            l++;
        }
        /* insert in place l */
        for (i=0 ; i<iface-NB_IFACE_MIN+1 ; i++) {
            rapport_t tmp     = table_iface[i][k];
            for (j=k ; j>l ; j--) {
                table_iface[i][j] = table_iface[i][j-1];
            }
            table_iface[i][l] = tmp;
        }
    }

    return nb_case;
}

