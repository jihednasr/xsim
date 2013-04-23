#ifndef XSIM_SC_WRAPPER_RECV_H_
#define XSIM_SC_WRAPPER_RECV_H_

#include <systemc.h>
#include <xsim_sc_FIFO_channel.h>

#include <xsim_sc_debug.h>

#define MIN(a, b)  ((a) < (b)) ? (a) : (b) 
#define MAX(a, b)  ((a) < (b)) ? (b) : (a)

template <int DATA_SIZE>
SC_MODULE(xsim_sc_wrapper_recv) {
    private:
        /* INSTANCE NAME & COORDINATES */
        const	char*	NAME;
        int iface_id;
        int node_id;
        int nb_nodes;
        int message_to_read;
        sc_uint<DATA_SIZE> msg_buffer[MAX_PACKET_LENGTH];
        int indice;

    public:
        sc_event event_receive_a_msg;
        sc_port< read_if<sc_uint<DATA_SIZE> > >*in;
        sc_port< write_if<sc_uint<DATA_SIZE> > >out;

        SC_HAS_PROCESS(xsim_sc_wrapper_recv);
        xsim_sc_wrapper_recv(sc_module_name insname, 
                int number_nodes, int iface_ident, int node_ident) : sc_module(insname) {
            NAME     = (char*) strdup(insname);  
            nb_nodes = number_nodes;
            iface_id = iface_ident;
            node_id  = node_ident;
            message_to_read = 0;

            in = new sc_port< read_if<sc_uint<DATA_SIZE> > >[nb_nodes];

            SC_THREAD(receive_new_msg_else_wait_it);
            SC_THREAD(receive_a_msg);
        }

        void receive_new_msg_else_wait_it()
        {
            sc_uint<DATA_SIZE> data;
            sc_time time_receive_a_message(MIN_PACKET_LENGTH, TIMING);
            int n = 0;
            int last = 0;
            int i = 0;
            indice = 0;
            while (1) {
                last = n;                
                do {
                    if (in[n]->num_available() != 0) {
                        in[n]->read(data);
                        if (((data >> (DATA_SIZE-4)) & DSPIN_BOP) != DSPIN_BOP) {
                            EMSG("A router does not receive a BOP, received: %09llX\n",
                                    data.value());
                        } else {
                            DMSG("Router receives a message from node %d.\n", n);
                        }
                        while (((data >> (DATA_SIZE-4)) & DSPIN_EOP) != DSPIN_EOP) {
                            msg_buffer[indice] = data;                            
                            indice++;
                            in[n]->read(data);
                        }
                        msg_buffer[indice] = data;                            
                        indice++;
                        wait(time_receive_a_message);
                        for (i=0 ; i<indice ; i++) {
                            DMSG("<%d/%d> Transmit to node data: %llX.\n",
                                    node_id, iface_id, msg_buffer[i].value());
                            out->write(msg_buffer[i]);
                        }
                        last = n;
                        indice = 0;
                    }
                    n = (n+1)%nb_nodes;
                } while (last != n);
                if (!message_to_read) {
                    wait(event_receive_a_msg);
                } else {
                    message_to_read = 0;
                }
            }
        }

        void receive_a_msg()
        {
            while (1) {
                wait(event_receive_a_msg);
                message_to_read = 1;
            }
        }

};

#endif
