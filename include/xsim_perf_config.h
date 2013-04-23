#ifndef _XSIM_PERF_CONFIG_H_
#define _XSIM_PERF_CONFIG_H_

/* if rdtscp is available */
//#define RDTSCP 
/* set to fixe processus on a cpu with affinity */
//#define AFFINITY 

/* set or remove all performance evaluation */
#define PERFORMANCE_EVALUATION     

/* constante for the benchmarks */
#define BENCHMARK                       1
#define OVERHEAD_REPETITION             10
#define COPY_REPETITION                 10



/* set or remove a part of performance evaluation */
/* For the listener */
//#define LISTENER_MSG_QUEUE_LOCK
//#define LISTENER_MSG_QUEUE_LOCK_WAIT
//#define LISTENER_SEND_QUEUE_LOCK
//#define LISTENER_SEND_QUEUE_LOCK_WAIT

//#define LISTENER_RECV_QUEUE_LOCK
//#define LISTENER_RECV_QUEUE_LOCK_WAIT
//#define LISTENER_MSG_BOX_COND_LOCK
//#define LISTENER_MSG_BOX_COND_LOCK_WAIT

/* 
 * Warning: 
 * LISTENER_MSG_EMPTY_LOCK and the next is used in a special case
 * Only 1 function uses it for the measures and is always called to put or 
 * to pop element from the empty_queue of the msg_box 
 */
//#define LISTENER_MSG_EMPTY_LOCK    
//#define LISTENER_MSG_EMPTY_LOCK_WAIT

/* for the node */
//#define NODE_SEND_QUEUE_LOCK
//#define NODE_SEND_QUEUE_LOCK_WAIT

//#define NODE_RECV_QUEUE_LOCK 
//#define NODE_RECV_QUEUE_LOCK_WAIT
//#define NODE_MSG_BOX_COND_LOCK
//#define NODE_MSG_BOX_COND_LOCK_WAIT


/* for global measures */
#define GLOBAL_PROCESSUS_TIME
#define GLOBAL_SIMULATION_TIME
#define GLOBAL_LISTENER_TIME
//#define GLOBAL_USEFUL_TIME

/* for message type counting */
#define COUNT_MSG_TYPE

/* output all the measures taken */
/* this option takes a lot of memory and can create easily memory swapping */
//#define OUTPUT_ALL_MEASURES 


#endif
