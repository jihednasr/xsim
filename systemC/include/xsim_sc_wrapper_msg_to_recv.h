#ifndef SOCLIB_3DSPIN_WRAPPER_MSG_TO_RECV_H
#define SOCLIB_3DSPIN_WRAPPER_MSG_TO_RECV_H


#include <systemc.h>
#include <soclib_dspin_interfaces.h>
#include <soclib_generic_fifo.h>

#include <xsim_iface.h>
#include <xsim_msg.h>
#include <xsim_time.h>
#include <xsim_time_model_common.h>
#include <xsim_perf_measure.h>

#define MIN(a, b)  ((a) < (b)) ? (a) : (b) 
#define MAX(a, b)  ((a) < (b)) ? (b) : (a)

#include <xsim_sc_debug.h>

template<int DATA_SIZE>
struct Xsim_sc_wrapper_msg_to_recv : sc_module {

    enum 
    {
        S_IDLE = 0,
        S_DATA = 1,
        S_EOP  = 2
    };

    // EXTERNAL PORTS
    sc_in<bool>  CLK;
    sc_in<bool>  RESETN;
    DSPIN_OUT<DATA_SIZE>    OUT;

    //// REGISTERS
    sc_signal<bool>	ALLOC_OUT;	// output ports allocation state 
    int  indice_recv;           // Indice of the data in the msg to receive
    int  real_size_msg;         // Store the real size of the msg for xsim 
    sc_signal<int>  STATE;
    sc_uint<DATA_SIZE>  fifo_in_data;

    xsim_msg_t     *msg_to_recv;
    xsim_iface_t   *iface;

    // INSTANCE COORDINATES
    int     LOCAL_ADDR;
//    int		XLOCAL;
//    int		YLOCAL;
//    int		ZLOCAL;


    /////////////////////////////////////////////
    //          SC_Thread
    /////////////////////////////////////////////
    void receive_new_msg_else_wait_it()
    {
        while (1) {
            sc_time t = sc_time_stamp();
            iface->node->current_time = SC2XSIM(t);

            /* Manage the reception of a message and the transfert to the node */
            if (indice_recv >= XSIM_MAX_PAYLOAD-1) {
                msg_to_recv = NULL;
                xsim_time_model_message_to_recv(iface->node, iface->x_id, &msg_to_recv);

                if (msg_to_recv != NULL) {
                    printf("<%d/%d> Receive a message from %d.\n",
                            iface->node->node_id, iface->x_id, 
                            msg_to_recv->src_id);
                    real_size_msg = msg_to_recv->size * PACKET_SIZE;
                }
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
                sc_time wait_time(sc_time(time_next_msg) - t);
                DMSG("<%d/%d> wrapper_msg_to_recv_thread is going to sleep.\n",
                            iface->node->node_id, iface->x_id);
                wait(wait_time);
                DMSG("<%d/%d> wrapper_msg_to_recv_thread is wake-up.\n",
                            iface->node->node_id, iface->x_id);
            } else {
                /* Or wait the new next possible message arrives */
                wait(SC_ZERO_TIME);
            }
        }
    }

    /////////////////////////////////////////////
    //           Transition()
    /////////////////////////////////////////////

