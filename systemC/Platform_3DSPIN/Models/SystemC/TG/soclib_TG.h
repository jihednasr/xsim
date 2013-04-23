#define LIMIT_LOOP 8000
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
#include "../Common/soclib_generic_fifo.h"

template <int DSPIN_DATA_SIZE, int MAX_PACKET_LENGTH, int MIN_PACKET_LENGTH, int NUM_CORE>
struct SOCLIB_PRODUCER : sc_module {

    /*** Declaration des interfaces ***/
    sc_in<bool> CLK;
    sc_in<bool> RESETN;
    DSPIN_IN<DSPIN_DATA_SIZE>  targ_in;
    DSPIN_OUT<DSPIN_DATA_SIZE> init_out;
    soclib_generic_fifo <10,DSPIN_DATA_SIZE>    FIFO_OUT;
    soclib_generic_fifo <10,DSPIN_DATA_SIZE>    FIFO_IN; 

    /*** Instance name ***/
    const char *NAME;
    float Percent;
    sc_signal<int>	G_FSM, A_FSM;
    int ID, n, NUM, TOTAL, START, Cycle, A_COUNT, DELAY, *ADDR;
    sc_uint<DSPIN_DATA_SIZE>    hasan;
    bool writeok ;
    int Latency ;

    float P[NUM_CORE];

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

    SOCLIB_PRODUCER ( sc_module_name insname, int index, int *address, float PERCENT ) 
        // nom de l'instance 
    {
        SC_METHOD (transition);
        sensitive_pos << CLK;
        SC_METHOD (genMoore);
        sensitive_neg << CLK;

        ID = index ;
        ADDR = address;

        Percent = PERCENT;

        NAME = (char*) strdup(insname);  

        if (PERCENT > 75)
        {
            perror ("no good percent");
            exit(1);
        }

        if (MIN_PACKET_LENGTH < 3)
        {
            perror ("no good min length");
            exit(1);
        }

        if (NAME == NULL) 
        {
            perror("malloc");
            exit(1);
        }

    } // fin fonction de creation

    void timing ()
    {
        int lat;

        lat = ((0x10000 + Cycle - ((FIFO_IN.read()>>16) & 0xffff)) % 0x10000)-1;	// -1 is the FIFO latency of the destination node and does not belong to the path

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


    void localize (int dist [NUM_CORE])
    {
        double A=0;
        float tot;

        tot = 0;

        for (int i=0; i<NUM_CORE ; i++)
            if (i!=ID)
                A += 1/pow(2, abs(dist[i]-dist[ID]));

        for (int i=0; i<NUM_CORE ; i++)   
            if (i!=ID)
            {
                tot += 1/ (A*pow(2,abs(dist[i]-dist[ID])));
                P[i] = tot;
            }
    }

    int next_length ()
    {
        return (((float) random() / RAND_MAX) * (MAX_PACKET_LENGTH - MIN_PACKET_LENGTH + 1)) + MIN_PACKET_LENGTH ;
    }

    void next_time ()
    {
        int l;
        l = next_length () ;
        TOTAL += NUM ;
        START = (((float) random() / RAND_MAX) * ((float)(l+TOTAL)/Percent - START - NUM)) + 1 + START + NUM ;
        NUM = l;
    }

    int next_destination ()
    {
        double pr;
        pr = ((float) random() / RAND_MAX) ;
        int i;
        for (i=0; i<NUM_CORE ; i++)
            if (i!=ID)
                if (P[i] > pr) 
                {
                    return ADDR[i];
                }
        return 0;
    }

    void transition()
    {
        if(RESETN == false) 
        {
            G_FSM 	= G_IDLE;
            A_FSM	= A_IDLE;

            //localize();
            //unif();

            Latency = 0;

            START = 0;
            TOTAL = 0;
            NUM = 0;
            DELAY = ((float) random() / RAND_MAX) * ((float)((MAX_PACKET_LENGTH + MIN_PACKET_LENGTH)/2)/Percent) ;

            next_time ();

            Cycle = 0 ;
            A_COUNT = 0 ;
            FIFO_IN.init();
            FIFO_OUT.init();
        } 
        else 
        {
            Cycle ++;
            int work=0;
            while (work<LIMIT_LOOP) {
                work++;
            }
            switch (A_FSM)
            {
                case A_IDLE :
                    if (FIFO_IN.rok() == true)
                    {
                        A_FSM = A_DATA;
                        timing ();
                        if ((unsigned)ADDR[ID] != (FIFO_IN.read() & 0x7fff) )
                        { 
                            fprintf(stderr, "[%d] A message is not received by the right destinataire.\n", ID);
                            //exit(0);
                        }	

                        OK = FIFO_IN.read() & 0xffffffff;
                    }
                    break;
                case A_DATA :
                    if (FIFO_IN.rok() == true )
                    {
                        OK += FIFO_IN.read() & 0xffffffff;

                        if ((FIFO_IN.read() >> (DSPIN_DATA_SIZE-4) ) & DSPIN_EOP) 
                        {
                            A_FSM = A_IDLE;

                            if (OK)
                            { 
                                fprintf(stderr, "[%d] There is an error in the checksum\n", ID);
                                //exit(0);
                            }		
                        }			
                    }
                    break;
            }

            writeok = false;

            switch (G_FSM) 
            {
                case G_IDLE :
                    if (Cycle+1 >= START+DELAY) G_FSM = G_BOP;
                    break;   
                case G_BOP :
                    if (FIFO_OUT.wok() == true)
                    {
                        G_FSM = G_DATA; 
                        n = 1;
                        writeok = true;
                        hasan = ((sc_uint<DSPIN_DATA_SIZE>) DSPIN_BOP << (DSPIN_DATA_SIZE-4)) + next_destination() ;//+ ((ID&0x7fff)<<16);
                        hasan |= (sc_uint<DSPIN_DATA_SIZE>) 
                            ((Cycle+1)&0xffff)
                            //((START+DELAY+1)&0xffff)
                            <<16; // +1 is the FIFO latency of the source node and does not belong to the path
                        Checker = hasan & 0xffffffff;
                    }
                    break;
                case G_DATA :
                    if (FIFO_OUT.wok() == true)
                    {
                        n++;
                        writeok = true;
                        if (NUM-1 == n) G_FSM = G_EOP;
                        hasan = Cycle ; 
                        Checker += hasan & 0xffffffff;
                    }
                    break;
                case G_EOP :
                    if(FIFO_OUT.wok()  == true) 
                    { 
                        G_FSM = G_IDLE; 
                        next_time ();
                        writeok = true;
                        hasan = ((sc_uint<DSPIN_DATA_SIZE> ) DSPIN_EOP << (DSPIN_DATA_SIZE-4) ) + ((-1*Checker)&0xffffffff);
                    }
                    break;
            } // end switch G_FSM

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
        targ_in.READ = FIFO_IN.wok();
        init_out.DATA = FIFO_OUT.read();
        init_out.WRITE =  FIFO_OUT.rok();
    }

    ~SOCLIB_PRODUCER() {
    }; // end 
};

#endif
