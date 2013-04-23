#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xsim_performance_evaluation.h>
#include <xsim_perf_config.h>
#include <xsim_topology.h>
#include <xsim_node.h>
#include <xsim_listener.h>
#include <xsim_garbage_list.h>
#include <xsim_FIFO_list.h>


#define DBG_HDR "xsim:perf"
#ifdef XSIM_PERF_DEBUG
#define DEBUG
#endif /* XSIM_PERF_DEBUG */

#ifdef XSIM_PERF_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_PERF_HDEBUG */

#include <xsim_debug.h>

xsim_specific_perf_counter_t xsim_specific_counter[number_of_specific_area];
xsim_global_perf_counter_t   global_counter[number_of_global_area];
int                          msg_type_counter[xsim_need_time_info];

void xsim_perf_init()
{
#ifdef PERFORMANCE_EVALUATION
	for (int area=0 ; area<number_of_specific_area ; area++) {
		xsim_specific_counter[area].nb_of_passage  = 0;
		xsim_specific_counter[area].sum            = 0;
#ifdef OUTPUT_ALL_MEASURES
		xsim_specific_counter[area].list_measure   = NULL;
#else
        xsim_specific_counter[area].length         = 0;
#endif
	}

	for (int area=0 ; area<number_of_global_area ; area++) {
		global_counter[area].nb_of_passage  = 0;
		global_counter[area].sum.tv_sec     = 0;
		global_counter[area].sum.tv_nsec    = 0;
		global_counter[area].list_measure   = NULL;
	}

	for (int i=0 ; i<xsim_need_time_info ; i++) {
		msg_type_counter[i] = 0;
	}
	return;
#endif
}

