#ifndef _XSIM_SC_FIFO_TIME_H_
#define _XSIM_SC_FIFO_TIME_H_

#include <systemc.h>
#include <xsim_sc_interfaces.h>

template <int DATA_SIZE>
class xsim_sc_fifo_time: public sc_channel,
    public write_if<sc_uint<DATA_SIZE> >,
    public read_if<sc_uint<DATA_SIZE> >
{
    private:
        enum e {max_elements=20};
        sc_uint<DATA_SIZE> data_to_transmit[max_elements];
        sc_uint<DATA_SIZE> data_received[max_elements];

        int num_elements_received;
        int num_elements_to_transmit;
        int first_received;
        int first_to_transmit;

        bool fifo_empty() {return (num_elements_to_transmit==0);};
        bool fifo_full() {return (num_elements_received==max_elements);};
        bool fifo_to_transmit_full() {
            return (num_elements_to_transmit == max_elements);
        }
        sc_time time_delay;

        int tmp_length;
        sc_uint<DATA_SIZE> tmp_table[max_elements];

        sc_event *wake_up_node;
        sc_event write_event, read_event;
        sc_event data_in_tmp;
        sc_event copy_free;

    public:
        SC_HAS_PROCESS(xsim_sc_fifo_time);
        xsim_sc_fifo_time(sc_module_name name, int delay) 
            : sc_module(name), 
            num_elements_received(0), num_elements_to_transmit(0),
            first_received(0), first_to_transmit(0),
            tmp_length(0) { 

            time_delay = sc_time(delay, TIMING); 
            SC_THREAD(delay_thread);
        }
        
        void write(sc_uint<DATA_SIZE> c) {
            if (fifo_full()) {
                fprintf(stderr, "[EE] FIFO reception error: full but not design to be.\n");
                copy(first_received, num_elements_received);
                first_received += num_elements_received;
                num_elements_received = 0;
            }
            data_received[(first_received+num_elements_received)%max_elements] = c;
            num_elements_received++;
            if (((c >> (DATA_SIZE - 4)) & DSPIN_EOP) == DSPIN_EOP) {
                copy(first_received, num_elements_received);
                first_received += num_elements_received;
                num_elements_received = 0;
            }
        }

        void copy(int first, int length) {
            if (tmp_length != 0) {
                fprintf(stderr, "[EE] FIFO_time error: 2 messages send are overlaping.\n");
                wait(copy_free);
            }
            tmp_length = length;
            for (int i=0 ; i<length ; i++) {
                tmp_table[i] = data_received[(first+i)%max_elements];
            }
            data_in_tmp.notify();
        }

        void delay_thread() {
            while (1) {
                wait(data_in_tmp);
                wait(time_delay);
                for (int i=0 ; i<tmp_length ; i++) {
                    if (fifo_to_transmit_full()) {
                        fprintf(stderr, "fifo_to_transmit is full\n");
                        wait(read_event);
                    }
                    data_to_transmit
                        [(first_to_transmit+num_elements_to_transmit)%max_elements] = 
                        tmp_table[i];
                    num_elements_to_transmit++;
                }
                tmp_length = 0;
                copy_free.notify();
                write_event.notify();
                if (wake_up_node == NULL) {
                    fprintf(stderr, "wake_up_node event not register.\n");
                } else {
                    wake_up_node->notify(SC_ZERO_TIME);
                }
            }
        }

        void read(sc_uint<DATA_SIZE> &c) {
            if (fifo_empty())
                wait(write_event);
            c = data_to_transmit[first_to_transmit];
            --num_elements_to_transmit;
            first_to_transmit = (first_to_transmit+1)%max_elements;
            read_event.notify();
        }

        void reset() {
            num_elements_to_transmit = 0;
            num_elements_received = 0;
            first_received = 0;
            first_to_transmit = 0;
            tmp_length = 0;
        }

        int num_available() {
            return num_elements_to_transmit;
        }

        void event_register(sc_event & ev) {
            wake_up_node = &ev;
        }

}; // end of class declarations


#endif
