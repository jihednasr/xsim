#include <stdlib.h>
#include <stdio.h>

#include <xsim_topology.h>
#include <xsim_error.h>



#define DBG_HDR "xsim:topology"
#ifdef XSIM_TOPOLOGY_DEBUG
#define DEBUG
#endif /* XSIM_TOPOLOGY_DEBUG */

#ifdef XSIM_TOPOLOGY_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_TOPOLOGY_HDEBUG */

#include <xsim_debug.h>

#ifndef TOPOLOGY_INFORMATION
#define TOPOLOGY_INFORMATION    1
#endif

sim_time **topology;

void xsim_topology_init(xsim_t *xsim)
{
	topology = malloc(sizeof(sim_time*) * xsim->nb_nodes);
	for (int i=0 ; i<xsim->nb_nodes ; i++) {
		topology[i] = malloc(sizeof(sim_time) * xsim->nb_nodes);
		for (int j=0 ; j<xsim->nb_nodes ; j++) {
			topology[i][j] = 1;
		}
	}

    if (xsim->topology != NULL) {
        xsim_topology_load_info(xsim->topology, xsim->nb_nodes);
#ifdef TOPOLOGY_INFORMATION
        for (int i=0 ; i<xsim->nb_nodes ; i++) {
            for (int j=0 ; j<xsim->nb_nodes ; j++) {
                fprintf(stdout, "node %d -> node %d : %"PRIu64" t \n",
                        i, j, topology[i][j]);
            }
        }
#endif
    }
    return;
}

void xsim_topology_free(xsim_t *xsim)
{
	for (int i=0 ; i<xsim->nb_nodes ; i++) {
		free(topology[i]);
	}
	free(topology);

    return;
}


sim_time xsim_topology_travel_time(uint32_t src, uint32_t dest)
{
	return topology[src][dest];
}





/* Function and macro to read the topology file */


#define LINESIZE           81

#define SKIP_SPACE    while ((current < LINESIZE) && (line[current] == ' ')) current++;
#define NEXT_TOKEN    do { \
                          SKIP_SPACE;\
                          if ((line[current] == '#') || (line[current] == '\n')) {  \
                              EMSG("The line is not ended as expected: %s.\n", line); \
			                  continue; \
					      } \
                      } while (0)

static inline int get_int(int *current, char *line, int line_nb)
{
    int res = 0;

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
        EMSG("line %d: expected a number, find: %c.\n", line_nb, line[*current]);
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

void xsim_topology_load_info(char *file, int nb_node)
{
	FILE *fptr = fopen(file, "r");
	if (fptr == NULL) {
		EMSG("Error when loading file: %s.\n", file);
		return;
	}

	int current = 0;
	int i       = 0;
	int j       = 0;
    int time    = 1;
    int line_nb = 0;
    char *line = malloc(sizeof(char) * LINESIZE);
	while (fgets(line, LINESIZE, fptr) != NULL) {
        line_nb++;
		current = 0;
		SKIP_SPACE;

		if ((line[current] == '#')           /* It is a commentaire */
				|| (line[current] == '\n'))  /* It is an empty line */
			continue;

		i = get_int(&current, line, line_nb); 
        if (i < 0)
            continue;
		if (i >= nb_node) {/* This node does not match the configuration */
			EMSG("line %d: the node number is too high compare to the configuration given as parameters: got %d .\n", line_nb, i);
			continue;
		}

        NEXT_TOKEN;

        if (current > LINESIZE-2) {
            EMSG("line %d: line too long: max characters in one line: %d\n", line_nb, LINESIZE);
            continue;
        }
		if ((line[current] != '-') || (line[current+1] != '>')) {
			EMSG("line %d: Token \"->\" expected. Find: %c%c.\n", line_nb, line[current], line[current+1]);
			continue;
		}
        current += 2;
        
        NEXT_TOKEN;

        j = get_int(&current, line, line_nb);
        if (j < 0)
            continue;
		if (j >= nb_node) {/* This node does not match the configuration */
			EMSG("line %d: the node number is too high compare to the configuration given as parameters: got %d.\n", line_nb, j);
			continue;
		}

        NEXT_TOKEN;

        if (line[current] != ':') {
            EMSG("line %d: Token ':' expected. Find: %c.\n", line_nb, line[current]);
            continue;
        }
        current++;
        
        NEXT_TOKEN;

        time = get_int(&current, line, line_nb);
		if (time <= 0) {/* This travel time is not possible */
			EMSG("line %d: the travel time read is not possible: %d.\n", line_nb, time);
			continue;
		}

        /* The rest of the line is not important - skip it and memorized the result*/
        topology[i][j] = time;
	}
    free(line);

	fclose(fptr);
	return;
}
