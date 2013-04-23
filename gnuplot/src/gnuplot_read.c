#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <gnuplot_read.h>

uint64_t get_cycle(FILE* fptr, int *line_nb, int select)
{
    int i = 0;

    /* variable for the file reading */
    char     line[LINESIZE];
    char     word[LINESIZE];
    int      current      = 0;

    for (i=0 ; i<select ; i++) {
        if (fgets(line, LINESIZE, fptr) == NULL) {
            return 0;
        }
        (*line_nb)++;
    }
    for (i=0 ; i<6 ; i++) {
        SKIP_SPACE;
        if(get_word(&current, line, word) != 0) {
            return 0;
        }
    }
    current++;
    SKIP_SPACE;
    int64_t time = get_int(&current, line, *line_nb);
    if (time < 0) {
        fprintf(stderr, "Error: expected a positif number get: %"PRId64".\n", time);
        return 0;
    }

    return (uint64_t) time;
}


uint64_t get_ns(FILE* fptr, int *line_nb, int select)
{
    int i = 0;

    /* variable for the file reading */
    char     line[LINESIZE];
    char     word[LINESIZE];
    int      current      = 0;

    for (i=0 ; i<select ; i++) {
        if (fgets(line, LINESIZE, fptr) == NULL) {
            return 0;
        }
        (*line_nb)++;
    }
    for (i=0 ; i<6 ; i++) {
        SKIP_SPACE;
        if(get_word(&current, line, word) != 0) {
            return 0;
        }
    }
    current++;
    SKIP_SPACE;
    int64_t sec = get_int(&current, line, *line_nb);
    if (sec < 0) {
        fprintf(stderr, "Error: expected a positif number get: %"PRId64".\n", sec);
        return 0;
    }
    if (line[current] != 's') {
        fprintf(stderr, "Error: expected 's', get %c.\n", line[current]);
        return 0;
    }
    current++;
    SKIP_SPACE;
    int64_t nsec = get_int(&current, line, *line_nb);
    if (nsec < 0) {
        fprintf(stderr, "Error: expected a positif number get: %"PRId64".\n", nsec);
        return 0;
    }
    return (uint64_t) (sec*1000000000 + nsec);
}


int  get_title(FILE* fptr, int *line_nb, char *word)
{
    int  current = 0;
    char line[LINESIZE];
    while (fgets(line, LINESIZE, fptr) != NULL) {
        (*line_nb)++;
        if (line[0] == '-') {
            current++;
            SKIP_SPACE;
            get_word(&current, line, word);
            return 0;
        }
    }
    return -1;
}



int  get_word(int *current, char *line, char *word)
{
    int word_current = 0;
    while ((line[*current] != '\n') && (line[*current] != '\t') && 
           (line[*current] != ' ')  && (line[*current] != ':')  &&
           (word_current < WORDSIZE)) {
        word[word_current] = line[*current];
        word_current++;
        (*current)++;
    }
    if (word_current == WORDSIZE)
        return -1;

    word[word_current] = '\0';

    return 0;
}

int get_nb_cpu(FILE *fptr)
{
    int  current = 0;
    int line_nb = 1;
    char line[LINESIZE] = "";
    int size = 0;
    /* read all the file until the end */
    while ((size = fread(line, sizeof(char), LINESIZE, fptr)) == LINESIZE) {
        line_nb++;
    }
    current = size;
    /* find the last cpu id and put current on the first number */
    while ((current >= 0) && (line[current] != '-')) {
        current--;
    }
    current++;

    int res = (int) get_int(&current, line, line_nb);
    if (res < 0) {
        fprintf(stderr, "Error when reading the cpu information\n");
        return -1;
    }
    return res + 1;

}

int64_t get_int(int *current, char *line, int line_nb)
{
    uint64_t res = 0;

    switch (line[*current]) {
    case '0':
        break;
    case '1':
        break;
    case '2':
        break; 
    case '3': 
        break;
    case '4': 
        break;
    case '5': 
        break;
    case '6': 
        break;
    case '7': 
        break;
    case '8': 
        break;
    case '9':
        break;
    default:
        fprintf(stderr, "line %d: expected a number, find: %c.\n", line_nb, line[*current]);
        return -1;
    }


    while (1) {
        switch (line[*current]) {
        case '0':
            res *= 10;
            break;
        case '1':
            res *= 10;
            res += 1;
            break;
        case '2':
            res *= 10;
            res += 2;
            break;
        case '3':
            res *= 10;
            res += 3;
            break;
        case '4':
            res *= 10;
            res += 4;
            break;
        case '5':
            res *= 10;
            res += 5;
            break;
        case '6':
            res *= 10;
            res += 6;
            break;
        case '7':
            res *= 10;
            res += 7;
            break;
        case '8':
            res *= 10;
            res += 8;
            break;
        case '9':
            res *= 10;
            res += 9;
            break;
        default:
            return res;
        }
        (*current)++;
    }
}
