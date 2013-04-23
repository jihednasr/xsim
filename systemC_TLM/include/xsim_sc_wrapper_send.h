#ifndef _XSIM_SC_WRAPPER_SEND_H_
#define _XSIM_SC_WRAPPER_SEND_H_

#include <systemc.h>
#include <xsim_sc_interfaces.h>

#include <xsim_iface.h>
#include <xsim_msg.h>
#include <xsim_time_model_common.h>

#include <xsim_sc_debug.h>

template <int DATA_SIZE>
SC_MODULE(xsim_sc_wrapper_send) {
    public:
        sc_port< read_if<sc_uint<DATA_SIZE> > >in;

        SC_HAS_PROCESS(xsim_sc_wrapper_send);
        xsim_sc_wrapper_send(sc_module_name name, 
                xsim_iface_t *ptr_iface) : sc_module(name) {
            NAME = (const char *) name;

            iface = ptr_iface;
            msg_to_send = NULL;
            last_time = 0;

            SC_THREAD(send_in_network_thread);
            SC_THREAD(broadcast_time_information);
        }

        void send_in_network_thread();
        void broadcast_time_information();

    private:
        /* XSIM structures */
        xsim_iface_t   *iface;
        xsim_msg_t     *msg_to_send;
        sim_time        last_time;

        /* INSTANCE NAME & COORDINATES */
        const	char*	NAME;

};

template <int DATA_SIZE>
void 
xsim_sc_wrapper_send<DATA_SIZE>::send_in_network_thread() {
    sc_uint<DATA_SIZE> data;
    int nb_frame = 0;
    while (true) {
        nb_frame = 0;
        DMSG("<%d/%d> Wrapper_send is going to sleep.\n",
                iface->node->node_id, iface->x_id);
        in->read(data);
        DMSG("<%d/%d> Wrapper_send is woken-up.\n",
                iface->node->node_id, iface->x_id);
        nb_frame++;
        
        /* Begin of packet */
        if (((data >> (DATA_SIZE-4)) & DSPIN_BOP) == DSPIN_BOP) {
            DMSG("Receive the BOP.\n");
            msg_to_send = xsim_msg_new(); 
            DMSG("<%d/%d> First data send: %08X\n", 
                    iface->node->node_id,
                    iface->x_id,
                    (unsigned int)(fifo_in_data & 0xffffffff));

            msg_to_send->size = 0;
            msg_to_send->stamp_time = SC2XSIM(sc_time_stamp());
            xsim_targets_set(&msg_to_send->targets, (data & ADDR_MASK));
            msg_to_send->type = xsim_has_payload;
        } else {
            EMSG("<%d/%d> Error: expected BOP, but has not received it.\n",
                    iface->node->node_id,
                    iface->x_id);
            continue;
        }

        while (1) {
            if (msg_to_send->size >= XSIM_MAX_PAYLOAD) {
                EMSG("<%d/%d> Error: receive more packet as expect.\n",
                    iface->node->node_id,
                    iface->x_id);
                break;
            }
            msg_to_send->payload[msg_to_send->size+0] = (data & FIRST_BYTE_MASK) >> 24;
            msg_to_send->payload[msg_to_send->size+1] = (data & SECOND_BYTE_MASK) >> 16;
            msg_to_send->payload[msg_to_send->size+2] = (data & THIRD_BYTE_MASK) >> 8;
            msg_to_send->payload[msg_to_send->size+3] = (data & FOURTH_BYTE_MASK) >> 0;
            DMSG("<%d/%d> Frame %d: %02X%02X%02X%02X\n",
                    iface->node->node_id,
                    iface->x_id,
                    nb_frame,
                    msg_to_send->payload[msg_to_send->size+0],
                    msg_to_send->payload[msg_to_send->size+1],
                    msg_to_send->payload[msg_to_send->size+2],
                    msg_to_send->payload[msg_to_send->size+3]);
            msg_to_send->size += PACKET_SIZE;

            if (((data >> (DATA_SIZE-4)) & DSPIN_EOP) == DSPIN_EOP) {
                break;
            } else {
                in->read(data);
                nb_frame++;
            }
        }
        /* End of packet */
        DMSG("<%d/%d> (%lld t) Send a message to node %05lld with last data: %08X.\n", 
                iface->node->node_id, iface->x_id,
                iface->node->current_time,
                msg_to_send->targets,
                (unsigned int)(data & 0xffffffff));

        msg_to_send->size /= PACKET_SIZE; /* in order the internal control worked correctly */
        xsim_iface_send(iface, msg_to_send);
        msg_to_send = NULL;
    }
}

template <int DATA_SIZE>
void 
xsim_sc_wrapper_send<DATA_SIZE>::broadcast_time_information() 
{
    while (1) {
        iface->node->current_time = SC2XSIM(sc_time_stamp());
        if (last_time != iface->node->current_time) {
            DMSG("<%d/%d> send a null message with current time %lld.\n",
                    iface->node->node_id, iface->x_id, iface->node->current_time);
            /* indicate the new time to the others nodes */
            broadcast(&iface->node->msg_box->mark, iface, xsim_null_msg);
            last_time = SC2XSIM(sc_time_stamp());
        }
        DMSG("<%d/%d> Wrapper_send_broadcast_time is going to sleep.\n",
                iface->node->node_id, iface->x_id);
        wait(BASE_TIME, TIMING);
        DMSG("<%d/%d> Wrapper_send_broadcast_time is woken-up.\n",
                iface->node->node_id, iface->x_id);
    }
}

#endif

