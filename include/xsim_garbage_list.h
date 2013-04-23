#ifndef _XSIM_GARBAGE_LIST_H_
#define _XSIM_GARBAGE_LIST_H_


int xsim_garbage_list_init(xsim_msg_list_t *list, xsim_msg_t *first_msg, xsim_msg_t *last_msg);
int xsim_garbage_list_fini(xsim_msg_list_t *list);


/* Garbage cells already marked by the listeners */
void xsim_garbage_list_collect_cells(xsim_msg_list_t *source, xsim_msg_list_t *garbage_list);

/* add in tail */
void xsim_garbage_list_add(xsim_msg_list_t *list, xsim_msg_t *first_msg, xsim_msg_t *last_msg);

/* 
 * Remove in head 
 * Return NULL if there is no more valid cells.
 */

xsim_msg_t *xsim_garbage_list_del(xsim_msg_list_t *list, xsim_msg_list_t *source);


#endif 
