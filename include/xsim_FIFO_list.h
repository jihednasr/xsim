#ifndef _XSIM_FIFO_MSG_H_
#define _XSIM_FIFO_MSG_H_

typedef struct xsim_FIFO_elt  xsim_FIFO_elt_t;
typedef struct xsim_FIFO_list xsim_FIFO_list_t;

#include <inttypes.h>

struct xsim_FIFO_elt {
    xsim_msg_t       *msg;
    int               used;
    
    xsim_FIFO_elt_t  *next;
};

struct xsim_FIFO_list {
    xsim_FIFO_elt_t    *head;
    xsim_FIFO_elt_t    *tail;
    xsim_FIFO_elt_t    *last;
    xsim_FIFO_elt_t    *pending_free;
    uint32_t            seq_msg_FIFO_add;
    uint32_t            seq_msg_FIFO_del;
};


xsim_FIFO_list_t *xsim_FIFO_list_new(void);
void              xsim_FIFO_list_free(xsim_FIFO_list_t *list);
int               xsim_FIFO_list_init(xsim_FIFO_list_t *list);
int               xsim_FIFO_list_fini(xsim_FIFO_list_t *list);

int               xsim_FIFO_list_is_empty(xsim_FIFO_list_t *list);

/* add in tail */
void              xsim_FIFO_list_add(xsim_FIFO_list_t *list, xsim_msg_t *msg);

/* 
 * del the top
 * Return 0 if there is no msg in the FIFO, else return true.
 */
int               xsim_FIFO_list_del(xsim_FIFO_list_t *list, xsim_msg_t **msg);

/* Private */
void              xsim_FIFO_free_what_is_possible(xsim_FIFO_list_t *list);

#endif
