/* __HEADER_HERE__ */

#include <xsim_sync.h>
#include <xsim_msg_box.h>
#include <xsim_garbage_list.h>
#include <xsim_central_list.h>
#include <xsim_error.h>
#include <xsim_targets.h>
#include <xsim_perf_config.h>

#define DBG_HDR "xsim:msg_box"
#ifdef XSIM_MSG_BOX_DEBUG
#define DEBUG
#endif

#ifdef XSIM_MSG_BOX_HDEBUG
#define HUGE_DEBUG
#endif 

#include <xsim_debug.h>

int
xsim_msg_box_init(xsim_msg_box_t *box, int nb_nodes) 
{
    int i = 0;
    xsim_msg_list_elt_t *msg_elt = NULL;

    xsim_barrier_init(&box->init_barrier, nb_nodes);
    xsim_cond_init(&box->cond);
    //xsim_targets_clearall(&box->read_mark);

    /* TODO: WARNING: get rid of alignment */
    msg_elt = (xsim_msg_list_elt_t *)( (uint8_t *)box + sizeof(xsim_msg_box_t) );
    /* Link the elements */
    for(i = 0; i < XSIM_MSG_BUFFER_SIZE-1; i++) {
        /* Minor init */
        msg_elt[i].next = &(msg_elt[i+1]);
    }
    /* Init the garbage list */
    xsim_garbage_list_init(&(box->garbage_queue), 
                (xsim_msg_t*)&(msg_elt[0]), 
                (xsim_msg_t*)&(msg_elt[XSIM_MSG_BUFFER_SIZE-1]));

    /* Init of the central_list */
    xsim_msg_t* first_msg = xsim_garbage_list_del(&(box->garbage_queue), &(box->msg_queue));
    if (first_msg == NULL) {
        EMSG("No valid message in the garbage list just after its initialization!!\n");
        return EXIT_FAILURE;
    }
    /* the type of the message has no importance because no listener will read it */
    first_msg->mark = 0;
    //for (int i=0 ; i<nb_nodes ; i++) {
    //    first_msg->mark |= (xsim_targets_t)1<<i;
    //}
    xsim_central_list_init(&(box->msg_queue), first_msg);

#ifdef BENCHMARK
    xsim_lock_init(&box->benchmark_lock);
#endif
    return XSIM_SUCCESS;
}

int xsim_msg_box_node_init(xsim_msg_box_t *box, int node_id) 
{
    xsim_targets_atomic_set(&box->mark, node_id);

    return XSIM_SUCCESS;
}

int
xsim_msg_box_node_fini(xsim_msg_box_t *box __attribute__((unused)) ) 
{
    /* Nothing to do for now */
    /* Kill the listener thread */
    return XSIM_SUCCESS;
}

int
xsim_msg_box_fini(xsim_msg_box_t *box) 
{
    xsim_lock_fini(&box->msg_queue.lock);
    xsim_lock_fini(&box->garbage_queue.lock);

    xsim_barrier_fini(&box->init_barrier);
    xsim_cond_fini(&box->cond);

#ifdef BENCHMARK
    xsim_lock_fini(&box->benchmark_lock);
#endif

    return XSIM_SUCCESS;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

xsim_msg_t *xsim_msg_box_get_empty(xsim_msg_box_t *box) 
{
    xsim_msg_t *msg = xsim_garbage_list_del(&box->garbage_queue, &box->msg_queue);
    if (msg == NULL)
        EMSG("Empty list has no more free message to give !!!\n");
    return msg;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


int
xsim_msg_box_add_central_queue(xsim_msg_box_t *box, xsim_msg_t *msg) 
{
    xsim_targets_copy(&msg->mark, &box->mark);
    xsim_central_list_add(&box->msg_queue, msg);
    return XSIM_SUCCESS;
}

