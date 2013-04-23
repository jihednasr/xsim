/* __HEADER_HERE__ */

#include <stdlib.h>
#include <xsim_msg.h>
#include <xsim_sync.h>
#include <xsim_error.h>
#include <xsim_central_list.h>
#include <xsim_garbage_list.h>

#define DBG_HDR "xsim:central_list"
#ifdef XSIM_CENTRAL_LIST_DEBUG
#define DEBUG
#endif 

#ifdef XSIM_CENTRAL_LIST_HDEBUG
#define HUGE_DEBUG
#endif

#include <xsim_debug.h>


int xsim_central_list_init(xsim_msg_list_t *list, xsim_msg_t *first_msg)
{
    (&list->head)->next = (xsim_msg_list_elt_t*) first_msg;
    list->tail          = (xsim_msg_list_elt_t*) first_msg;
    ((xsim_msg_list_elt_t*)first_msg)->next = NULL;
    list->head.msg.size = 0;
    list->head.msg.real_arrival_time = 0;

    return XSIM_SUCCESS;
}

int xsim_central_list_fini(xsim_msg_list_t *list __attribute__((__unused__)))
{
    /* Nothing to do */
    return XSIM_SUCCESS;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int xsim_central_list_all_read(xsim_msg_list_t *list, xsim_msg_t *current_msg) 
{
    return (xsim_msg_list_next(list, current_msg) == NULL);
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* add in tail */
void 
xsim_central_list_add(xsim_msg_list_t *list, xsim_msg_t *msg) 
{
    xsim_msg_list_elt_t *elt = (xsim_msg_list_elt_t *)msg;
    xsim_msg_list_elt_t *last = NULL;

    elt->next = NULL;
    while (1) {
        last = list->tail;
        DMSG("last->next: %X ; &(list->tail): %X ; elt: %X\n", 
                (unsigned int)(last->next), 
                (unsigned int)&(list->tail), (unsigned int)elt);
        if (__sync_bool_compare_and_swap(&(last->next), NULL, elt)) {
            break;
        } else {
            __sync_bool_compare_and_swap(&(list->tail), last, last->next);
        }
    }
    DMSG("&(list->tail.next): %X ; last: %X ; last->next: %X \n",
            (unsigned int)(list->tail.next), 
            (unsigned int)last, (unsigned int)(last->next));
    __sync_bool_compare_and_swap(&(list->tail), last, last->next);

    //xsim_msg_printf_list(list);
    return;
}


/* Remove in head */
void xsim_central_list_conditional_del(xsim_msg_list_t *central_list, 
        xsim_msg_t       *current,
        int              target,
        xsim_msg_list_t *garbage_list) 
{
    int            try_again = 1;
    xsim_targets_t its_mark  = 0;
    xsim_msg_t    *last_msg  = current;
    xsim_msg_t    *next      = xsim_msg_list_next(central_list, current);
    xsim_msg_list_elt_t *first_msg = NULL;

    xsim_targets_set(&its_mark, target); 
    do {
        xsim_targets_t old_mark = last_msg->mark;
        if (old_mark == its_mark) {
            /* we can delete the current cell and the previous */
            do {
                first_msg = central_list->head.next;
            } while (!__sync_bool_compare_and_swap(&(central_list->head.next), first_msg, next));
            xsim_garbage_list_add(garbage_list, (xsim_msg_t*)first_msg, last_msg);
            try_again = 0;
        } else {
            xsim_targets_t new_mark = old_mark;
            xsim_targets_clear(&new_mark, target);
            try_again = !__sync_bool_compare_and_swap(&(last_msg->mark), old_mark, new_mark);
        }
    } while (try_again); 

    return;
}




