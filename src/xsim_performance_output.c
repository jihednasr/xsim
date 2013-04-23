#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xsim_perf_config.h>
#include <xsim_performance_output.h>


#define DBG_HDR "xsim:perf_output"
#ifdef XSIM_PERF_OUTPUT_DEBUG
#define DEBUG
#endif /* XSIM_PERF_OUTPUT_DEBUG */

#ifdef XSIM_PERF_OUTPUT_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_PERF_OUTPUT_HDEBUG */

#include <xsim_debug.h>


void xsim_fprintf_area(FILE *fptr, xsim_specific_area_to_measure_t area);
void xsim_fprintf_global_area(FILE *fptr, xsim_global_area_to_measure_t area);
void xsim_fprintf_msg_counter(FILE *fptr);

void xsim_fprintf_msg_counter(FILE *fptr)
{
	fprintf(fptr, "********************************************************************************\n\n");
	fprintf(fptr, "Number of messages with a payload sent: %d \n", 
			msg_type_counter[xsim_has_payload]);
	fprintf(fptr, "Number of null messages sent: %d \n",
			msg_type_counter[xsim_null_msg]);
	fprintf(fptr, "Number of messages sent which ask for time information: %d \n",
			msg_type_counter[xsim_need_time_info]);
	fprintf(fptr, "Number of messages sent to indicate the end of the simulation: %d \n",
			msg_type_counter[xsim_end_of_simulation]);


	return;
}

void xsim_fprintf_global_area(FILE *fptr, xsim_global_area_to_measure_t area)
{
	fprintf(fptr, "********************************************************************************\n");

	switch (area) {
	case global_benchmark:
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
		fprintf(fptr, "- global_benchmark:\n\n");
		break;
#else
		return;
#endif

/*************************** global *******************************************/
	case global_processus_time:
#if defined(PERFORMANCE_EVALUATION) && defined(GLOBAL_PROCESSUS_TIME)
		fprintf(fptr, "- global_processus_time:\n\n");
		break;
#else
		return;
#endif
	case global_simulation_time:
#if defined(PERFORMANCE_EVALUATION) && defined(GLOBAL_SIMULATION_TIME)
		fprintf(fptr, "- global_simulation_time:\n\n");
		break;
#else
		return;
#endif
	case global_listener_time:
#if defined(PERFORMANCE_EVALUATION) && defined(GLOBAL_LISTENER_TIME)
		fprintf(fptr, "- global_listener_time:\n\n");
		break;
#else
		return;
#endif
	case global_sim_lose_time:
#if defined(PERFORMANCE_EVALUATION) && defined(GLOBAL_SIM_LOSE_TIME)
		fprintf(fptr, "- global_sim_lose_time:\n\n");
		break;
#else
		return;
#endif
	case global_useful_time:
#if defined(PERFORMANCE_EVALUATION) && defined(GLOBAL_USEFUL_TIME)
		fprintf(fptr, "- global_useful_time:\n\n");
		break;
#else
		return;
#endif

/****************************** default ***************************************/

	default:
		EMSG("Area given as parameter in xsim_fprintf_global_area is unknown\n");
		return;
	}
/******************************************************************************/

	fprintf(fptr, "Total time spent in this part:		%5lds%9ldns	cycles\n", 
			(long int) (global_counter[area].sum.tv_sec), 
			global_counter[area].sum.tv_nsec); 
	fprintf(fptr, "Number of time this part is used:	%17"PRIu64"\n", 
			global_counter[area].nb_of_passage);
	if (global_counter[area].nb_of_passage != 0) {
		long long int means = global_counter[area].sum.tv_sec*1000000000 +
			global_counter[area].sum.tv_nsec;
		means /= global_counter[area].nb_of_passage;
		fprintf(fptr, "Mean time spent in this part:	\t%5llds%9lldns	cycles\n", 
				means/1000000000, means-(1000000000*(means/1000000000)));
	}

#if defined(PERFORMANCE_EVALUATION) && defined(OUTPUT_ALL_MEASURES)
	xsim_global_measure_t *cursor = global_counter[area].list_measure;
	int num = global_counter[area].nb_of_passage; 

	if (area == global_benchmark) {
		fprintf(fptr, "\nResolution of the CLOCK_OTHERS: %5lds%9ldns\n",
				cursor->begin.tv_sec,
				cursor->begin.tv_nsec);
		fprintf(fptr, "Resolution of the CLOCK_PROC:   %5lds%9ldns\n",
				cursor->end.tv_sec,
				cursor->end.tv_nsec);
		cursor = cursor->next;
	}

	fprintf(fptr, "\nnumber            length                begin               end");
#ifdef AFFINITY
    fprintf(fptr, "       cpu\n");
#else
    fprintf(fptr, "\n");
#endif
	while (cursor != NULL) {
		fprintf(fptr, "%6d	%5lds%9ldns	%5lds%9ldns %5lds%9ldns",
				num,
				(long int) (cursor->length.tv_sec), 
				cursor->length.tv_nsec, 
				(long int) (cursor->begin.tv_sec),
				cursor->begin.tv_nsec,
				(long int) (cursor->end.tv_sec),
				cursor->end.tv_nsec);
#ifdef AFFINITY
        fprintf(fptr, "        %d\n", 
				cursor->cpu);
#else
        fprintf(fptr, "\n");
#endif
		num--;
		cursor = cursor->next;
	}
	fprintf(fptr, "\t(nb of cycles)\n");
#endif

	fprintf(fptr, "\n");

	return;
}



