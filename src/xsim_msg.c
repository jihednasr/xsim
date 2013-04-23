/* __HEADER_HERE__ */

#include <stdlib.h>
#include <xsim_msg.h>
#include <xsim_sync.h>
#include <xsim_error.h>
#include <xsim_topology.h>

#define DBG_HDR "xsim:msg"
#ifdef XSIM_MSG_DEBUG
#define DEBUG
#endif /* XSIM_MSG_DEBUG */

#ifdef XSIM_MSG_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_MSG_HDEBUG */

#include <xsim_debug.h>
#include <xsim_perf_measure.h>


int xsim_msg_list_is_empty(xsim_msg_list_t *list)
{
    return (list->head.next == NULL);
}

xsim_msg_t* xsim_msg_list_first(xsim_msg_list_t *list)
{
    return (xsim_msg_t*) (list->head.next);
}

xsim_msg_t* xsim_msg_list_next(xsim_msg_list_t *list __attribute__((__unused__)), xsim_msg_t *msg)
{
    xsim_msg_list_elt_t* elt = (xsim_msg_list_elt_t*) msg;
    return (xsim_msg_t*) (elt->next);
}


int xsim_msg_list_init(xsim_msg_list_t *list)
{
    (&list->head)->next = NULL;
    list->tail          = &(list->head);
    xsim_lock_init(&list->lock);
    list->head.msg.size = 0;
    list->head.msg.real_arrival_time = 0;
    return XSIM_SUCCESS;
}

xsim_msg_list_t *
xsim_msg_list_new(void) {

    xsim_msg_list_t *res = malloc(sizeof(xsim_msg_list_t));  
    xsim_msg_list_init(res);

    return res;
}

int xsim_msg_list_fini(xsim_msg_list_t *list)
{
    xsim_msg_list_elt_t *elt = NULL;

    xsim_lock_lock(&list->lock);

    elt = list->head.next;
    while(elt != NULL) {
        xsim_msg_t *tmp = (xsim_msg_t *)elt;
        elt = elt->next;
        xsim_msg_free(tmp);
    }
    xsim_lock_unlock(&list->lock);

    xsim_lock_fini(&list->lock);

    return XSIM_SUCCESS;
}


void
xsim_msg_list_free(xsim_msg_list_t *list) {

    xsim_msg_list_fini(list);
    free(list);
    return;
}

/* add in tail */
void
xsim_msg_list_add(xsim_msg_list_t *list, xsim_msg_t *msg) {

    xsim_msg_list_elt_t *elt = (xsim_msg_list_elt_t *)msg;

#if 0
    DMSG("<msg_list> adding %p (to %p)\n", elt, list->head);
#endif

    elt->next        = NULL;
    list->tail->next = elt;
    list->tail       = elt;

    return;
}

xsim_msg_t *
xsim_msg_list_del(xsim_msg_list_t *list) {

    xsim_msg_list_elt_t *elt = NULL;

    if (list->head.next != NULL) {
        elt = list->head.next;
        list->head.next = elt->next;

        if(elt == list->tail)
            list->tail = &(list->head);

        elt->next = NULL;
    }

#if 0
    if(elt) {
        DMSG("<msg_list> deleting %p (to %p)\n", elt, elt->next);
    }
#endif

    return (xsim_msg_t *)elt;
}

/* The list must have at least one element */
void
xsim_msg_update_list_time(xsim_msg_list_t *list, sim_time current_time)
{
    xsim_msg_list_elt_t *cursor = (&(list->head))->next;

    /* set the real_arrival_time of the first elt in function of current_time */
    if (current_time > cursor->msg.real_arrival_time) {
        if (current_time > cursor->msg.due_time + time_window + SYNCHRONIZATION_PERIODE) {
            EMSG("The time window is exceeded of %lld time. The message was send by %d.\n", 
                    (long long int) (current_time - cursor->msg.due_time - time_window),
                    cursor->msg.src_id);
        }
        cursor->msg.real_arrival_time = current_time;
    }

    /* update the real_arrival_time of the rest of the list */ 
    sim_time next_arrival_time = cursor->msg.real_arrival_time + cursor->msg.size;
    cursor = cursor->next;
    DMSG("Next arrival time: %llut, size message: %"PRIu32"\n", next_arrival_time, cursor->msg.size);
    while ((cursor != NULL) && (cursor->msg.real_arrival_time < next_arrival_time)) {
        DMSG("Update arrival_time: new: %llut, last: %llut\n", next_arrival_time, cursor->msg.real_arrival_time);
        cursor->msg.real_arrival_time = next_arrival_time;
        next_arrival_time   += cursor->msg.size;
        cursor = cursor->next;
    }

    return;
}

