#ifndef XSIM_SC_WRAPPER_RECV_MODEL9_H_
#define XSIM_SC_WRAPPER_RECV_MODEL9_H_

#include <systemc.h>
#include <xsim_sc_FIFO_channel.h>

#include <xsim_iface.h>
#include <xsim_time_model_common.h>
#include <xsim_time.h>
#include <xsim_perf_measure.h>

#include <xsim_sc_debug.h>

#define MIN(a, b)  ((a) < (b)) ? (a) : (b) 
#define MAX(a, b)  ((a) < (b)) ? (b) : (a)

template <int DATA_SIZE>
SC_MODULE(xsim_sc_wrapper_recv) {
    private:
        /* INSTANCE NAME & COORDINATES */
        const	char*	NAME;
        xsim_iface_t *iface;
        int already_ask;

    public:
        sc_port< write_if<sc_uint<DATA_SIZE> > > out;

        SC_HAS_PROCESS(xsim_sc_wrapper_recv);
        xsim_sc_wrapper_recv(sc_module_name insname, 
                xsim_iface_t *ptr_iface) : sc_module(insname) {
            NAME     = (char*) strdup(insname);  
            iface = ptr_iface;

            already_ask = 0;

            SC_THREAD(receive_new_msg_else_wait_it);
        }

        void receive_new_msg_else_wait_it()
        {
            sc_uint<DATA_SIZE> data;
            xsim_msg_t *msg_to_recv = NULL;
            int indice_packet = 0;
            while (1) {
                sc_time t = sc_time_stamp();
                iface->node->current_time = SC2XSIM(t);

                /* take the next message if there is one */
                xsim_time_model_message_to_recv(iface->node, iface->x_id, &msg_to_recv);

                if (msg_to_recv != NULL) {
                    DMSG("<%d/%d> (%lld) Receive a message from %d.\n",
                            iface->node->node_id, iface->x_id, 
                            sc_time_stamp().value(),
                            msg_to_recv->src_id);
                    int real_size = msg_to_recv->size * PACKET_SIZE;

                    /* begin of packet */
                    indice_packet = 0;
                    data = 
                        (((uint64_t) DSPIN_BOP) << 32) |
                        ((uint64_t)(msg_to_recv->payload[indice_packet+0]) << 24) | 
                        ((uint64_t)msg_to_recv->payload[indice_packet+1] << 16) | 
                        ((uint64_t)msg_to_recv->payload[indice_packet+2] << 8) | 
                        ((uint64_t)msg_to_recv->payload[indice_packet+3] << 0); 

                    /* transfert the data */
                    do {
                        DMSG("<%d/%d> Wrapper flit received: %llX ; indice_packet: %d\n", 
                                iface->node->node_id, iface->x_id, data.value(), indice_packet);
                        out->write(data);
                        indice_packet += PACKET_SIZE;

                        data = 
                            ((uint64_t)(msg_to_recv->payload[indice_packet+0]) << 24) | 
                            ((uint64_t)msg_to_recv->payload[indice_packet+1] << 16) | 
                            ((uint64_t)msg_to_recv->payload[indice_packet+2] << 8) | 
                            ((uint64_t)msg_to_recv->payload[indice_packet+3] << 0); 
                    } while (indice_packet+4 != real_size);

                    /* end of packet */
                    data = data | (((uint64_t) DSPIN_EOP) << 32);
                    DMSG("<%d/%d> Wrapper flit received: %llX\n", 
                            iface->node->node_id, iface->x_id, data.value());
                    out->write(data);
                    msg_to_recv = NULL;
                }

                /* 
                 * slip until the next possible message to receive
                 */
                LOCK_NODE_RECV_QUEUE(iface->node->recv_queue[iface->x_id]); 
                sim_time tmp = MAX(iface->next_possible_recv,
                        MIN(xsim_time_iface_next_message_recv(iface), iface->min_table_recv));
                UNLOCK_NODE_RECV_QUEUE(iface->node->recv_queue[iface->x_id]);

                sc_time time_next_msg = XSIM2SC(tmp);
                if (time_next_msg > t) {
                    sc_time wait_time(time_next_msg - t);
                    DMSG("<%d/%d> Wrapper_recv is going to sleep for %lld.\n",
                            iface->node->node_id, iface->x_id, wait_time.value());
                    wait(wait_time);
                    already_ask = 0;
                    info_time_already_send = 0;
                    DMSG("<%d/%d> Wrapper_recv is woken-up.\n",
                            iface->node->node_id, iface->x_id);
                } else {
                    /* Ask the time of the other node if possible */
                    if (!(already_ask)) {
                        xsim_msg_t *msg = xsim_msg_new();
                        msg->size = 0;
                        msg->type = xsim_need_time_info;
                        xsim_targets_clearall(&msg->targets);
                        xsim_iface_send(iface, msg);
                        DMSG("<%d/%d> (%llu t) Send a request for time information\n",
                                iface->node->node_id, iface->x_id, iface->node->current_time);
                        already_ask = 1;
                    }
                    /* Or wait the new next possible message arrives */
                    wait(SC_ZERO_TIME);
                }
            }
        }

};

#endif
