/* __HEADER_HERE__ */

#include <stdlib.h>
#include <xsim_msg.h>
#include <xsim_sync.h>
#include <xsim_error.h>
#include <xsim_garbage_list.h>

#define DBG_HDR "xsim:garbage_list"
#ifdef XSIM_GARBAGE_LIST_DEBUG
#define DEBUG
#endif 

#ifdef XSIM_GARBAGE_LIST_HDEBUG
#define HUGE_DEBUG
#endif

#include <xsim_debug.h>
#include <xsim_perf_measure.h>




int xsim_garbage_list_init(xsim_msg_list_t *list, xsim_msg_t *first_msg, xsim_msg_t *last_msg)
{
    (&list->head)->next = NULL;
    list->tail          = &(list->head);
    list->head.msg.size = 0;
    list->head.msg.real_arrival_time = 0;

    xsim_garbage_list_add(list, first_msg, last_msg);
    return XSIM_SUCCESS;
}

int xsim_garbage_list_fini(xsim_msg_list_t *list __attribute__((__unused__)))
{
    /* Nothing to do */
    return XSIM_SUCCESS;
}



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/* add in tail */
void 
xsim_garbage_list_add(xsim_msg_list_t *list, xsim_msg_t *first_msg, xsim_msg_t *last_msg) 
{
    xsim_msg_list_elt_t *begin = (xsim_msg_list_elt_t *)first_msg;
    xsim_msg_list_elt_t *end   = (xsim_msg_list_elt_t *)last_msg;
    xsim_msg_list_elt_t *last  = NULL;

    end->next = NULL;
    while (1) {
        last = list->tail;
        DMSG("last->next: %X ; list->tail: %X ; begin: %X\n", 
                (unsigned int)(last->next),
                (unsigned int)list->tail, (unsigned int)begin);
        if (__sync_bool_compare_and_swap(&(last->next), NULL, begin)) {
            break;
        } else {
            __sync_bool_compare_and_swap(&(list->tail), last, last->next);
        }
    }
    __sync_bool_compare_and_swap(&(list->tail), last, end);

    return;
}

/* Remove in head */
xsim_msg_t *
xsim_garbage_list_del(xsim_msg_list_t *garbage_list, xsim_msg_list_t *source) 
{
    xsim_msg_list_elt_t *remove_cell = NULL;
    do {
        remove_cell = garbage_list->head.next;
        if (garbage_list->head.next == garbage_list->tail) {
            /*
             * Does not use remove_cell->next for the comparison
             * because if remove_cell can be removed by an other listener
             */
            DMSG("Not enought msg!\n");
            xsim_garbage_list_collect_cells(source, garbage_list);
            if (garbage_list->head.next == garbage_list->tail) {
                /* 
                 * There is nowhere a free cell, so return the hand to the listener
                 * in order it reads other msg.
                 */
                return NULL;
            }
        }
    } while (!__sync_bool_compare_and_swap(&(garbage_list->head.next), remove_cell, remove_cell->next));

    remove_cell->next = NULL;
    xsim_targets_clearall(&(remove_cell->msg.mark));
    return (xsim_msg_t*) remove_cell;
}

/* Garbage cells already marked by the listeners */
void
xsim_garbage_list_collect_cells(xsim_msg_list_t *source, xsim_msg_list_t *garbage_list)
{
    xsim_msg_list_elt_t *first_msg = source->head.next;
    xsim_msg_list_elt_t *last_msg  = &(source->head);

    while (last_msg->next->msg.mark == 0) {
        /*
         * should never go further than the end of the list
         * because the last cell can not be marked by the listener
         */
        last_msg = last_msg->next;
    }
    if (last_msg == &(source->head)) {
        /* no cell are free to be garbaged */
        return;
    }
    if (__sync_bool_compare_and_swap(&(source->head.next), first_msg, last_msg->next)) {
        xsim_garbage_list_add(garbage_list, (xsim_msg_t*)first_msg, (xsim_msg_t*)last_msg);
        return;
    }
    /* an other listener has already collected some cells */
    return;
}

