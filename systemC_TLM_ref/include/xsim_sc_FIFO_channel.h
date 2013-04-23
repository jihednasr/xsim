#ifndef _XSIM_SC_FIFO_CHANNEL_H_
#define _XSIM_SC_FIFO_CHANNEL_H_

#include <systemc.h>
#include <xsim_sc_interfaces.h>

template <int DATA_SIZE>
class xsim_sc_fifo: public sc_channel,
    public write_if<sc_uint<DATA_SIZE> >,
    public read_if<sc_uint<DATA_SIZE> >
{
    private:
        enum e {max_elements=10};
        sc_uint<DATA_SIZE> data[max_elements];
        int num_elements, first;
        sc_event write_event, read_event;
        bool fifo_empty() {return (num_elements==0);};
        bool fifo_full() {return (num_elements==max_elements);};

        sc_event *wake_up_node;

    public:
        SC_HAS_PROCESS(xsim_sc_fifo);
        xsim_sc_fifo(sc_module_name name) : sc_module(name), num_elements(0), first(0) {} ;
        
        void write(sc_uint<DATA_SIZE> c) {
            if (fifo_full())
                wait(read_event);
            data[(first+num_elements)%max_elements] = c;
            ++num_elements;
            write_event.notify();
        }

        void read(sc_uint<DATA_SIZE> &c) {
            if (fifo_empty())
                wait(write_event);
            c = data[first];
            --num_elements;
            first = (first+1)%max_elements;
            read_event.notify();
        }

        void reset() {
            num_elements = first = 0;
        }

        int num_available() {
            return num_elements;
        }

        void will_send() {
            if (wake_up_node == NULL) {
                fprintf(stderr, "wake_up_node event not register.\n");
            } else {
                wake_up_node->notify(SC_ZERO_TIME);
            }
        }

        void event_register(sc_event & ev) {
            wake_up_node = &ev;
        }

}; // end of class declarations


#endif
