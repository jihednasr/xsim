#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gnuplot_main_tools.h>

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/



#define DEFAULT_OUTPUT              "output"

#define NB_OPTION_NON_READJUSTABLE  6
#define GP_NAME                     0
#define TL      					1
#define NB_IFACE                    2
#define NB_NODE                     3
#define NB_PROC                     4
#define MODEL                       5
int read_argument(int argc, char **argv, char **gp_name, int *tl,
        int *nb_iface, int *nb_node, int *nb_proc, int *model)
{
	/* 
	 * variable in order to not change variable already set 
	 * and test if enought information
	 */
	int done[NB_OPTION_NON_READJUSTABLE];
	memset(done, 0, NB_OPTION_NON_READJUSTABLE*sizeof(int));

	char *error_type = NULL;

	/* read the arguments */
	for (int i=1 ; i<argc ; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			/* help */
			print_help(argv[0]);
			return EXIT_FAILURE;

		} else if (strcmp(argv[i], "-gpo") == 0) {
			/* gnuplot output file name */ 
			if (done[GP_NAME]) {
				fprintf(stdout, "Gnuplot output file name already set to %s\n", *gp_name);
				return EXIT_FAILURE;
			} else {
				done[GP_NAME] = 1;
				i++;
				if (i < argc) {
                    *gp_name= malloc(sizeof(char) * (strlen(argv[i])+1));
                    strcpy(*gp_name, argv[i]);
				} else {
					error_type = "-gpo";
					goto error;
				}
			}

		} else if (strcmp(argv[i], "-tl") == 0) {
			/* simulation time */ 
			if (done[TL]) {
				fprintf(stdout, "Simulation time already set to %d.\n", *tl);
				return EXIT_FAILURE;
			} else {
				done[TL] = 1;
				i++;
				if (i < argc) {
					*tl = atoi(argv[i]);
					if (*tl <= 0) {
						error_type = "-tl";
						goto error;
					}
				} else {
					error_type = "-tl";
					goto error;
				}
			}

		} else if (strcmp(argv[i], "-nbi") == 0) {
			/* number of interfaces */ 
			if (done[NB_IFACE]) {
				fprintf(stdout, "Number of interfaces already set to %d.\n", *nb_iface);
				return EXIT_FAILURE;
			} else {
				done[NB_IFACE] = 1;
				i++;
				if (i < argc) {
					*nb_iface = atoi(argv[i]);
					if (*nb_iface <= 0) {
						error_type = "-nbi";
						goto error;
					}
				} else {
					error_type = "-nbi";
					goto error;
				}
			}

		} else if (strcmp(argv[i], "-nbn") == 0) {
			/* number of nodes */ 
			if (done[NB_NODE]) {
				fprintf(stdout, "Number of nodes already set to %d.\n", *nb_node);
				return EXIT_FAILURE;
			} else {
				done[NB_NODE] = 1;
				i++;
				if (i < argc) {
					*nb_node = atoi(argv[i]);
					if (*nb_node <= 0) {
						error_type = "-nbn";
						goto error;
					}
				} else {
					error_type = "-nbn";
					goto error;
				}
			}

		} else if (strcmp(argv[i], "-nbp") == 0) {
			/* number of processors */ 
			if (done[NB_PROC]) {
				fprintf(stdout, "Number of processors already set to %d.\n", *nb_proc);
				return EXIT_FAILURE;
			} else {
				done[NB_PROC] = 1;
				i++;
				if (i < argc) {
					*nb_proc = atoi(argv[i]);
					if (*nb_proc <= 0) {
						error_type = "-nbp";
						goto error;
					}
				} else {
					error_type = "-nbp";
					goto error;
				}
			}

		} else if (strcmp(argv[i], "-m") == 0) {
			/* time model used */ 
			if (done[MODEL]) {
				fprintf(stdout, "Time model used already set to %d.\n", *model);
				return EXIT_FAILURE;
			} else {
				done[MODEL] = 1;
				i++;
				if (i < argc) {
					*model = atoi(argv[i]);
					if (*model <= 0) {
						error_type = "-m";
						goto error;
					}
				} else {
					error_type = "-m";
					goto error;
				}
			}

		} else {
			/* reject invalide argument */
			printf("%s is not a valid argument.\nUse -h for help.\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

    for (int i=0 ; i<NB_OPTION_NON_READJUSTABLE ; i++) {
        if (!done[i]) {
            fprintf(stdout, "Error: lack of argument\n");
            return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;

error:
	fprintf(stdout, "Non-correct argument after %s. Use -h for help.\n", error_type);
	return EXIT_FAILURE;
}


void print_help(const char *name)
{
    fprintf(stdout, "Usage: %s -gpo gnuplot_output -tl simulation_time_limit -nbn nb_node -nbp nb_proc -nbi nb_ifacei -m time_model\n", name);
}