void
xsim_msg_list_add_sorted(xsim_msg_list_t *list, xsim_msg_t *msg, sim_time current_time) {

    xsim_msg_list_elt_t *elt    = (xsim_msg_list_elt_t *)msg;
    xsim_msg_list_elt_t *cursor = &(list->head);

    /* find the right place for the new msg */
    while ((cursor->next != NULL) && ((cursor->next->msg).due_time < msg->due_time)) {
        cursor = cursor->next; 
    }

    /* insert the msg in the list and set its real_arrival_time */
    sim_time next_arrival_time = cursor->msg.real_arrival_time + cursor->msg.size;
    if (current_time > next_arrival_time) {
        next_arrival_time = current_time;
    }

    elt->msg.real_arrival_time = (next_arrival_time < elt->msg.due_time) ?
        elt->msg.due_time : next_arrival_time;
    elt->next    = cursor->next;
    cursor->next = elt;

    if (elt->next == NULL) {
        list->tail = elt;
        return;
    }

    /* update the real_arrival_time of the rest of the list */ 
    next_arrival_time = elt->msg.real_arrival_time + elt->msg.size;
    cursor = cursor->next->next;
    DMSG("Next arrival time: %llut, size message: %"PRIu32"\n", next_arrival_time, elt->msg.size);
    while ((cursor != NULL) && (cursor->msg.real_arrival_time < next_arrival_time)) {
        DMSG("Update arrival_time: new: %llut, last: %llut\n", next_arrival_time, cursor->msg.real_arrival_time);
        cursor->msg.real_arrival_time = next_arrival_time;
        next_arrival_time   += cursor->msg.size;
        cursor = cursor->next;
    }

    return;
}

#if 0
void
xsim_msg_list_push(xsim_msg_list_t *list, xsim_msg_t *msg) {

    xsim_msg_list_elt_t *elt = (xsim_msg_list_elt_t *)msg;

    //xsim_lock_lock(&list->lock);
    LOCK_LISTENER_MSG_EMPTY(list);

#if 0
    DMSG("<msg_list> pushing %p (to %p) \n", elt, list->head);
#endif    

    elt->next       = list->head.next;
    list->head.next = elt;
    if (elt->next == &(list->tail))
        list->tail.next = elt;

    UNLOCK_LISTENER_MSG_EMPTY(list);
    //xsim_lock_unlock(&list->lock);
    return;

}

xsim_msg_t *
xsim_msg_list_pop(xsim_msg_list_t *list) {

    xsim_msg_t *poped = NULL;

    //xsim_lock_lock(&list->lock);
    LOCK_LISTENER_MSG_EMPTY(list);
    poped = xsim_msg_list_del(list);
    UNLOCK_LISTENER_MSG_EMPTY(list);
    //xsim_lock_unlock(&list->lock);

    return poped;
}
#endif

/*
 * Messages ... setup a pool
 */
xsim_msg_t *
xsim_msg_new(void) {

    xsim_msg_list_elt_t *res = malloc(sizeof(xsim_msg_list_elt_t));

    res->next = NULL;
    xsim_targets_clearall(&res->msg.targets);

    return (xsim_msg_t *)res;
}

    void
xsim_msg_free(xsim_msg_t *msg)
{
    free(msg);
    return;
}

void
xsim_msg_dump(xsim_msg_t *msg){

    IMSG("===========================\n");
    IMSG(" x_id: 0x%02x\n", msg->x_id);
    IMSG(" src_id: 0x%08x\n", msg->src_id);
    IMSG(" seq_id: 0x%08x\n", msg->seq_id);

    IMSG("===========================\n");

    return;
}


void xsim_msg_printf_list(xsim_msg_list_t *list)
{
    xsim_msg_list_elt_t *current = list->head.next;
    fprintf(stdout, "@Head: %p ; @next: %p\n", 
            &(list->head), current);
    while(current != NULL) {
        fprintf(stdout, "@msg: %p ; @next: %p\n",
                current, current->next);
        current = current->next;
    }
    fprintf(stdout, "@Tail: %p ; @next: %p\n",
            list->tail, list->tail->next); 
    return;
}
