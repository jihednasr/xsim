#ifndef XSIM_SC_RECEIVER_H_
#define XSIM_SC_RECEIVER_H_

#include <systemc.h>
#include <xsim_sc_interfaces.h>

#include <xsim_sc_debug.h>

template <int DATA_SIZE>
SC_MODULE(xsim_sc_receiver) {
    public:
        sc_port<read_if<sc_uint<DATA_SIZE> > >in;

        SC_HAS_PROCESS(xsim_sc_receiver);
        xsim_sc_receiver (sc_module_name insname, int identity) 
            : sc_module(insname) {

            NAME     = (char*) strdup(insname);  
            ID       = identity ;

            latency = 0;
            A_COUNT = 0 ;
            SC_THREAD(send_in_network_thread);
        }

        void send_in_network_thread();

        int  get_latency() {return latency;};
        int  get_a_count() {return A_COUNT;};

    private:
        /* INSTANCE NAME & COORDINATES */
        const	char*	NAME;
        int     ID;

        int     latency, A_COUNT ;

        void timing(sc_uint<DATA_SIZE> first_data);
};

template <int DATA_SIZE>
void 
xsim_sc_receiver<DATA_SIZE>::timing (sc_uint<DATA_SIZE> first_data)
{
    int lat;

    lat = ((0x10000 + sc_time_stamp().value() - ((first_data>>16) & 0xffff)) % 0x10000)-1;	// -1 is the FIFO latency of the destination node and does not belong to the path

    //    if( (FIFO_IN.read()>>16) & 0xffff)
    {
        latency += lat;
        A_COUNT ++;
    }                                
}


template <int DATA_SIZE>
void 
xsim_sc_receiver<DATA_SIZE>::send_in_network_thread()
{
    sc_uint<DATA_SIZE> data;
    int OK = 0;
    while (true) {
        DMSG("[%d] receiver is going to sleep.\n", ID);
        in->read(data);
        DMSG("[%d] receiver is woken-up - receive a message.\n", ID);

        /* begin of packet */
        if (((data >> (DATA_SIZE-4)) & DSPIN_BOP) == DSPIN_BOP) {
            DMSG(stdout, "[%d] (%lld) Receive a message.\n", 
                    ID, sc_time_stamp().value());
            if ((unsigned int) ID != (data & 0x7fff)) { 
                EMSG("[%d] Error: message not received by the right node, must be node %lld.\n", ID, (data & 0x7fff));
            }	
            timing (data);
            OK = data & 0xffffffff;

        } else {
            EMSG("[%d] Error: expected a begin of packet but does not receive one ; received: %llX\n",
                    ID, data.value());
            continue;
        }

        /* data */
        int nb_flit = 1;
        while (1) {
            if (nb_flit > MAX_PACKET_LENGTH) {
                EMSG("[%d] Error: receive more flit as expect. Received: %d, Expected: %d.\n", 
                        ID, nb_flit, MAX_PACKET_LENGTH);
                break;
            }
            DMSG("[%d] flit received: %09llX.\n",
                    ID, data.value());
            if (((data >> (DATA_SIZE-4)) & DSPIN_EOP) == DSPIN_EOP) {
                break;
            } else {
                in->read(data);
                nb_flit++;
                OK += data & 0xffffffff;
            }
        } 

        /* end of packet */
        if (OK) { 
            EMSG("[%d] Error on the checksum.\n", ID);
        } else {
            printf("[%d] (%lld) Node has received the whole message correctly.\n",
                    ID, sc_time_stamp().value());
        }
    }
}

#endif