    void Transition()
    {
        if(RESETN == false) {
            ALLOC_OUT = false;
            indice_recv = XSIM_MAX_PAYLOAD-1;
            if (msg_to_recv != NULL) {
                xsim_msg_free(msg_to_recv);
                msg_to_recv = NULL;
            }

        } else {
            /* Manage the reception of a message and the transfert to the node */
            switch (STATE) {
            case S_IDLE:
                if (msg_to_recv != NULL) {
                    DMSG("Size message: %d\n", msg_to_recv->size);
                    STATE = S_DATA;
                    fifo_in_data = 0 |
                        ((sc_uint<DSPIN_DATA_SIZE>) DSPIN_BOP << 32) |
                        (msg_to_recv->payload[0] << 24) | 
                        (msg_to_recv->payload[1] << 16) | 
                        (msg_to_recv->payload[2] << 8) | 
                        (msg_to_recv->payload[3] << 0); 
                    DMSG("<%d/%d> wrapper_recv: data transmitted: %X\n", 
                            iface->node->node_id, iface->x_id,
                            (unsigned int)(fifo_in_data & 0xffffffff));
                    indice_recv = PACKET_SIZE;
                    ALLOC_OUT = true;
                } else {
                    ALLOC_OUT = false;
                }
                break;
            case S_DATA:
                if (OUT.READ.read() == true) {
                    if (msg_to_recv == NULL) {
                        EMSG("An internal error is arrived - msg lost.\n");
                        STATE = S_IDLE;
                        break;
                    }
                    fifo_in_data = 0 |
                        (msg_to_recv->payload[indice_recv+0] << 24) | 
                        (msg_to_recv->payload[indice_recv+1] << 16) | 
                        (msg_to_recv->payload[indice_recv+2] << 8)  | 
                        (msg_to_recv->payload[indice_recv+3] << 0); 
                    ALLOC_OUT = true;
                    DMSG("<%d/%d> data received: %X\n", 
                            iface->node->node_id, iface->x_id,
                            (unsigned int)(fifo_in_data & 0xffffffff));

                    if (indice_recv+8 == real_size_msg) {
                        STATE = S_EOP;
                    }

                    indice_recv += PACKET_SIZE;
                } else {
                    DMSG("<%d/%d> TG can not immedialty read the data, so we resend it. But it should normaly not happend in this example.\n",
                            iface->node->node_id, iface->x_id);

                    // not read so resend the same data
                }

                break;
            case S_EOP:
                if (OUT.READ.read() == true) {
                    DMSG("<%d> EOP\n", iface->node->node_id);

                    fifo_in_data = 0 |
                        ((sc_uint<DATA_SIZE>) DSPIN_EOP << 32) |
                        (((uint64_t) 
                          msg_to_recv->payload[indice_recv+0]) << 24)  | 
                        (msg_to_recv->payload[indice_recv+1] << 16)  | 
                        (msg_to_recv->payload[indice_recv+2] << 8)   | 
                        (msg_to_recv->payload[indice_recv+3] << 0); 
 
                    DMSG("<%d/%d> wrapper_recv eop: %llx\n",
                            iface->node->node_id, iface->x_id,
                            (unsigned long long)fifo_in_data.value());

                    indice_recv += PACKET_SIZE;
                    STATE = S_IDLE;
                    msg_to_recv = NULL;
                    ALLOC_OUT = true;
                }

                break;
            }


        } 
    };  // end Transition()

    /////////////////////////////////////////////
    //           GenMoore()
    /////////////////////////////////////////////

    void GenMoore()
    {
        if (ALLOC_OUT == true) {
            OUT.DATA  = fifo_in_data;
            OUT.WRITE = true;
        } else {
            OUT.WRITE = false;
        }

    }; // end GenMoore()

    ////////////////////////////////////////
    //           Constructor   
    ////////////////////////////////////////

    SC_HAS_PROCESS(Xsim_sc_wrapper_msg_to_recv);

    Xsim_sc_wrapper_msg_to_recv(sc_module_name name, int ident, xsim_iface_t *ptr_iface) : sc_module(name)
    {
        SC_THREAD(receive_new_msg_else_wait_it);

        SC_METHOD(Transition);
        sensitive_pos << CLK;

        SC_METHOD(GenMoore);
        sensitive_neg << CLK;

        LOCAL_ADDR = ident;
       // XLOCAL = ident & 0x0000001F;
       // YLOCAL = (ident & 0x000003e0) >> 5;
       // ZLOCAL = (ident & 0x00007c00) >> 10;

        iface = ptr_iface;
        msg_to_recv = NULL;
        ALLOC_OUT = false;
        STATE  = S_IDLE;

    }; // end constructor

    ~Xsim_sc_wrapper_msg_to_recv() {
    }

}; // end struct 3DSPIN_ROUTER

#endif 
