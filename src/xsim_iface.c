/* __HEADER_HERE__ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xsim_node.h>
#include <xsim_iface.h>
#include <xsim_topology.h>

#include <xsim_time_model_common.h>

#define DBG_HDR "xsim:iface"
#ifdef XSIM_IFACE_DEBUG
#define DEBUG
#endif /* XSIM_IFACE_DEBUG */

#ifdef XSIM_IFACE_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_IFACE_HDEBUG */

#include <xsim_debug.h>

xsim_iface_t *xsim_iface_new(xsim_node_t *node, int x_id) 
{
    xsim_iface_t *res = malloc(sizeof(xsim_iface_t));
    res->table        = malloc(sizeof(sim_time) * node->nb_nodes);
    res->state        = malloc(sizeof(xsim_node_state_t) * node->nb_nodes);
    res->x_id         = x_id;
    res->node         = node;

    xsim_iface_clean(res);

    return res;
}

void xsim_iface_free(xsim_iface_t *iface) 
{
    free(iface->table);
    free(iface->state);
    free(iface);

    return;
}

void xsim_iface_clean(xsim_iface_t *iface)
{
    memset(iface->table, 0, sizeof(sim_time) * iface->node->nb_nodes);
    sim_time min = MAX_SIM_TIME;
    for (int i=0 ; i<iface->node->nb_nodes; i++) {
        iface->state[i] = simulating;
        sim_time tmp = xsim_topology_travel_time(i, iface->node->node_id);
        if (tmp < min) {
            min = tmp; 
        }
    }
    iface->next_possible_send = 0;
    iface->next_possible_recv = 0;
    iface->min_table_recv     = min;
    iface->msg_recv           = 0;
    iface->msg_send           = 0;

    return;
}

int xsim_iface_send(xsim_iface_t *iface, xsim_msg_t *msg) 
{
    DMSG("<%d/%d> (%03llu t) Send a message\n",
            iface->node->node_id, iface->x_id, iface->node->current_time);
    if (msg->type == xsim_has_payload) {
        if (iface->next_possible_send > iface->node->current_time) {
            EMSG("<%d-%d> Error: some message sent are overlapping. current_time: %"PRIu64", next possible send: %"PRIu64"\n", 
                    iface->node->node_id, iface->x_id, iface->node->current_time, iface->next_possible_send);
        }
        iface->next_possible_send  = iface->node->current_time + msg->size;
    }
    msg->stamp_time = iface->node->current_time;   
    if ((msg->type != xsim_has_payload) && (msg->size != 0)) 
        EMSG("There is message without payload  but with size > 0: type %d, size: %d.\n", msg->type, msg->size);

    return xsim_node_send(iface->node, iface->x_id, msg);
}

xsim_msg_t *xsim_iface_recv(xsim_iface_t *iface) 
{
    xsim_msg_t *msg = xsim_node_recv(iface->node, iface->x_id); 
    if (msg != NULL) {
        if (msg->real_arrival_time != iface->node->current_time) {
            EMSG("<%d/%d> Error: time error when the message was received by the NIC: msg->real_arrival_time: %"PRIu64" ; current: %"PRIu64"\n",
                    iface->node->node_id, iface->x_id,
                    msg->real_arrival_time, iface->node->current_time);
        }
        if (msg->real_arrival_time < iface->next_possible_recv) {
            EMSG("<%d/%d> Error: error in the management of the arrival_time or next_possible_recv.\n",
                    iface->node->node_id, iface->x_id);
        }

        iface->next_possible_recv = iface->node->current_time + msg->size;
    }

    return msg;
}
