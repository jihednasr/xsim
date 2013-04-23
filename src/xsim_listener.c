/* __HEADER_HERE__ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#include <xsim.h>
#include <xsim_msg_box.h>
#include <xsim_node.h>
#include <xsim_error.h>
#include <xsim_sync.h>
#include <xsim_internals.h>
#include <xsim_time.h>
#include <xsim_perf_measure.h>
#include <xsim_listener.h>
#include <xsim_central_list.h>
#include <xsim_FIFO_list.h>

#define DBG_HDR "xsim:listener"
#ifdef XSIM_LISTENER_DEBUG
#define DEBUG
#endif 

#ifdef XSIM_LISTENER_HDEBUG
#define HUGE_DEBUG
#endif 

#include <xsim_debug.h>


void *xsim_listener(void *arg) 
{
    xsim_perf_begin_global_measure(global_listener_time, CLOCK_OTHERS);

    xsim_node_t *node        = (xsim_node_t *)arg;
    xsim_cond_t *global_cond = &node->msg_box->cond;

#if AFFINITY
    fprintf(stdout, "Listener of node %d is on cpu %d.\n", node->node_id, sched_getcpu());
#endif

    node->list_state = LIST_STATE_RUNNING;
    xsim_barrier_wait(&node->list_barrier);
    while(1) {
        /*
         * Cond wait
         */

        LOCK_LISTENER_MSG_BOX_COND(global_cond);
        if(xsim_FIFO_list_is_empty(node->send_queue)) {

            if(node->list_state == LIST_STATE_STOPPING) {
                DMSG("<%2d> Listener going to die\n", node->node_id);
                UNLOCK_LISTENER_MSG_BOX_COND(global_cond);
                break;
            } else {
                //DMSG("<%2d> Listener read_mark %016"PRIu64"\n", node->node_id, (uint64_t) (node->msg_box->read_mark));
                if(!xsim_central_list_all_read(&node->msg_box->msg_queue, node->current)) {
                    DMSG("<%2d> Listener skipping sleep (due to pending read)\n", node->node_id);
                    UNLOCK_LISTENER_MSG_BOX_COND(global_cond);
                } else {
                    DMSG("<%2d> Listener going to sleep\n", node->node_id);
                    xsim_cond_wait(global_cond);
                    DMSG("<%2d> Listener waken up\n", node->node_id);
                    UNLOCK_LISTENER_MSG_BOX_COND(global_cond);
                }
            }

        } else {
            DMSG("<%2d> Listener skipping sleep (due to pending write)\n", node->node_id);
            UNLOCK_LISTENER_MSG_BOX_COND(global_cond);
        }
        /*
         * Write
         */
        xsim_listener_post_msg(node);
        /*
         * Read 
         */
        xsim_listener_read_msg(node);
    }

    node->list_state = LIST_STATE_DEAD;

    xsim_perf_end_global_measure(global_listener_time, CLOCK_OTHERS);
    return NULL;
}


void xsim_listener_read_msg(xsim_node_t *node) 
{
    xsim_msg_list_t *msg_queue = &node->msg_box->msg_queue;
    xsim_msg_t      *current   = node->current;
    xsim_msg_t      *next_msg  = xsim_msg_list_next(msg_queue, current);

       /* the current msg was already read, so check if there are other msg */
    if (next_msg != NULL) {
        //xsim_targets_atomic_clear(&node->msg_box->read_mark, node->node_id);
        /* Remove the first message if they was already readed */
        xsim_central_list_conditional_del(msg_queue, current, node->node_id, 
                &(node->msg_box->garbage_queue));

        /* Read the next messages */
        if (!xsim_targets_is_set(&next_msg->mark, node->node_id)) {
            //DMSG("next_msg->mark: %"PRIu64"\n", next_msg->mark);
            EMSG("<%02d> The mark was already clear but should not be!\n", 
                    node->node_id);
        } else {
            //DMSG("<%d> (%llu t) Read a message from node %d.\n",
                    //node->node_id, node->current_time, next_msg->src_id);
            xsim_listener_node_incoming_msg(node, next_msg);
            xsim_time_update(node, next_msg);
        }

        /* Read the next messages if there are ones and mark them */
        current  = next_msg;
        next_msg = xsim_msg_list_next(msg_queue, current);
        while(next_msg != NULL) {
            if (((xsim_msg_list_elt_t*)next_msg)->next == 
                    (xsim_msg_list_elt_t*)current)
                EMSG("Error: there is a loop in the list.\n");
            if (!xsim_targets_is_set(&next_msg->mark, node->node_id)) {
                //DMSG("internal_loop: next_msg->mark: %"PRIu64"\n", next_msg->mark);
                EMSG("<%02d> The mark was already clear but should not be!\n", 
                        node->node_id);
            } else {
                xsim_targets_atomic_clear(&current->mark, node->node_id);
                xsim_listener_node_incoming_msg(node, next_msg);
                xsim_time_update(node, next_msg);
            }
            current  = next_msg;
            next_msg = xsim_msg_list_next(msg_queue, current);
        }
    } else {
        DMSG("<%d> No message to read\n", node->node_id);
        //printf("<%d> current: %X ; current->next: %X ; @tail: %X \n",
        //        node->node_id,
        //        (unsigned int)current, 
        //        (unsigned int)(((xsim_msg_list_elt_t*)current)->next),
        //        (unsigned int)&(msg_queue->tail));
        //xsim_msg_printf_list(msg_queue);
        //sleep(1);
    }
    node->current = current;

    return;
}

