/* __HEADER_HERE__ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sched.h>

#include <xsim.h>
#include <xsim_node.h>
#include <xsim_error.h>
#include <xsim_sync.h>
#include <xsim_internals.h>
#include <xsim_time.h>
#include <xsim_perf_measure.h>
#include <xsim_listener.h>
#include <xsim_FIFO_list.h>

#define DBG_HDR "xsim:node"
#ifdef XSIM_NODE_DEBUG
#define DEBUG
#endif /* XSIM_NODE_DEBUG */

#ifdef XSIM_NODE_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_NODE_HDEBUG */

#include <xsim_debug.h>



xsim_node_t *
xsim_node_init(xsim_t *xsim, int node_id) {

    int i = 0, ret;
    xsim_msg_box_t *box   = NULL;
    xsim_node_t    *res   = NULL;

    /*
     * Allocation first + vars
     */
    res               = malloc(sizeof(xsim_node_t));
    res->node_id      = node_id;
    res->nb_nodes     = xsim->nb_nodes;
    res->nb_iface     = xsim->nb_x;
    res->seq_id       = 0;
    res->current_time = 0;

    /* Init iface */
    res->iface = malloc(sizeof(xsim_iface_t*) * res->nb_iface);
    for (i=0 ; i<res->nb_iface ; i++) {
        res->iface[i] = xsim_iface_new(res, i);
    }

    /* Init recv_queue */
    res->recv_queue = malloc(sizeof(xsim_msg_list_t *) * xsim->nb_x);
    for(i = 0; i < res->nb_iface; i++) {
        res->recv_queue[i] = xsim_msg_list_new();
    }

    /* Init send_queue */
    res->send_queue = xsim_FIFO_list_new();


    xsim_barrier_init(&res->list_barrier, 2);

    /*
     * Attach shared memory
     */
    box = xsim->shm_addr;
    res->msg_box = box;

    xsim_msg_box_node_init(box, node_id);
    res->current = xsim_msg_list_first(&(box->msg_queue));

    /* Init the table which is used to check if we do not forget message */
    res->seq_msg_table = malloc(sizeof(uint32_t) * res->nb_nodes);
    memset(res->seq_msg_table, 0, sizeof(uint32_t) * res->nb_nodes);

#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
    xsim_barrier_wait(&box->init_barrier);
    /* test the performance evaluation output */
    xsim_perf_benchmark(res);
#endif

    /* 
     * The barrier is usefull to avoid that a listener is running 
     * during the benchmark measure.
     */
    xsim_barrier_wait(&box->init_barrier);
    DMSG("Ready to go\n");  


    res->list_state = LIST_STATE_STARTING;

    ret = pthread_create(&(res->listener), NULL, &xsim_listener, (void *)res);

    if( ret != 0 ) {
        EMSG("Listener creation error\n");
    }
    /*
     * Wait for listener to be started.
     */
    xsim_barrier_wait(&res->list_barrier);
    //DMSG("<%2d> my listener is ready\n", res->node_id);

    return res;
}


int
xsim_node_free(xsim_node_t *node) {

    int i, ret;

    //DMSG("<%2d> will try to kill my listener!!\n", node->node_id);

    /*
     * Broadcasting is not the better way ... but it works !!!
     */
    LOCK_NODE_MSG_BOX_COND(&node->msg_box->cond);
    node->list_state = LIST_STATE_STOPPING;
    xsim_cond_sign(&node->msg_box->cond);
    UNLOCK_NODE_MSG_BOX_COND(&node->msg_box->cond);

    //DMSG("<%2d> will wait my listener now : %x!!\n", node->node_id, (unsigned int)node->listener);

    ret = pthread_join(node->listener, NULL);
    if( ret != 0 ) {
        EMSG("Error on join\n");
    }

    /* Free recv_queue */
    for(i = 0; i < node->nb_iface; i++) {
        xsim_msg_list_free(node->recv_queue[i]);
    }
    free(node->recv_queue);

    /* Free send_queue */
    xsim_FIFO_list_free(node->send_queue);

    xsim_msg_box_node_fini(node->msg_box);

    /* Free iface */
    for (i=0 ; i<node->nb_iface ; i++) {
        xsim_iface_free(node->iface[i]);
    }
    free(node->iface);

    /* Free the seq_id table */
    free(node->seq_msg_table);

    /* Détache the msg_box */
    if (shmdt(node->msg_box) == -1) {
        fprintf(stderr, "shmdt failed\n");
    }

    //DMSG("<%2d> node is dead !!!\n", node->node_id);

    free(node);

    return XSIM_SUCCESS;
}

int
xsim_node_send(xsim_node_t *node, uint8_t x_id, xsim_msg_t *msg) 
{
    msg->x_id   = x_id;
    msg->src_id = node->node_id;
    msg->seq_id = node->seq_id++;

    if (!info_time_already_send) {
        info_time_already_send = 1;
        ask_our_time_info      = 0;
    }

#if defined(PERFORMANCE_EVALUATION) && defined(COUNT_MSG_TYPE)
    msg_type_counter[msg->type]++;
#endif

    DMSG("<%2d> Place message <%d-%d> in the queue ...\n", 
            node->node_id, msg->src_id, msg->seq_id);

    xsim_FIFO_list_add(node->send_queue, msg);

    LOCK_NODE_MSG_BOX_COND(&node->msg_box->cond);
    DMSG("<%2d> Cond_sign.\n", node->node_id);
    xsim_cond_sign(&node->msg_box->cond);
    UNLOCK_NODE_MSG_BOX_COND(&node->msg_box->cond);

    return XSIM_SUCCESS;
}

/*
 * Need that the lock on the recv_queue of the interface
 * is taken before to call this function
 */
xsim_msg_t *
xsim_node_recv(xsim_node_t *node, uint8_t x_id) {

    xsim_msg_t *msg = NULL;

    //LOCK_NODE_RECV_QUEUE(node->recv_queue[x_id]);

    msg = xsim_msg_list_del(node->recv_queue[x_id]);
    if (msg != NULL) {
        node->recv_queue[x_id]->head.msg.real_arrival_time = msg->real_arrival_time;
        node->recv_queue[x_id]->head.msg.size = msg->size;
        node->iface[x_id]->msg_recv--;
        if ((node->iface[x_id]->msg_recv == 0) && 
                !(xsim_msg_list_is_empty(node->recv_queue[x_id]))) {
            EMSG("<%d/%d> Error: msg_recv is equal to 0 but message are still in the recv_queue.\n",
                    node->node_id, x_id);
        }
    }

    //UNLOCK_NODE_RECV_QUEUE(node->recv_queue[x_id]);

    return msg;
}