void xsim_fprintf_specific_area(FILE *fptr, xsim_specific_area_to_measure_t area)
{
	fprintf(fptr, "********************************************************************************\n");

	switch (area) {
	case specific_benchmark:
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
		fprintf(fptr, "- benchmark:\n\n");
		break;
#else
		return;
#endif
	case msg_src_sh_benchmark:
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
		fprintf(fptr, "- msg_src_sh_benchmark:\n\n");
		break;
#else
		return;
#endif
	case msg_sh_src_benchmark:
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
		fprintf(fptr, "- msg_sh_src_benchmark:\n\n");
		break;
#else
		return;
#endif
	case copy_src_sh_benchmark:
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
		fprintf(fptr, "- copy_src_sh_benchmark:\n\n");
		break;
#else
		return;
#endif
	case copy_sh_src_benchmark:
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
		fprintf(fptr, "- copy_sh_src_benchmark:\n\n");
		break;
#else
		return;
#endif

/***************************** listener ***************************************/
	case listener_msg_queue_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_QUEUE_LOCK)
		fprintf(fptr, "- listener_msg_queue_lock:\n\n");
		break;
#else
		return;
#endif
	case listener_msg_queue_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_QUEUE_LOCK_WAIT)
		fprintf(fptr, "- listener_msg_queue_lock_wait:\n\n");
		break;
#else
		return;
#endif
	case listener_send_queue_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_SEND_QUEUE_LOCK)
		fprintf(fptr, "- listener_send_queue_lock:\n\n");
		break;
#else
		return;
#endif
	case listener_send_queue_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_SEND_QUEUE_LOCK_WAIT)
		fprintf(fptr, "- listener_send_queue_lock_wait:\n\n");
		break;
#else
		return;
#endif
	case listener_recv_queue_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_RECV_QUEUE_LOCK)
		fprintf(fptr, "- listener_recv_queue_lock:\n\n");
		break;
#else
		return;
#endif
	case listener_recv_queue_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_RECV_QUEUE_LOCK_WAIT)
		fprintf(fptr, "- listener_recv_queue_lock_wait:\n\n");
		break;
#else
		return;
#endif
	case listener_msg_box_cond_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_BOX_COND_LOCK)
		fprintf(fptr, "- listener_msg_box_cond_lock:\n\n");
		break;
#else
		return;
#endif
	case listener_msg_box_cond_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_BOX_COND_LOCK_WAIT)
		fprintf(fptr, "- listener_msg_box_cond_lock_wait:\n\n");
		break;
#else
		return;
#endif
	case listener_msg_empty_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_EMPTY_LOCK)
		fprintf(fptr, "- listener_msg_empty_lock:\n\n");
		break;
#else
		return;
#endif
	case listener_msg_empty_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_EMPTY_LOCK_WAIT)
		fprintf(fptr, "- listener_msg_empty_lock_wait:\n\n");
		break;
#else
		return;
#endif

/***************************** node *******************************************/
	case node_send_queue_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_SEND_QUEUE_LOCK)
		fprintf(fptr, "- node_send_queue_lock:\n\n");
		break;
#else
		return;
#endif
	case node_send_queue_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_SEND_QUEUE_LOCK_WAIT)
        fprintf(fptr, "- node_send_queue_lock_wait:\n\n");
		break;
#else
		return;
#endif
	case node_recv_queue_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_RECV_QUEUE_LOCK)
		fprintf(fptr, "- node_recv_queue_lock:\n\n");
		break;
#else
		return;
#endif
	case node_recv_queue_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_RECV_QUEUE_LOCK_WAIT)
		fprintf(fptr, "- node_recv_queue_lock_wait:\n\n");
		break;
#else
		return;
#endif
	case node_msg_box_cond_lock:
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_MSG_BOX_COND_LOCK)
		fprintf(fptr, "- node_msg_box_cond_lock:\n\n");
		break;
#else
		return;
#endif
	case node_msg_box_cond_lock_wait:
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_MSG_BOX_COND_LOCK_WAIT)
		fprintf(fptr, "- node_msg_box_cond_lock_wait:\n\n");
		break;
#else
		return;
#endif


/****************************** default ***************************************/

	default:
		EMSG("Area given as parameter in xsim_fprintf_specific_area is unknown\n");
		return;
	}

	fprintf(fptr, "Total time spent in this part:		%20"PRIu64"	cycles\n", 
			xsim_specific_counter[area].sum); 
	fprintf(fptr, "Number of time this part is used:	%20"PRIu64"\n", 
			xsim_specific_counter[area].nb_of_passage);
	if ((int64_t)xsim_specific_counter[area].nb_of_passage-(int)xsim_specific_counter[area].error > 0) 
		fprintf(fptr, "Mean time spent in this part:		%20"PRIu64"	cycles\n", 
				xsim_specific_counter[area].sum/(xsim_specific_counter[area].nb_of_passage-xsim_specific_counter[area].error));

