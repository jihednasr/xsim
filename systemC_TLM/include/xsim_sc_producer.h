#ifndef _XSIM_SC_PRODUCER_H_
#define _XSIM_SC_PRODUCER_H_

#include <systemc.h>
#include <xsim_sc_interfaces.h>

#include <xsim_sc_debug.h>

template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
class xsim_sc_producer : sc_module {
    private:
        const char *NAME;
        float Percent;
        int ID;
        int NUM_CORE;
        float *P;


        /* variable to compute the message caracteristiques */
        int length, TOTAL, DELAY;
        sc_time send_next_message;

    public:
        sc_port< write_if<sc_uint<DATA_SIZE> > >out;

        SC_HAS_PROCESS(xsim_sc_producer);
        xsim_sc_producer(sc_module_name insname, int identity, 
                float PERCENT, int nb_core ) : sc_module(insname) {

            NAME = (char*) strdup(insname);  
            ID = identity;
            Percent = PERCENT;
            NUM_CORE = nb_core;
            P = new float[NUM_CORE];

            SC_THREAD(produceur_thread);

            if (PERCENT > 75) {
                perror ("no good percent");
                exit(1);
            }
            if (MIN_P_LENGTH < 3) {
                perror ("no good min length");
                exit(1);
            }
            if (NAME == NULL) {
                perror("malloc");
                exit(1);
            }
        };

        void set_percent(float per) {Percent = per;};
        void unif ();
        void localize (int *dist); // [NUM_CORE]); should be of size NUM_CORE

    private:
        void produceur_thread();

        int next_length ();
        void next_time ();
        int next_destination ();


};


template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
void 
xsim_sc_producer<DATA_SIZE, MAX_P_LENGTH, MIN_P_LENGTH>::unif ()
{ 
    double A;
    float tot;

    A = (*this).NUM_CORE-1;
    tot = 0;

    for (int i=0; i<NUM_CORE ; i++) {
        if (i!=ID) {
            tot += 1/A;
            P[i] = tot;
        }
    }
    return;
}


template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
void 
xsim_sc_producer<DATA_SIZE, MAX_P_LENGTH, MIN_P_LENGTH>::localize (int *dist)// [NUM_CORE])
{
    double A=0;
    float tot;

    tot = 0;

    for (int i=0; i<NUM_CORE ; i++)
        if (i!=ID)
            A += 1/pow(2, abs(dist[i]-dist[ID]));

    for (int i=0; i<NUM_CORE ; i++){
        if (i!=ID) {
            tot += 1/ (A*pow(2,abs(dist[i]-dist[ID])));
            P[i] = tot;
        }
    }
    return;
}

template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
int 
xsim_sc_producer<DATA_SIZE, MAX_P_LENGTH, MIN_P_LENGTH>::next_length ()
{
    return (((float) random() / RAND_MAX) * (MAX_P_LENGTH - MIN_P_LENGTH + 1)) + MIN_P_LENGTH ;
}

template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
void 
xsim_sc_producer<DATA_SIZE, MAX_P_LENGTH, MIN_P_LENGTH>::next_time ()
{
    int end_last_message = send_next_message.value() + length;
    TOTAL += length ;
    length = next_length () ;
    send_next_message = sc_time((((float) random() / RAND_MAX) * ((float)(length+TOTAL)/Percent - end_last_message)) + 1 + end_last_message, TIMING);

}

template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
int 
xsim_sc_producer<DATA_SIZE, MAX_P_LENGTH, MIN_P_LENGTH>::next_destination ()
{
    double pr;
    pr = ((float) random() / RAND_MAX) ;
    int i;
    for (i=0; i<NUM_CORE ; i++)
        if (i!=ID)
            if (P[i] > pr) 
            {
                return i;
            }
    return 0;
}

template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH>
void 
xsim_sc_producer<DATA_SIZE, MAX_P_LENGTH, MIN_P_LENGTH>::produceur_thread() {
    sc_uint<DATA_SIZE> output = 0;
    int send = 0;
    int checker = 0;
    out->reset();

    while (true) {
        /* wait next sending */
        next_time ();
        DMSG("[%d] producteur thread is going to sleep for %lld.\n", 
                ID, send_next_message.value() - sc_time_stamp().value());
        wait(send_next_message - sc_time_stamp());
        DMSG("[%d] producteur thread is woken-up.\n", ID);

        if (sc_time_stamp() != send_next_message) {
            EMSG("<%d> Error: send message not at the right moment.\n",
                    ID);
        }
        send = 0;
        sc_dt::uint64 current_time = send_next_message.value();

        /* begin of packet */
        int dest = next_destination();
        printf("[%d] (%lld) Producer send a message for node %d.\n",
                ID, sc_time_stamp().value(), dest);
        output = ((sc_uint<DATA_SIZE>) DSPIN_BOP << (DATA_SIZE-4)) + dest;//+ ((ID&0x7fff)<<16);
        output |= (sc_uint<DATA_SIZE>) ((current_time+send)&0xffff)<<16;
        //((START+DELAY+1)&0xffff)<<16;
        checker = output & 0xffffffff;
        out->write(output);
        send++;

        /* data */
        while (send != length-1) {
            output  = current_time + send;
            checker += output & 0xffffffff;
            out->write(output);
            send++;
        } 

        /* end of packet */
        output = ((sc_uint<DATA_SIZE> ) DSPIN_EOP << (DATA_SIZE-4) ) + ((-1*checker)&0xffffffff);
        out->write(output);
    }

}



#endif
