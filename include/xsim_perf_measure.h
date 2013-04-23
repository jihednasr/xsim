#ifndef _XSIM_PERF_MEASURE_H_
#define _XSIM_PERF_MEASURE_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include <xsim_perf_config.h>
#include <xsim_sync.h>
#include <xsim_msg.h>
#include <xsim_performance_evaluation.h>



/************************** LISTENER_MSG_QUEUE ********************************/

static inline void LOCK_LISTENER_MSG_QUEUE(xsim_msg_list_t *msg_queue)
{
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_QUEUE_LOCK_WAIT)
	/* msg_queue lock wait */
	xsim_perf_begin_measure(listener_msg_queue_lock_wait);
	xsim_lock_lock(&msg_queue->lock);
	xsim_perf_end_measure(listener_msg_queue_lock_wait);
#else
	xsim_lock_lock(&msg_queue->lock);
#endif
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_QUEUE_LOCK)
	/* msg_queue lock */
	xsim_perf_begin_measure(listener_msg_queue_lock);
#endif
}

static inline void UNLOCK_LISTENER_MSG_QUEUE(xsim_msg_list_t *msg_queue)
{
	xsim_lock_unlock(&msg_queue->lock);
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_QUEUE_LOCK)
	xsim_perf_end_measure(listener_msg_queue_lock);
#endif
}

/************************** LISTENER_SEND_QUEUE *******************************/

static inline void LOCK_LISTENER_SEND_QUEUE(xsim_msg_list_t *send_queue)
{
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_SEND_QUEUE_LOCK_WAIT)
	/* Send_queue lock wait */
	xsim_perf_begin_measure(listener_send_queue_lock_wait);
	xsim_lock_lock(&send_queue->lock);
	xsim_perf_end_measure(listener_send_queue_lock_wait);
#else
	xsim_lock_lock(&send_queue->lock);
#endif

#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_SEND_QUEUE_LOCK)
	/* Send_queue lock */
	xsim_perf_begin_measure(listener_send_queue_lock);
#endif
}

static inline void UNLOCK_LISTENER_SEND_QUEUE(xsim_msg_list_t *send_queue)
{
	xsim_lock_unlock(&send_queue->lock);
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_SEND_QUEUE_LOCK)
	/* Send_queue lock end */
	xsim_perf_end_measure(listener_send_queue_lock);
#endif
}


/************************** LISTENER_RECV_QUEUE *******************************/

static inline void LOCK_LISTENER_RECV_QUEUE(xsim_msg_list_t *recv_queue)
{
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_RECV_QUEUE_LOCK_WAIT)
	xsim_perf_begin_measure(listener_recv_queue_lock_wait);
	xsim_msg_list_lock(recv_queue);
	xsim_perf_end_measure(listener_recv_queue_lock_wait);
#else
	xsim_msg_list_lock(recv_queue);
#endif

#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_RECV_QUEUE_LOCK)
	xsim_perf_begin_measure(listener_recv_queue_lock);
#endif
}

static inline void UNLOCK_LISTENER_RECV_QUEUE(xsim_msg_list_t *recv_queue)
{
	xsim_msg_list_unlock(recv_queue);
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_RECV_QUEUE_LOCK)
	xsim_perf_end_measure(listener_recv_queue_lock);
#endif
}

/************************* LISTENER_MSG_BOX_COND ******************************/

static inline void LOCK_LISTENER_MSG_BOX_COND(xsim_cond_t *cond)
{
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_BOX_COND_LOCK_WAIT)
	xsim_perf_begin_measure(listener_msg_box_cond_lock_wait);
	xsim_cond_lock(cond);
	xsim_perf_end_measure(listener_msg_box_cond_lock_wait);
#else
	xsim_cond_lock(cond);
#endif

#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_BOX_COND_LOCK)
	xsim_perf_begin_measure(listener_msg_box_cond_lock);
#endif
}

static inline void UNLOCK_LISTENER_MSG_BOX_COND(xsim_cond_t *cond)
{
	xsim_cond_unlock(cond);
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_BOX_COND_LOCK)
	xsim_perf_end_measure(listener_msg_box_cond_lock);
#endif
}

/************************** LISTENER_MSG_EMPTY ********************************/

static inline void LOCK_LISTENER_MSG_EMPTY(xsim_msg_list_t *msg_empty)
{
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_EMPTY_LOCK_WAIT)
	/* msg_empty lock wait */
	xsim_perf_begin_measure(listener_msg_empty_lock_wait);
	xsim_lock_lock(&msg_empty->lock);
	xsim_perf_end_measure(listener_msg_empty_lock_wait);
