#ifndef XSIM_SC_WRAPPER_MSG_TO_SEND_H_ 
#define XSIM_SC_WRAPPER_MSG_TO_SEND_H_ 

#include <systemc.h>
#include <soclib_dspin_interfaces.h>
#include <soclib_generic_fifo.h>

#include <xsim_msg.h>
#include <xsim_iface.h>
#include <xsim_time_model_common.h>

#include <xsim_sc_debug.h>

template<int DATA_SIZE>
struct Xsim_sc_wrapper_msg_to_send: sc_module {

    enum {
        S_IDLE  = 0,
        S_DATA  = 2,
    };

    // EXTERNAL PORTS
    sc_in<bool>  CLK;
    sc_in<bool>  RESETN;
    DSPIN_IN<DATA_SIZE>     IN ;

    sc_in<bool>  SENDER_CLK;
    sc_out<bool> WRAPPER_CLK;
    sc_signal<bool> internal_signal;

    //// REGISTERS
    sc_signal<bool>	ALLOC_IN ;	// input ports allocation state 
    int  STATE;      // state of the automate

    /* XSIM structures */
    xsim_iface_t   *iface;
    xsim_msg_t     *msg_to_send;

    // INSTANCE COORDINATES
    //const	char*	NAME;
    int     ID;
    int    *ADDR;

    int convert_to_targets(int addr);
    void time_management();
    void GenMoore();
    void Transition();

    SC_HAS_PROCESS(Xsim_sc_wrapper_msg_to_send);
    Xsim_sc_wrapper_msg_to_send
        (sc_module_name name, int ident, 
         xsim_iface_t *ptr_iface, int *address_table);

    ~Xsim_sc_wrapper_msg_to_send() {
        if (msg_to_send != NULL) {
            xsim_msg_free(msg_to_send);
            msg_to_send = NULL;
        }
    }

}; 


/* Convertion function between DSPIN et xsim adressing */
template<int DATA_SIZE>
int 
Xsim_sc_wrapper_msg_to_send<DATA_SIZE>::convert_to_targets(int addr) {
    for (int i=0 ; i<iface->node->nb_nodes ; i++) {
        if (ADDR[i] == addr) {
            return i;
        }
    }
    EMSG("Error: Identifier corresponding to the addr %x are not found.\n", addr);
    return 0;
}


/////////////////////////////////////////////
//      SC_THREAD
/////////////////////////////////////////////
template<int DATA_SIZE>
void
Xsim_sc_wrapper_msg_to_send<DATA_SIZE>::time_management ()
{
    wait(2*BASE_TIME, TIMING);

    while (1) {
        iface->node->current_time = SC2XSIM(sc_time_stamp());
        DMSG("<%d/%d> send a null message with current time %lld.\n",
                iface->node->node_id, iface->x_id, iface->node->current_time);
        /* indicate the new time to the others nodes */
        broadcast(&iface->node->msg_box->mark, iface, xsim_null_msg);
        wait(2*TIME_INFO_PERIOD*BASE_TIME, TIMING); 
    }
}


template<int DATA_SIZE>
void 
Xsim_sc_wrapper_msg_to_send<DATA_SIZE>::GenMoore()
{
//    printf("<%d/%d> wrapper_send::GenMoore\n",
//            iface->node->node_id, iface->x_id);
//    if (internal_signal == true) {
//        printf("<%d/%d> internal signal true.\n",
//                iface->node->node_id, iface->x_id);
//    } else {
//        printf("<%d/%d> internal signal false.\n",
//                iface->node->node_id, iface->x_id);
//    }
    if(ALLOC_IN == true) {
        IN.READ  = true;
        WRAPPER_CLK = !WRAPPER_CLK;
    } else {
        IN.READ  = false;
    }
};


/////////////////////////////////////////////
//           Transition()
/////////////////////////////////////////////

