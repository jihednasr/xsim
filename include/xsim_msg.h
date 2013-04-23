#ifndef _XSIM_MSG_H_
#define _XSIM_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct xsim_msg           xsim_msg_t;
typedef struct xsim_msg_list      xsim_msg_list_t;
typedef struct xsim_msg_list_elt  xsim_msg_list_elt_t;
typedef struct xsim_vci_request  xsim_vci_request_t;
typedef struct xsim_vci_response xsim_vci_response_t;

#include <stdint.h>
#include <xsim_targets.h>
#include <xsim_sync.h>
#include <xsim_time_type.h>

/* in bytes */
//#define XSIM_MAX_PAYLOAD   8
#define XSIM_MAX_PAYLOAD   64

/* possible value of the type field of xsim_msg_t */
enum xsim_msg_type {
  xsim_has_payload,
  xsim_null_msg,
  xsim_end_of_simulation,
  xsim_need_time_info /* must be the last name of the enum */
};
typedef enum   xsim_msg_type      xsim_msg_type_t;

struct xsim_vci_request {
    uint32_t    address;                                                   
    uint8_t     be;                                                        
    uint8_t     cmd;                                                       
    char        contig;                                                    
    //uint8_t     wdata[8];                                                  
    char        eop;                                                       
    char        cons;                                                      
    uint8_t     plen;                                                      
    char        wrap;                                                      
    char        cfixed;                                                    
    char        clen;                                                      
    uint16_t    srcid;                                                     
    uint16_t    srcnodeid;                                                 
    uint8_t     trdid;                                                     
    uint8_t     pktid;                                                     

    uint32_t    initial_address;                                           
    int         slave_id;                                                  
};                           

struct xsim_vci_response {
    //uint8_t   rdata[8];
    char      reop;
    char      rerror;
    uint16_t  rsrcid;
    uint16_t  rsrcnodeid;
    uint8_t   rtrdid;
    uint8_t   rpktid;

    // **************
    // Extra field
    uint8_t   rbe;
};

enum xsim_noc_msg_type {
    xsim_request,
    xsim_response,
    xsim_null
};
typedef enum   xsim_noc_msg_type xsim_noc_msg_type_t;


struct xsim_msg {

  uint8_t               x_id;
  uint32_t              src_id;
  uint32_t              seq_id;
  xsim_targets_t        targets;

  xsim_targets_t        mark;

  sim_time              stamp_time;        /* send time of the first flit */
  sim_time              due_time;          /* arrival time of the first flit if it is alone in the network */
  sim_time              arrival_time;      /* time when received by the listener - useful for debug */
  sim_time              real_arrival_time; /* real arrival time of the first bit */
  sim_time              min_table_recv;    /* min_table_recv when receives - useful for debug */

  xsim_vci_request_t    req;
  xsim_vci_response_t   res;
  uint8_t               payload[XSIM_MAX_PAYLOAD];
  uint32_t              size;

  xsim_msg_type_t       type; 
  xsim_noc_msg_type_t   noc_type;

};

struct xsim_msg_list_elt {
  xsim_msg_t msg;
  xsim_msg_list_elt_t *next;
};

struct xsim_msg_list {

  xsim_lock_t           lock; /* The big lock :-) */
  xsim_msg_list_elt_t   head;
  xsim_msg_list_elt_t*  tail; /* Point to the last element of the list */
  xsim_msg_list_elt_t   dummy; /* Dummy cell for the FIFO */

};

xsim_msg_list_t *xsim_msg_list_new(void);
void             xsim_msg_list_free(xsim_msg_list_t *list);
int              xsim_msg_list_init(xsim_msg_list_t *list);
int              xsim_msg_list_fini(xsim_msg_list_t *list);

int              xsim_msg_list_is_empty(xsim_msg_list_t *list);
xsim_msg_t*      xsim_msg_list_first(xsim_msg_list_t *list);
xsim_msg_t*      xsim_msg_list_next(xsim_msg_list_t *list, xsim_msg_t *msg);


/* add in tail */
void             xsim_msg_list_add(xsim_msg_list_t *list, xsim_msg_t *msg);

/* 
 * The msg is inserted at its right place compare to its due_time.
 * The list must be sorted by the due_time value of its messages. 
 * This function updates also the real_arrival_time field of the message in list
 * in order to avoid overlapping.
 * */
void             xsim_msg_list_add_sorted(xsim_msg_list_t *list, xsim_msg_t *msg, sim_time current_time);

/* del the top - Return NULL if no element */
xsim_msg_t      *xsim_msg_list_del(xsim_msg_list_t *list);

#if 0
void             xsim_msg_list_push(xsim_msg_list_t *list, xsim_msg_t *msg);
/* Return NULL is list is empty */
xsim_msg_t      *xsim_msg_list_pop(xsim_msg_list_t *list);
#endif

xsim_msg_t      *xsim_msg_new(void);
void             xsim_msg_free(xsim_msg_t *msg);

/* Up-date the real_arrival_time of the msg of the list according to current_time */
void
xsim_msg_update_list_time(xsim_msg_list_t *list, sim_time current_time);

static inline void
xsim_msg_list_lock(xsim_msg_list_t *list) {
  xsim_lock_lock(&list->lock);
}

static inline void
xsim_msg_list_unlock(xsim_msg_list_t *list) {
  xsim_lock_unlock(&list->lock);
}

void xsim_msg_printf_list(xsim_msg_list_t *list);

#ifdef __cplusplus
}
#endif
#endif /* _XSIM_MSG_H_ */
