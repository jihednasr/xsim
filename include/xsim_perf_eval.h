#ifndef _XSIM_PERF_EVAL_H_
#define _XSIM_PERF_EVAL_H_

//#define _POSIX_C_SOURCE 199309L

#include <inttypes.h>
#include <sched.h>
#include <time.h>
#include <xsim_perf_config.h>
#include <xsim_node.h>

#include <string.h>

//#define CLOCK_PROC      CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROC      CLOCK_MONOTONIC
#define CLOCK_OTHERS    CLOCK_THREAD_CPUTIME_ID

typedef struct timespec	                    xsim_global_time_t;
typedef struct xsim_global_measure          xsim_global_measure_t;
typedef struct xsim_global_perf_counter	    xsim_global_perf_counter_t;

typedef int64_t                             xsim_cycle_time_t;
typedef struct xsim_specific_measure        xsim_specific_measure_t;
typedef struct xsim_specific_perf_counter   xsim_specific_perf_counter_t;


/* structure for global time measures */
enum xsim_global_area_to_measure {
	global_benchmark = 0,

	global_processus_time,
	global_simulation_time,
	global_listener_time,
	global_sim_lose_time,
	global_useful_time, // =5

	number_of_global_area
};
typedef enum xsim_global_area_to_measure    xsim_global_area_to_measure_t;

struct xsim_global_measure {
#ifdef AFFINITY
	int cpu;
#endif
	xsim_global_time_t begin;
	xsim_global_time_t end;
	xsim_global_time_t length;
	xsim_global_measure_t*   next;
};

struct xsim_global_perf_counter {
	xsim_global_measure_t*   list_measure;
	xsim_global_time_t       sum;
	uint64_t                 nb_of_passage;
};	


/* structure for specific time measures */
enum xsim_specific_area_to_measure {
	specific_benchmark = 0,
	msg_src_sh_benchmark,
	msg_sh_src_benchmark,
	copy_src_sh_benchmark,
	copy_sh_src_benchmark,

	listener_msg_queue_lock, // =5
	listener_msg_queue_lock_wait,
	listener_send_queue_lock,
	listener_send_queue_lock_wait,
	listener_recv_queue_lock,
	listener_recv_queue_lock_wait, // =10
	listener_msg_box_cond_lock,
	listener_msg_box_cond_lock_wait,
	listener_msg_empty_lock,
	listener_msg_empty_lock_wait,

	node_send_queue_lock, // =15
	node_send_queue_lock_wait,
	node_recv_queue_lock,
	node_recv_queue_lock_wait,
	node_msg_box_cond_lock,
	node_msg_box_cond_lock_wait, // =20

	/* 
	 * must always be at the end of the enum and 
	 * never used as a real area 
	 */
	number_of_specific_area		
};
typedef enum xsim_specific_area_to_measure  xsim_specific_area_to_measure_t;

struct xsim_specific_measure {
	uint32_t                    cpu;
    uint32_t                    cpu_end;
	xsim_cycle_time_t           begin;
	xsim_cycle_time_t           end;
	xsim_cycle_time_t           length;
	xsim_specific_measure_t*    next;
};

struct xsim_specific_perf_counter {
#ifdef ALL_MEASURE
    xsim_specific_measure_t    *list_measure;
#else
	uint32_t                    cpu;
    uint32_t                    cpu_end;
	xsim_cycle_time_t           length;
#endif
	xsim_cycle_time_t           sum;
    unsigned int                error;
	uint64_t                    nb_of_passage;
};	



#endif
