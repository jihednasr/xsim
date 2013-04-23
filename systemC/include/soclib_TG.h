#define LIMIT_LOOP 0
////////////////////////////////////////////
//// Sahar Foroutan & Hamed Sheibanyrad ////
////////////////////////////////////////////


#ifndef SOCLIB_PRODUCER_H
#define SOCLIB_PRODUCER_H

#include <signal.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <systemc.h>
#include <stdio.h>
#include <soclib_generic_fifo.h>

#include <xsim_sc_3dspin_config.h>
#include <xsim_sc_debug.h>

template <int DATA_SIZE, int MAX_P_LENGTH, int MIN_P_LENGTH> //, int NUM_CORE>
struct SOCLIB_PRODUCER : sc_module {

    /*** Declaration des interfaces ***/
    sc_in<bool> CLK;
    sc_in<bool> RESETN;
    DSPIN_IN<DATA_SIZE>  targ_in;
    DSPIN_OUT<DATA_SIZE> init_out;
    soclib_generic_fifo <10,DATA_SIZE>    FIFO_OUT;
    soclib_generic_fifo <10,DATA_SIZE>    FIFO_IN; 

    sc_out<bool> SENDER_CLK;
    sc_in<bool>  WRAPPER_CLK;
    sc_signal<bool> internal_signal;

    /*** Instance name ***/
    //const char *NAME;
    float Percent;
    sc_signal<int>	A_FSM;
    int G_FSM;
    int ID, n, NUM, TOTAL, A_COUNT, DELAY, *ADDR;
    uint64 START;
    sc_uint<DATA_SIZE>    hasan;
    bool writeok ;
    int Latency ;

    int NUM_CORE;
    float *P;
    //float P[NUM_CORE];

    int Checker, OK;

    enum
    {
        G_IDLE       	= 0,
        G_BOP		= 1,
        G_DATA		= 2,
        G_EOP	  	= 3
    };
    enum
    {
        A_IDLE          = 0,
        A_DATA          = 1
    };
    /*** Constructeur et Destructeur ***/
    SC_HAS_PROCESS (SOCLIB_PRODUCER);
    /******************************************************************
      Fonction de creation d'une instance du composant simpletarget 
     ******************************************************************/

    SOCLIB_PRODUCER ( sc_module_name insname, int index, int *address, float PERCENT, int nb_core ) : sc_module(insname)
        // nom de l'instance 
    {
        SC_METHOD(transition_reception);
        sensitive_pos << CLK;
        
        SC_METHOD (transition_sending);
        sensitive_pos << CLK;
        sensitive << WRAPPER_CLK;
        SC_METHOD (genMoore);
        sensitive_neg << CLK;
        sensitive << internal_signal;

        ID = index ;
        ADDR = address;

        Percent = PERCENT;

        NUM_CORE = nb_core;
        P = new float[NUM_CORE];

        //NAME = (char*) strdup(insname);  

        if (PERCENT > 75)
        {
            perror ("no good percent");
            exit(1);
        }

        if (MIN_P_LENGTH < 3)
        {
            perror ("no good min length");
            exit(1);
        }

       // if (NAME == NULL) 
       // {
       //     perror("malloc");
       //     exit(1);
       // }

    } // fin fonction de creation

    void timing ()
    {
        int lat;

        lat = ((0x10000 + sc_time_stamp().value() - ((FIFO_IN.read()>>16) & 0xffff)) % 0x10000)-1;	// -1 is the FIFO latency of the destination node and does not belong to the path

        //    if( (FIFO_IN.read()>>16) & 0xffff)
        {
            Latency += lat;
            A_COUNT ++;
        }                                
    }

    void unif ()
    { 

        double A;
        float tot;

        A = NUM_CORE-1;
        tot = 0;

        for (int i=0; i<NUM_CORE ; i++)
            if (i!=ID)
            {
                tot += 1/A;
                P[i] = tot;
            }
    }


//    void localize (int dist [NUM_CORE])
//    {
//        double A=0;
//        float tot;
//
//        tot = 0;
//
//        for (int i=0; i<NUM_CORE ; i++)
//            if (i!=ID)
//                A += 1/pow(2, abs(dist[i]-dist[ID]));
//
//        for (int i=0; i<NUM_CORE ; i++)   
//            if (i!=ID)
//            {
//                tot += 1/ (A*pow(2,abs(dist[i]-dist[ID])));
//                P[i] = tot;
//            }
//    }

