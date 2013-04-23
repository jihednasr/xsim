#ifndef _XSIM_SC_WRAPPER_SEND_H_
#define _XSIM_SC_WRAPPER_SEND_H_

#include <systemc.h>
#include <xsim_sc_interfaces.h>

#include <xsim_sc_debug.h>

template <int DATA_SIZE>
SC_MODULE(xsim_sc_wrapper_send) {
    public:
        sc_port< read_if<sc_uint<DATA_SIZE> > >in;
        sc_port< write_if<sc_uint<DATA_SIZE> > >*out;

        SC_HAS_PROCESS(xsim_sc_wrapper_send);
        xsim_sc_wrapper_send(sc_module_name name, 
                int number_nodes, int iface_ident, int node_ident) : sc_module(name) {
            NAME = (const char *) name;
            nb_nodes = number_nodes;
            iface_id = iface_ident;
            node_id  = node_ident;

            out = new sc_port< write_if<sc_uint<DATA_SIZE> > >[nb_nodes];

            SC_THREAD(send_in_network_thread);
        }

        void send_in_network_thread();

    private:
        /* INSTANCE NAME & COORDINATES */
        const	char*	NAME;
        int nb_nodes;
        int iface_id;
        int node_id;

};

template <int DATA_SIZE>
void 
xsim_sc_wrapper_send<DATA_SIZE>::send_in_network_thread() {
    sc_uint<DATA_SIZE> data;
    int dest = 0;
    while (true) {
        DMSG("Wrapper_send is going to sleep.\n");
        in->read(data);
        DMSG("Wrapper_send is woken-up.\n");
        
        /* Begin of packet */
        if (((data >> (DATA_SIZE-4)) & DSPIN_BOP) == DSPIN_BOP) {
            DMSG("Receive the BOP.\n");
            DMSG("First data send: %09llX\n", data.value());

            dest = data & ADDR_MASK;
        } else {
            EMSG("Error: expected BOP, but has not received it.\n");
            continue;
        }

        while (1) {
            DMSG("Flit send to node %d: %09llX\n", dest, data.value());

            if (((data >> (DATA_SIZE-4)) & DSPIN_EOP) == DSPIN_EOP) {
                out[dest]->write(data);
                break;
            } else {
                out[dest]->write(data);
                in->read(data);
            }
        }
    }
}

#endif