#else
	xsim_lock_lock(&msg_empty->lock);
#endif
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_EMPTY_LOCK)
	/* msg_empty lock */
	xsim_perf_begin_measure(listener_msg_empty_lock);
#endif
}

static inline void UNLOCK_LISTENER_MSG_EMPTY(xsim_msg_list_t *msg_empty)
{
	xsim_lock_unlock(&msg_empty->lock);
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_MSG_EMPTY_LOCK)
	xsim_perf_end_measure(listener_msg_empty_lock);
#endif
}


/************************** LISTENER_TIME_COND ********************************/

static inline void LOCK_LISTENER_TIME_COND(xsim_cond_t *cond)
{
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_TIME_COND_LOCK_WAIT)
	/* msg_empty lock wait */
	xsim_perf_begin_measure(listener_time_cond_lock_wait);
	xsim_cond_lock(cond);
	xsim_perf_end_measure(listener_time_cond_lock_wait);
#else
	xsim_cond_lock(cond);
#endif
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_TIME_COND_LOCK)
	/* msg_empty lock */
	xsim_perf_begin_measure(listener_time_cond_lock);
#endif
}

static inline void UNLOCK_LISTENER_TIME_COND(xsim_cond_t *cond)
{
	xsim_cond_unlock(cond);
#if defined(PERFORMANCE_EVALUATION) && defined(LISTENER_TIME_COND_LOCK)
	xsim_perf_end_measure(listener_time_cond_lock);
#endif
}

/******************************************************************************/
/***************************** FIN LISTENER ***********************************/
/******************************************************************************/

/**************************** NODE_SEND_QUEUE *********************************/

static inline void LOCK_NODE_SEND_QUEUE(xsim_msg_list_t *send_queue)
{
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_SEND_QUEUE_LOCK_WAIT)
	xsim_perf_begin_measure(node_send_queue_lock_wait);
	xsim_lock_lock(&send_queue->lock);
	xsim_perf_end_measure(node_send_queue_lock_wait);
#else
	xsim_lock_lock(&send_queue->lock);
#endif

#if defined(PERFORMANCE_EVALUATION) && defined(NODE_SEND_QUEUE_LOCK)
	xsim_perf_begin_measure(node_send_queue_lock);
#endif
}

static inline void UNLOCK_NODE_SEND_QUEUE(xsim_msg_list_t *send_queue)
{
	xsim_lock_unlock(&send_queue->lock);
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_SEND_QUEUE_LOCK)
	xsim_perf_end_measure(node_send_queue_lock);
#endif
}


/************************** NODE_RECV_QUEUE *******************************/

static inline void LOCK_NODE_RECV_QUEUE(xsim_msg_list_t *recv_queue)
{
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_RECV_QUEUE_LOCK_WAIT)
	xsim_perf_begin_measure(node_recv_queue_lock_wait);
	xsim_msg_list_lock(recv_queue);
	xsim_perf_end_measure(node_recv_queue_lock_wait);
#else
	xsim_msg_list_lock(recv_queue);
#endif

#if defined(PERFORMANCE_EVALUATION) && defined(NODE_RECV_QUEUE_LOCK)
	xsim_perf_begin_measure(node_recv_queue_lock);
#endif
}

static inline void UNLOCK_NODE_RECV_QUEUE(xsim_msg_list_t *recv_queue)
{
	xsim_msg_list_unlock(recv_queue);
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_RECV_QUEUE_LOCK)
	xsim_perf_end_measure(node_recv_queue_lock);
#endif
}

/************************* NODE_MSG_BOX_COND ******************************/

static inline void LOCK_NODE_MSG_BOX_COND(xsim_cond_t *cond)
{
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_MSG_BOX_COND_LOCK_WAIT)
	xsim_perf_begin_measure(node_msg_box_cond_lock_wait);
	xsim_cond_lock(cond);
	xsim_perf_end_measure(node_msg_box_cond_lock_wait);
#else
	xsim_cond_lock(cond);
#endif

#if defined(PERFORMANCE_EVALUATION) && defined(NODE_MSG_BOX_COND_LOCK)
	xsim_perf_begin_measure(node_msg_box_cond_lock);
#endif
}

static inline void UNLOCK_NODE_MSG_BOX_COND(xsim_cond_t *cond)
{
	xsim_cond_unlock(cond);
#if defined(PERFORMANCE_EVALUATION) && defined(NODE_MSG_BOX_COND_LOCK)
	xsim_perf_end_measure(node_msg_box_cond_lock);
#endif
}



/******************************************************************************/
/******************************** FIN NODE ************************************/
/******************************************************************************/


#ifdef __cplusplus
}
#endif 



#endif
