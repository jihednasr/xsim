#ifndef _XSIM_CENTRAL_LIST_H_
#define _XSIM_CENTRAL_LIST_H_


int  xsim_central_list_init(xsim_msg_list_t *list, xsim_msg_t *first_msg);
int  xsim_central_list_fini(xsim_msg_list_t *list);

int xsim_central_list_all_read(xsim_msg_list_t *list, xsim_msg_t *current_msg);

/* add in tail */
void xsim_central_list_add(xsim_msg_list_t *list, xsim_msg_t *msg);

/* Remove in head */
void xsim_central_list_conditional_del(xsim_msg_list_t *central_list, 
        xsim_msg_t *current,
        int              target,
        xsim_msg_list_t *garbage_list);


#endif 