template<int DATA_SIZE>
void 
Xsim_sc_wrapper_msg_to_send<DATA_SIZE>::Transition()
{
    bool                fifo_in_write;	// control signals
    sc_uint<DATA_SIZE>  fifo_in_data;

//    if (SENDER_CLK == true) {
//        printf("<%d/%d> sender_clk true.\n",
//                iface->node->node_id, iface->x_id);
//    } else {
//        printf("<%d/%d> sender_clk false.\n",
//                iface->node->node_id, iface->x_id);
//    }

    if(RESETN == false) {
        ALLOC_IN    = false;
        if (msg_to_send != NULL) {
            xsim_msg_free(msg_to_send);
            msg_to_send = NULL;
        }

    } else {
        fifo_in_write = IN.WRITE.read();
        fifo_in_data  = IN.DATA.read();

        DMSG("<%d/%d> wrapper_send::transition\n",
                iface->node->node_id, iface->x_id);

        /* Manage the send of data from the node */
        switch (STATE) {
            case S_IDLE:
                if ((fifo_in_write == true) &&
                        ((fifo_in_data >> (DATA_SIZE-4)) & DSPIN_BOP) == DSPIN_BOP) {
                    DMSG("<%d/%d> Receive a new message to send.\n",
                            iface->node->node_id, iface->x_id);
                    STATE = S_DATA;

                    msg_to_send = xsim_msg_new(); 

                    DMSG("<%d/%d> First data send: %08X\n", 
                            iface->node->node_id,
                            iface->x_id,
                            (unsigned int)(fifo_in_data & 0xffffffff));

                    msg_to_send->payload[0] = (fifo_in_data & FIRST_BYTE_MASK) >> 24;
                    msg_to_send->payload[1] = (fifo_in_data & SECOND_BYTE_MASK) >> 16;
                    msg_to_send->payload[2] = (fifo_in_data & THIRD_BYTE_MASK) >> 8;
                    msg_to_send->payload[3] = (fifo_in_data & FOURTH_BYTE_MASK) >> 0;
                    DMSG("<%d/%d> Message to send: %02X%02X%02X%02X\n",
                            iface->node->node_id,
                            iface->x_id,
                            msg_to_send->payload[0],
                            msg_to_send->payload[1],
                            msg_to_send->payload[2],
                            msg_to_send->payload[3]);

                    msg_to_send->size = PACKET_SIZE;
                    msg_to_send->stamp_time = SC2XSIM(sc_time_stamp());

                    xsim_targets_set(&msg_to_send->targets, convert_to_targets(fifo_in_data & ADDR_MASK));
                    msg_to_send->type = xsim_has_payload;
                    ALLOC_IN = true;
                    internal_signal = !internal_signal;
                } else {
                    ALLOC_IN = false;
                }
                break;
            case S_DATA:
                //DMSG("[%d] S_DATA\n", ID);
                if (fifo_in_write == true) { 
                    if (msg_to_send->size > XSIM_MAX_PAYLOAD-4) {
                            EMSG("<%d/%d> (%lld t) The payload size of the message will be exceeded, futur size: %d/%d.\n", 
                                iface->node->node_id, iface->x_id,
                                (long long int)(iface->node->current_time),
                                msg_to_send->size + PACKET_SIZE, XSIM_MAX_PAYLOAD);
                    }

                    msg_to_send->payload[msg_to_send->size+0] = (fifo_in_data & FIRST_BYTE_MASK) >> 24;
                    msg_to_send->payload[msg_to_send->size+1] = (fifo_in_data & SECOND_BYTE_MASK) >> 16;
                    msg_to_send->payload[msg_to_send->size+2] = (fifo_in_data & THIRD_BYTE_MASK) >> 8;
                    msg_to_send->payload[msg_to_send->size+3] = (fifo_in_data & FOURTH_BYTE_MASK) >> 0;
                    DMSG("<%d/%d> Message to send: %02X%02X%02X%02X\n",
                            iface->node->node_id,
                            iface->x_id,
                            msg_to_send->payload[msg_to_send->size+0],
                            msg_to_send->payload[msg_to_send->size+1],
                            msg_to_send->payload[msg_to_send->size+2],
                            msg_to_send->payload[msg_to_send->size+3]);
                    msg_to_send->size += PACKET_SIZE;
                    ALLOC_IN = true;

                    if (((fifo_in_data >> (DATA_SIZE-4)) & DSPIN_EOP) 
                            == DSPIN_EOP) {
                        DMSG("<%d/%d> (%lld t) Send a message to node %lld with last data: %08X.\n", 
                                iface->node->node_id, iface->x_id,
                                iface->node->current_time,
                                msg_to_send->targets,
                                (unsigned int)(fifo_in_data & 0xffffffff));

                        msg_to_send->size /= PACKET_SIZE; /* in order the internal control worked correctly */
                        if (msg_to_send->size != MAX_PACKET_LENGTH) {
                            EMSG("<%d/%d> (%lld t) Send a message to node %lld with only %d packets.\n", 
                                iface->node->node_id, iface->x_id,
                                (long long int)(iface->node->current_time),
                                (long long int)(msg_to_send->targets),
                                msg_to_send->size);
                        }

                        iface->node->current_time = SC2XSIM(sc_time_stamp());
                        xsim_iface_send(iface, msg_to_send);
                        msg_to_send = NULL;
                        STATE = S_IDLE;
                    } else {
                        DMSG("Send data: %08X\n", (unsigned int)(fifo_in_data & 0xffffffff));
                    }
                    internal_signal = !internal_signal;
                }
                break;
        }
    } 
};  // end Transition()


////////////////////////////////////////
//           Constructor   
////////////////////////////////////////

template<int DATA_SIZE>
Xsim_sc_wrapper_msg_to_send<DATA_SIZE>::Xsim_sc_wrapper_msg_to_send(sc_module_name name, int ident, xsim_iface_t *ptr_iface, int *address_table) : sc_module(name)
{
    SC_THREAD(time_management); 

    SC_METHOD(Transition);
    sensitive_pos << CLK;
    sensitive << SENDER_CLK;

    SC_METHOD(GenMoore);
    sensitive_neg << CLK;
    sensitive << internal_signal;

    //NAME = (const char *) name;
    ID   = ident;

    ADDR = address_table;

    STATE = S_IDLE;

    iface = ptr_iface;
    msg_to_send = NULL;

    iface->node->current_time = 0;

}; // end constructor


#endif 