    int next_length ()
    {
        int r = 0;
        do {
            r=rand();
        } while ((r==RAND_MAX) || (r==0));
        int res = (int) ( (((float) r / RAND_MAX) * (MAX_P_LENGTH - MIN_P_LENGTH + 1)) + MIN_P_LENGTH );
        return (res > MAX_P_LENGTH) ? MAX_P_LENGTH : res; 
        /* It seems there are some special cases where "r/RAND_MAX" is so near 1 that the cast in int changes it to 1 */
    }

    void next_time ()
    {
        int l;
        l = next_length () ;
        if (l != MAX_P_LENGTH) {
            EMSG("[%d] Error: the length of the next message will be %d but it is greater than %d.\n", ID, l, MAX_P_LENGTH);
        }
        TOTAL += 2*BASE_TIME*NUM ;
        START = (((float) rand() / RAND_MAX) * ((float)(2*BASE_TIME*l+TOTAL)/Percent - START - 2*BASE_TIME*NUM)) + 1 + START + 2*BASE_TIME*NUM ;
        NUM = l;
    }

    int next_destination ()
    {
        double pr;
        pr = ((float) rand() / RAND_MAX) ;
        int i;
        for (i=0; i<NUM_CORE ; i++)
            if (i!=ID)
                if (P[i] > pr) 
                {
                    return ADDR[i];
                }
        return 0;
    }


    void transition_reception() {
        int nb_packet_recv=0;
        if(RESETN == false) {
            A_FSM	= A_IDLE;

            Latency = 0;
            A_COUNT = 0 ;
            FIFO_IN.init();

        } else {
            int work = 0;
            while (work < LIMIT_LOOP) {
                work++;
            }
            switch (A_FSM) {
                case A_IDLE :
                    DMSG("[%d] A_IDLE\n", ID);
                    if (FIFO_IN.rok() == true)
                    {
                        DMSG(stdout, "[%d] Receive a message. First data: %llX\n", 
                                ID, (unsigned long long) FIFO_IN.read());
                        A_FSM = A_DATA;
                        timing ();
                        if ((unsigned)ADDR[ID] != (FIFO_IN.read() & 0x7fff) )
                        { 
                            EMSG("[%x] Error: message not received by the right node.\n", ID);
                           // exit(0);
                        }	

                        nb_packet_recv++;
                        OK = FIFO_IN.read() & 0xffffffff;
                    }
                    break;
                case A_DATA :
                    DMSG("[%d] A_DATA\n", ID);
                    if (FIFO_IN.rok() == true )
                    {
                        OK += FIFO_IN.read() & 0xffffffff;
                        DMSG("[%d] data received: %llX \n", 
                                ID, (unsigned long long)(FIFO_IN.read()));
                        nb_packet_recv++;

                        if ((FIFO_IN.read() >> (DATA_SIZE-4) ) & DSPIN_EOP) {
                            A_FSM = A_IDLE;

                            if (OK) { 
                                EMSG("[%X] Error on the checksum. (receive %d packets)\n", ID, nb_packet_recv);
                                //exit(0);
                            } else {
                                fprintf(stdout, "[%d] (%lld) The whole message is received.\n", 
                                        ID, (unsigned long long) sc_time_stamp().value());
                            }
                            nb_packet_recv = 0;
                        }			
                    }
                    break;
            }

            if((targ_in.WRITE == true ) && (FIFO_IN.rok() == true ))
            {
                FIFO_IN.put_and_get( targ_in.DATA.read() );
            }
            if((targ_in.WRITE == true ) && (FIFO_IN.rok()  == false))
            {
                FIFO_IN.simple_put( targ_in.DATA.read() );
            }
            if((targ_in.WRITE == false) && (FIFO_IN.rok()  == true ))
            {
                FIFO_IN.simple_get();
            }
        }
    }

