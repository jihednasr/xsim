/* __HEADER_HERE__ */

#include <stdlib.h>
#include <xsim_msg.h>
#include <xsim_sync.h>
#include <xsim_error.h>
#include <xsim_FIFO_list.h>
#include <xsim_garbage_list.h>

#define DBG_HDR "xsim:FIFO_list"
#ifdef XSIM_FIFO_LIST_DEBUG
#define DEBUG
#endif 

#ifdef XSIM_FIFO_LIST_HDEBUG
#define HUGE_DEBUG
#endif

#include <xsim_debug.h>
#include <xsim_perf_measure.h>



xsim_FIFO_list_t *xsim_FIFO_list_new() 
{
    xsim_FIFO_list_t *res = malloc(sizeof(xsim_FIFO_list_t));  

    xsim_FIFO_list_init(res);
    return res;
}

void xsim_FIFO_list_free(xsim_FIFO_list_t *list) 
{
    xsim_FIFO_list_fini(list);
    free(list);
    return;
}

int xsim_FIFO_list_init(xsim_FIFO_list_t *list)
{
    list->head           = malloc(sizeof(xsim_FIFO_elt_t));
    list->head->next     = NULL;
    list->head->used     = 1;

    list->tail           = list->head;
    list->pending_free   = list->head;
    list->last           = NULL;

    list->seq_msg_FIFO_add = 0;
    list->seq_msg_FIFO_del = 0;

    return XSIM_SUCCESS;
}

int xsim_FIFO_list_fini(xsim_FIFO_list_t *list) 
{
    xsim_FIFO_elt_t *elt = NULL;
    while(list->pending_free != list->head) {
        elt                = list->pending_free;
        list->pending_free = list->pending_free->next;
        free(elt);
    }
    elt                = list->pending_free;
    list->pending_free = list->pending_free->next;
    free(elt);
    while(list->pending_free != NULL) {
        elt                = list->pending_free;
        list->pending_free = list->pending_free->next;
        xsim_msg_free(elt->msg);
        free(elt);
    }

    list->head  = NULL;
    list->tail  = NULL;
    list->last  = NULL;
    list->pending_free = NULL;
    return XSIM_SUCCESS;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int xsim_FIFO_list_is_empty(xsim_FIFO_list_t *list)
{
  return (list->head == list->tail);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/* 
 * Only one thread at a time can add a message. 
 * In our case, it is the node thread.
 */
/* add in tail */
void xsim_FIFO_list_add(xsim_FIFO_list_t *list, xsim_msg_t *msg) 
{
    if (list->seq_msg_FIFO_add != msg->seq_id) {
        EMSG("<%d/%d> (%"PRIu64") Error: a message was lost before to arrive in the FIFO list: %d expected, get %d as seq_id.\n",
                msg->src_id, msg->x_id, msg->stamp_time, list->seq_msg_FIFO_add, msg->seq_id);
    }
    list->seq_msg_FIFO_add = msg->seq_id+1;

    xsim_FIFO_elt_t *new_elt = malloc(sizeof(xsim_FIFO_elt_t));
    new_elt->msg  = msg;
    new_elt->next = NULL;
    new_elt->used = 1;

    if (list->last != NULL) 
        EMSG("Error: an other thread is currently adding an element in the list!\n");
    while (1) {
        list->last = list->tail;

        if (__sync_bool_compare_and_swap(&(list->last->next), NULL, new_elt)) {
            break;
        } else {
            __sync_bool_compare_and_swap(&(list->tail), list->last, list->last->next);
        }
    }
    __sync_bool_compare_and_swap(&(list->tail), list->last, list->last->next);
   
    list->last = NULL;

    return;
}


/* 
 * There is only one processus which tries to remove a cell at a time.
 * In our case, only the listener of the node try to remove a cell of the send list.
 * Return true only if the remove has succedded, and 0 in the other case.
 * msg will contains the pointer to the msg removed.
 */
/* Remove in head */
int xsim_FIFO_list_del(xsim_FIFO_list_t *list, xsim_msg_t **msg) 
{
    xsim_FIFO_elt_t *removed_elt = NULL;

    do {
        removed_elt = list->head;
        
        if (removed_elt == list->tail) {
            return 0;
        }

        /* 
         * There is at least one cell in the list after the dummy head cell
         * => we can remove it without danger 
         */
        *msg = removed_elt->next->msg;
    } while (!__sync_bool_compare_and_swap(&list->head, removed_elt, removed_elt->next));
    xsim_FIFO_free_what_is_possible(list);

    if (list->seq_msg_FIFO_del != (*msg)->seq_id) {
        EMSG("<%d/%d> (%"PRIu64") Error: a message was lost before to leave the FIFO list: %d expected, get %d as seq_id.\n",
                (*msg)->src_id, (*msg)->x_id, (*msg)->stamp_time, list->seq_msg_FIFO_del, (*msg)->seq_id);
    }
    list->seq_msg_FIFO_del = (*msg)->seq_id+1;
    return 1;
}

void xsim_FIFO_free_what_is_possible(xsim_FIFO_list_t *list)
{
    while(list->pending_free != list->head) {
        if (list->pending_free == list->last)
            return;
        xsim_FIFO_elt_t *tmp = list->pending_free;
        list->pending_free   = list->pending_free->next;
        free(tmp);
        if (list->pending_free == NULL)
            EMSG("Error: Pending is NULL after free one element.\n");
    }

    return;
}