void xsim_perf_fini()
{
#ifdef PERFORMANCE_EVALUATION
#ifdef OUTPUT_ALL_MEASURES
	for (int area=0 ; area<number_of_specific_area ; area++) {
		xsim_specific_measure_t *cursor = xsim_specific_counter[area].list_measure;
        unsigned int cnt = 0;
		while(cursor != NULL) {
			xsim_specific_measure_t *tmp = cursor;
			cursor = cursor->next;
			free(tmp);
            cnt++;
		}
        if (cnt != xsim_specific_counter[area].nb_of_passage)
            fprintf(stderr, "There is not the good count: got %d, expected %"PRIu64".\n",
                    cnt, xsim_specific_counter[area].nb_of_passage);
        xsim_specific_counter[area].nb_of_passage = 0;
        xsim_specific_counter[area].list_measure  = NULL;
	}
#endif

	for (int area=0 ; area<number_of_global_area ; area++) {
		xsim_global_measure_t *cursor = global_counter[area].list_measure;
		while(cursor != NULL) {
			xsim_global_measure_t *tmp = cursor;
			cursor = cursor->next;
			free(tmp);
		}
        global_counter[area].nb_of_passage = 0;
        global_counter[area].list_measure  = NULL;
	}
#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
#define BENCHMARK_MSG_SIZE      20
void xsim_perf_benchmark(xsim_node_t *node)
#else
void xsim_perf_benchmark(xsim_node_t *node __attribute__((__unused__)))
#endif
{
#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
	DMSG("<%d> start of the benchmark\n", node->node_id);
	int i = 0;

	/* Measure overhead = the test count produced by the test program itself */
	for (i = 0; i<OVERHEAD_REPETITION ; i++) {

		xsim_perf_begin_measure(specific_benchmark);

		/* no test code here */

		xsim_perf_end_measure(specific_benchmark);
	}

	/* Measure overhead = the test count produced by the test program itself */
	for (i = 0; i<OVERHEAD_REPETITION ; i++) {

		xsim_perf_begin_global_measure(global_benchmark, CLOCK_OTHERS);

		/* no test code here */

		xsim_perf_end_global_measure(global_benchmark, CLOCK_OTHERS);
	}


	/* Measure precision of the clock */
	xsim_global_measure_t* tmp = malloc(sizeof(xsim_global_measure_t));
	tmp->next                  = global_counter[global_benchmark].list_measure;
	global_counter[global_benchmark].list_measure   = tmp;
	clock_getres(CLOCK_OTHERS, &(global_counter[global_benchmark].list_measure->begin));
	clock_getres(CLOCK_PROC, &(global_counter[global_benchmark].list_measure->end));

	/* Measure cost for message passing */
	DMSG("<%d> Take the lock for the benchmark\n", node->node_id);
	xsim_lock_lock(&node->msg_box->benchmark_lock);

    xsim_msg_list_elt_t *save_current = (xsim_msg_list_elt_t*)(node->current);
	for (i=0 ; i<COPY_REPETITION ; i++) {
		xsim_perf_begin_measure(msg_src_sh_benchmark);

		xsim_msg_t *msg  = xsim_msg_new();
        msg->type = xsim_has_payload;
        msg->size = BENCHMARK_MSG_SIZE;
		xsim_targets_set(&msg->targets, node->node_id);
		xsim_iface_send(node->iface[0], msg);
		xsim_listener_post_msg(node);

		xsim_perf_end_measure(msg_src_sh_benchmark);

        if (i == 0) {
            node->current_time = xsim_topology_travel_time(node->node_id, node->node_id);
        } else {
            node->current_time += BENCHMARK_MSG_SIZE;
        }

		xsim_perf_begin_measure(msg_sh_src_benchmark);

		xsim_listener_read_msg(node);
		xsim_msg_t *tmp = xsim_iface_recv(node->iface[0]);
		xsim_msg_free(tmp);

		xsim_perf_end_measure(msg_sh_src_benchmark);
        node->iface[0]->next_possible_send = 0;
        node->iface[0]->next_possible_recv = 0;
	}

	/* Measure the time for message copy */
	for (i=0 ; i<COPY_REPETITION ; i++) {
		xsim_msg_t *msg  = xsim_msg_new();
		xsim_msg_t *sh = xsim_msg_box_get_empty(node->msg_box);

		xsim_perf_begin_measure(copy_src_sh_benchmark);
		memcpy(sh, msg, sizeof(xsim_msg_t));
		xsim_perf_end_measure(copy_src_sh_benchmark);

		xsim_perf_begin_measure(copy_sh_src_benchmark);
		memcpy(msg, sh, sizeof(xsim_msg_t));
		xsim_perf_end_measure(copy_sh_src_benchmark);


        memset(sh, 0, sizeof(xsim_msg_t)); /* let avoid error for the next of the execution */
        xsim_garbage_list_add(&(node->msg_box->garbage_queue), sh, sh);
		xsim_msg_free(msg);
	}

	DMSG("<%d> start to clean what was modified by the benchmark\n", node->node_id);
	/* clean the node */
	node->seq_id = 0;
    memset(node->seq_msg_table, 0, sizeof(uint32_t) * node->nb_nodes);
	node->current_time = 0;
    node->send_queue->seq_msg_FIFO_add = 0;
    node->send_queue->seq_msg_FIFO_del = 0;
    /* clean the recv_queue of interface 0 */
    node->recv_queue[0]->head.msg.size = 0;
    node->recv_queue[0]->head.msg.real_arrival_time = 0;
    if ((node->recv_queue[0]->head.next != NULL) ||
            (node->recv_queue[0]->tail != &(node->recv_queue[0]->head))) {
        EMSG("In the cleaning of the benchmark, the recv_queue is not totaly cleaned.\n");
    }
	/* clean the msg_box */
	//xsim_targets_clearall(&node->msg_box->read_mark);
	xsim_msg_list_t *msg_queue = &node->msg_box->msg_queue;
    xsim_msg_list_elt_t *begin = save_current->next;
    xsim_msg_list_elt_t *end   = save_current->next;
    while (end->next != NULL) {
        end = end->next;
    }
    save_current->next     = NULL;
    msg_queue->tail        = save_current;
    node->current          = (xsim_msg_t*)save_current;
    xsim_targets_set(&(save_current->msg.mark), node->node_id);
    xsim_garbage_list_add(&(node->msg_box->garbage_queue), 
            (xsim_msg_t*)begin, (xsim_msg_t*)end);
    /* clean the measures which are taken but are not for the benchmark */
	int area = 0;
	for (area=copy_sh_src_benchmark+1 ; area<number_of_specific_area ; area++) {
#ifdef OUTPUT_ALL_MEASURES
		xsim_specific_measure_t *cursor = xsim_specific_counter[area].list_measure;
		while(cursor != NULL) {
			xsim_specific_measure_t *tmp = cursor;
			cursor = cursor->next;
			free(tmp);
		}
		xsim_specific_counter[area].list_measure = NULL;
#else
        xsim_specific_counter[area].length = 0;
#endif
		xsim_specific_counter[area].nb_of_passage = 0;
	}
	/* clean the interface */
    for (i=0 ; i<node->nb_iface ; i++) {
        xsim_iface_clean(node->iface[i]);
    }
	/* clean the msg type counter */
	for (int i=0 ; i<xsim_need_time_info ; i++) {
		msg_type_counter[i] = 0;
	}

	DMSG("<%d> Release the lock for the benchmark\n", node->node_id);
	xsim_lock_unlock(&node->msg_box->benchmark_lock);

#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void xsim_compute_the_results_of_the_measures()
{
	int area = 0;
#ifdef OUTPUT_ALL_MEASURES
	/* for the specific measure : length and sum */
	for (area=0 ; area<number_of_specific_area ; area++) {
		xsim_specific_measure_t *cursor = xsim_specific_counter[area].list_measure;
        int cnt = 0;
        xsim_specific_counter[area].error = 0;
		while (cursor != NULL) {
            cnt++;
            if (cursor->end < cursor->begin) {
                fprintf(stderr, "Warning: the time measure of the end is before its start: start: %"PRIu64", end: %"PRIu64" ; area: %d ; measure %"PRIu64"/%"PRIu64".\n", 
                        cursor->begin, cursor->end, area, 
                        xsim_specific_counter[area].nb_of_passage - cnt, xsim_specific_counter[area].nb_of_passage);
            }
           cursor->length = cursor->end - cursor->begin;
#ifdef AFFINITY
           if (cursor->cpu != cursor->cpu_end) {
               /* Cancel this measure */
               cursor->length = 0; 
               xsim_specific_counter[area].error++;
               fprintf(stderr, "Area %d: cancellation of the measure %"PRIu64"/%"PRIu64" because the cpu has changed\n",
                       area, xsim_specific_counter[area].nb_of_passage - cnt, xsim_specific_counter[area].nb_of_passage);
           }
#endif
			xsim_specific_counter[area].sum += cursor->length;
			cursor = cursor->next;
		}	
	}
#endif

	/* for the global measure : length and sum */
	for (area=0 ; area<number_of_global_area ; area++) {
	/* skip the first measure of the benchmark which is the precision of the clock */
		xsim_global_measure_t *cursor = global_counter[area].list_measure;
		if ((cursor != NULL) && (area == 0)) {
			cursor = cursor->next; 
		}
		while (cursor != 0) {
			cursor->length.tv_sec = cursor->end.tv_sec - cursor->begin.tv_sec;
			if (cursor->end.tv_nsec < cursor->begin.tv_nsec) {
				cursor->length.tv_sec--;
				cursor->length.tv_nsec = 
					(1000000000 + cursor->end.tv_nsec) - cursor->begin.tv_sec;
			} else {
				cursor->length.tv_nsec = 
					cursor->end.tv_nsec - cursor->begin.tv_nsec;
			}

			global_counter[area].sum.tv_sec  += cursor->length.tv_sec;
			global_counter[area].sum.tv_nsec += cursor->length.tv_nsec;
			cursor = cursor->next;
		}	
	}
}