void xsim_listener_post_msg(xsim_node_t *node) 
{
    DMSG("<%2d> Sending ...\n", node->node_id);
    //xsim_msg_printf_list(node->send_queue);

    if (!xsim_FIFO_list_is_empty(node->send_queue)) {
        xsim_msg_t *msg = NULL;
        xsim_msg_t *new = xsim_msg_box_get_empty(node->msg_box);
        int del = 0;
        while (new != NULL) {
            del = xsim_FIFO_list_del(node->send_queue, &msg);
            if (!del) {
                xsim_msg_box_add_empty(node->msg_box, new);
                break;
            }
            DMSG("<%2d> Posting using ... %p \n", node->node_id, new);
            memcpy(new, msg, sizeof(xsim_msg_t));
            xsim_msg_box_add_central_queue(node->msg_box, new);
            xsim_msg_free(msg);
            msg = NULL;
            new = xsim_msg_box_get_empty(node->msg_box);
        }

        LOCK_LISTENER_MSG_BOX_COND(&node->msg_box->cond);
        //xsim_targets_copy(&node->msg_box->read_mark, &node->msg_box->mark);
        DMSG("<%2d> Cond_sign.\n", node->node_id);
        xsim_cond_sign(&node->msg_box->cond);
        UNLOCK_LISTENER_MSG_BOX_COND(&node->msg_box->cond);
    } else {
        DMSG("<%2d> Send_Queue is empty. \n", node->node_id);
    }
    return;
}


int xsim_listener_node_incoming_msg(xsim_node_t *node, xsim_msg_t *msg) 
{
    DMSG("<%2d> (%"PRIu64" t) Read message <%d-%d>\n", 
            node->node_id, node->current_time, msg->src_id, msg->seq_id);

    /* test if we get the message in the right order */
    if (msg->seq_id != node->seq_msg_table[msg->src_id]) {
        EMSG("<%2d> Forget a message from node %d: get seq_id: %d, expected: %d\n",
                node->node_id, msg->src_id, msg->seq_id, node->seq_msg_table[msg->src_id]);
    }
    node->seq_msg_table[msg->src_id] = msg->seq_id+1;

    if ((msg->type == xsim_has_payload) && 
            (xsim_targets_is_set(&msg->targets, node->node_id))) {

        xsim_msg_t *new  = xsim_msg_new();
        int         x_id = msg->x_id;

        memcpy(new, msg, sizeof(xsim_msg_t));
        new->due_time = xsim_time_due_time(node->iface[msg->x_id], msg); 
        new->arrival_time = node->current_time;
        new->min_table_recv = node->iface[msg->x_id]->min_table_recv;
        /* 
         * this solution is quick to put in place the time_window
         * but not really nice
         */
        //if (node->current_time > new->due_time) {
        //    new->due_time = node->current_time;
        //}

        DMSG("<%2d> Message <%d-%d> is for me\n", node->node_id, msg->src_id, msg->seq_id);

        LOCK_LISTENER_RECV_QUEUE(node->recv_queue[x_id]);

        xsim_msg_list_add_sorted(node->recv_queue[x_id], new, node->current_time);
        node->iface[x_id]->msg_recv++;
        DMSG("<%d/%d> Message receive with arrival time: %"PRIu64" t\n",
                node->node_id, new->x_id, new->real_arrival_time);

        UNLOCK_LISTENER_RECV_QUEUE(node->recv_queue[x_id]);
    }

    return XSIM_SUCCESS;
}
