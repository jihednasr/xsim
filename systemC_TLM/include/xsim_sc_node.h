#ifndef XSIM_SC_NODE_H_
#define XSIM_SC_NODE_H_

#include <xsim_node.h>
#include <systemc.h>

#include <xsim_sc_3dspin_config.h>

#include <xsim_sc_FIFO_channel.h>
#include <xsim_sc_producer.h>
#include <xsim_sc_receiver.h>
#include <xsim_sc_wrapper_send.h>
#include <xsim_sc_wrapper_recv.h>


template <int DATA_SIZE>
SC_MODULE(xsim_sc_node) {
    private:
        int NB_IFACE;

        /* carry the message from the network to the node */
        xsim_sc_fifo<DATA_SIZE> **fifo_in;
        /* carry the message from the node to the network */
        xsim_sc_fifo<DATA_SIZE> **fifo_out; 

        xsim_sc_producer<DATA_SIZE, MAX_PACKET_LENGTH, MIN_PACKET_LENGTH> **producer;
        xsim_sc_receiver<DATA_SIZE> **receiver;
        xsim_sc_wrapper_send<DATA_SIZE> **wrapper_send;
        xsim_sc_wrapper_recv<DATA_SIZE> **wrapper_recv;

    public:
        SC_HAS_PROCESS(xsim_sc_node);
        xsim_sc_node(sc_module_name name, xsim_node_t *ptr_node);

        ~xsim_sc_node() {
            for (int i=0 ; i<NB_IFACE ; i++) {
                delete fifo_in[i]; 
                delete fifo_out[i];
                delete producer[i];
                delete receiver[i];
                delete wrapper_send[i];
                delete wrapper_recv[i];
            }
            delete fifo_in; 
            delete fifo_out;
            delete producer;
            delete receiver;
            delete wrapper_send;
            delete wrapper_recv;
        }

        void set_percent(float per) {
            for (int i=0 ; i<NB_IFACE ; i++) {
                producer[i]->set_percent(per);
            }
        }

        float get_average() {
            float aver = 0;
            int count = 0;
            for (int i=0 ; i<NB_IFACE ; i++) {
                if (receiver[i]->get_a_count()) {
                    aver += receiver[i]->get_latency();
                    count += receiver[i]->get_a_count();
                }
            }
            aver /= count;

            return aver;
        }

};


template <int DATA_SIZE>
xsim_sc_node<DATA_SIZE>::xsim_sc_node(sc_module_name name, xsim_node_t *ptr_node)
: sc_module(name) {

    int i = 0;
    NB_IFACE = ptr_node->nb_iface;

    fifo_in      = new xsim_sc_fifo<DATA_SIZE>*[NB_IFACE];
    fifo_out     = new xsim_sc_fifo<DATA_SIZE>*[NB_IFACE];
    producer     = new xsim_sc_producer<DATA_SIZE, MAX_PACKET_LENGTH, MIN_PACKET_LENGTH>*[NB_IFACE];
    receiver     = new xsim_sc_receiver<DATA_SIZE>*[NB_IFACE];
    wrapper_send = new xsim_sc_wrapper_send<DATA_SIZE>*[NB_IFACE];
    wrapper_recv = new xsim_sc_wrapper_recv<DATA_SIZE>*[NB_IFACE];


    char fifo_out_str[30];
    char fifo_in_str[30];
    char producer_str[30];
    char receiver_str[30];
    char wrapper_send_str[30];
    char wrapper_recv_str[30];
    for (i=0 ; i<NB_IFACE ; i++) {
        sprintf(fifo_out_str, "fifo_out[%x/%x]", ptr_node->node_id,i);
        fifo_out[i] = new xsim_sc_fifo<DATA_SIZE>(fifo_out_str);

        sprintf(fifo_in_str, "fifo_in[%x/%x]", ptr_node->node_id,i);
        fifo_in[i] = new xsim_sc_fifo<DATA_SIZE>(fifo_in_str);

        sprintf(producer_str, "producer[%x/%x]", ptr_node->node_id,i);
        producer[i] = new xsim_sc_producer<DATA_SIZE, MAX_PACKET_LENGTH, MIN_PACKET_LENGTH>
            (producer_str, ptr_node->node_id, 50, ptr_node->nb_nodes);
        producer[i]->out(*fifo_out[i]);

        sprintf(receiver_str, "receiver[%x/%x]", ptr_node->node_id,i);
        receiver[i] = new xsim_sc_receiver<DATA_SIZE>
            (receiver_str, ptr_node->node_id);
        receiver[i]->in(*fifo_in[i]);

        sprintf(wrapper_send_str, "wrapper_send[%x/%x]", 
                ptr_node->node_id,i);
        wrapper_send[i] = new xsim_sc_wrapper_send<DATA_SIZE>
            (wrapper_send_str, ptr_node->iface[i]);
        wrapper_send[i]->in(*fifo_out[i]);

        sprintf(wrapper_recv_str, "wrapper_recv[%x/%x]", 
                ptr_node->node_id,i);
        wrapper_recv[i] = new xsim_sc_wrapper_recv<DATA_SIZE>
            (wrapper_recv_str, ptr_node->iface[i]);
        wrapper_recv[i]->out(*(fifo_in[i]));
    }

    /* for uniform */
    for (i=0 ; i<NB_IFACE ; i++) {
        producer[i]->unif();
    }

    /* for "localy" */
    //	int dist [NUM_Z*NUM_Y*NUM_X];
    //	for (int zz=0; zz<NUM_Z ; zz++)
    //		for (int yy=0; yy<NUM_Y ; yy++)
    //		        for (int xx=0; xx<NUM_X ; xx++)
    //				dist [zz*NUM_Y*NUM_X + yy*NUM_X + xx] = abs(zz-z) + abs(yy-y) + abs(xx-x);
    //	for (i=0 ; i<node->nb_iface ; i++) {
    //	    producer[i]->localize(dist);
    //	}

}


#endif
