#ifndef _GNUPLOT_READ_H_
#define _GNUPLOT_READ_H_

#include <inttypes.h>

#define LINESIZE     90
#define WORDSIZE     LINESIZE

#define SKIP_SPACE    while ((current < LINESIZE) && \
                             ((line[current] == ' ') || (line[current] == '\t'))) current++;

/* Use it for "select" int get_cycle and get_ns */
#define MEAN        4
#define TOTAL       2

uint64_t  get_cycle(FILE* fptr, int *line_nb, int select);
uint64_t  get_ns(FILE* fptr, int *line_nb, int select);

int       get_title(FILE* fptr, int *line_nb, char *word);
int       get_word(int *current, char *line, char *word);
int64_t   get_int(int *current, char *line, int line_nb);

int       get_nb_cpu(FILE *fptr);


#endif