    void transition_sending()
    {
//        if (WRAPPER_CLK == true) {
//            DMSG("[%d] TG::transition_sending ; wrapper_clk true\n", ID);
//        } else {
//            DMSG("[%d] TG::transition_sending ; wrapper_clk false\n", ID);
//        }
        int destination = 0;
        if(RESETN == false) 
        {
            G_FSM 	= G_IDLE;

            //localize();
            //unif();

            START = 0;
            TOTAL = 0;
            NUM = 0;
            DELAY = ((float) rand() / RAND_MAX) * ((float)((MAX_P_LENGTH + MIN_P_LENGTH)/2)/Percent) ;

            next_time ();

            FIFO_OUT.init();
        } 
        else 
        {
            uint64 current_time = sc_time_stamp().value();
            if (G_FSM == G_DATA) {
                DMSG("[%d] (%lld) Producer is sending a message, sendtime: %lld.\n", 
                        ID, (unsigned long long)sc_time_stamp().value(), (unsigned long long)current_time+n);
            }

            writeok = false;
            switch (G_FSM) 
            {
                case G_IDLE :
                    if (current_time+1 >= START+DELAY) G_FSM = G_BOP;
                    break;   
                case G_BOP :
                    DMSG("[%d] G_BOP\n", ID);
                    if (FIFO_OUT.wok() == true)
                    {
                        G_FSM = G_DATA; 
                        n = 1;
                        writeok = true;
                        destination = next_destination();
                        fprintf(stdout, "[%d] (%lld) Producer is sending a message to node %d\n",
                                ID, (unsigned long long)sc_time_stamp().value(), destination);
                        hasan = ((sc_uint<DATA_SIZE>) DSPIN_BOP << (DATA_SIZE-4)) + destination ;//+ ((ID&0x7fff)<<16);
                        hasan |= (sc_uint<DATA_SIZE>) 
                            ((current_time+1)&0xffff)
                            //((START+DELAY+1)&0xffff)
                            <<16; // +1 is the FIFO latency of the source node and does not belong to the path
                        Checker = hasan & 0xffffffff;
                    }
                    internal_signal = !internal_signal;
                    break;
                case G_DATA :
                    DMSG("[%d] G_DATA\n", ID);
                    if (FIFO_OUT.wok() == true)
                    {
                        n++;
                        writeok = true;
                        if (NUM-1 == n) G_FSM = G_EOP;
                        hasan = current_time+n ; 
                        Checker += hasan & 0xffffffff;
                    }
                    internal_signal = !internal_signal;
                    break;
                case G_EOP :
                    if(FIFO_OUT.wok()  == true) 
                    { 
                        n++;
                        DMSG("[%d] EOP, n = %d\n", ID, n);
                        G_FSM = G_IDLE; 
                        next_time ();
                        writeok = true;
                        hasan = ((sc_uint<DATA_SIZE> ) DSPIN_EOP << (DATA_SIZE-4) ) + ((-1*Checker)&0xffffffff);
                    }
                    internal_signal = !internal_signal;
                    break;
            } // end switch G_FSM


            if((writeok == true ) && (init_out.READ.read() == true ))
            {
                FIFO_OUT.put_and_get(hasan);
            }
            if((writeok == true ) && (init_out.READ.read() == false))
            {
                FIFO_OUT.simple_put(hasan);
            }
            if((writeok == false) && (init_out.READ.read() == true ))
            {
                FIFO_OUT.simple_get();
            }
        }
    }

    void genMoore()
    {
        DMSG("[%d] TG::genMoore\n", ID);
        targ_in.READ = FIFO_IN.wok();
        init_out.DATA = FIFO_OUT.read();
        init_out.WRITE =  FIFO_OUT.rok();
        if (FIFO_OUT.rok() == true) {
            DMSG("[%d] (%lld) send a data n°%d.\n",
                    ID, sc_time_stamp().value(), n);
            SENDER_CLK = ! SENDER_CLK;
        }
    }

    ~SOCLIB_PRODUCER() {
        delete [] P;
    }; // end 
};

#endif