#if defined(PERFORMANCE_EVALUATION) && defined(OUTPUT_ALL_MEASURES)
	xsim_specific_measure_t *cursor = xsim_specific_counter[area].list_measure;
	int num = xsim_specific_counter[area].nb_of_passage; 

	fprintf(fptr, "\nnumber                length                    begin                   end");
#ifdef AFFINITY
    fprintf(fptr, "      i  cpu begin    cpu end\n");
#else
    fprintf(fptr, "\n");
#endif
	while (cursor != NULL) {
		fprintf(fptr, "%6d	%21"PRIu64"	%21"PRIu64" %21"PRIu64"",
				num,
				cursor->length, 
				cursor->begin,
				cursor->end);
#ifdef AFFINITY
        fprintf(fptr, "            %d            %d\n", 
				cursor->cpu, cursor->cpu_end);
#else
        fprintf(fptr, "\n");
#endif
		num--;
		cursor = cursor->next;
	}
	fprintf(fptr, "\t(nb of cycles)\n");
#endif

	fprintf(fptr, "\n");

	return;
}

#ifdef PERFORMANCE_EVALUATION
void xsim_perf_output(const char *output_file, uint32_t node_id)
#else
void xsim_perf_output(const char *output_file __attribute__((__unused__)), uint32_t node_id __attribute__((__unused__)))
#endif
{
#ifdef PERFORMANCE_EVALUATION
	DMSG("<%d> Compute the results of the measures\n", node_id);
	xsim_compute_the_results_of_the_measures();

	DMSG("<%d> Output the measures\n", node_id);
	/* get the end of the file name */
	int length = snprintf(NULL, 0, "_node%02d.perf", node_id);
	char end_outputf_name[length+1];
	snprintf(end_outputf_name, length+1, "_node%02d.perf", node_id);

	/* get the beginning of the file name */
	char *of = malloc(sizeof(char) * (length+1+strlen(output_file)));
	strcpy(of, output_file);
	
	/* concatenation of the 2 names */
	strcat(of, end_outputf_name);

	/* open the file */
	FILE* fptr = fopen(of, "w+");
	if (fptr == NULL) {
		EMSG("<%02d> An error occurs when opening the performance evaluation output file.\n", node_id);
        free(of);
		return;
	}

	
	/*  write in the output file */
	fprintf(fptr, "node n°%02d:\n==========\n\n", node_id);

	/* write the result for the different area */
	xsim_fprintf_specific_area(fptr, specific_benchmark);
	xsim_fprintf_specific_area(fptr, msg_src_sh_benchmark);
	xsim_fprintf_specific_area(fptr, msg_sh_src_benchmark);
	xsim_fprintf_specific_area(fptr, copy_src_sh_benchmark);
	xsim_fprintf_specific_area(fptr, copy_sh_src_benchmark);
	xsim_fprintf_specific_area(fptr, listener_msg_queue_lock);
	xsim_fprintf_specific_area(fptr, listener_msg_queue_lock_wait);
	xsim_fprintf_specific_area(fptr, listener_send_queue_lock);
	xsim_fprintf_specific_area(fptr, listener_send_queue_lock_wait);
	xsim_fprintf_specific_area(fptr, listener_recv_queue_lock);
	xsim_fprintf_specific_area(fptr, listener_recv_queue_lock_wait);
	xsim_fprintf_specific_area(fptr, listener_msg_box_cond_lock);
	xsim_fprintf_specific_area(fptr, listener_msg_box_cond_lock_wait);
	xsim_fprintf_specific_area(fptr, listener_msg_empty_lock);
	xsim_fprintf_specific_area(fptr, listener_msg_empty_lock_wait);



	xsim_fprintf_specific_area(fptr, node_send_queue_lock);
	xsim_fprintf_specific_area(fptr, node_send_queue_lock_wait);
	xsim_fprintf_specific_area(fptr, node_recv_queue_lock);
	xsim_fprintf_specific_area(fptr, node_recv_queue_lock_wait);
	xsim_fprintf_specific_area(fptr, node_msg_box_cond_lock);
	xsim_fprintf_specific_area(fptr, node_msg_box_cond_lock_wait);


	xsim_fprintf_global_area(fptr, global_benchmark);
	xsim_fprintf_global_area(fptr, global_processus_time);
	xsim_fprintf_global_area(fptr, global_simulation_time);
	xsim_fprintf_global_area(fptr, global_listener_time);
	xsim_fprintf_global_area(fptr, global_sim_lose_time);
	xsim_fprintf_global_area(fptr, global_useful_time);


	xsim_fprintf_msg_counter(fptr);


	

	/* close the file */
	if (fclose(fptr) != 0) {
		EMSG("<%02d> An error occurs when closing the performance evaluation output file.\n", node_id);
	}

	fprintf(stdout, "<%d> Measures output file writen. \n", node_id);
    free(of);
#endif
	return;
}



