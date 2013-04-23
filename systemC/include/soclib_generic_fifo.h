
/**************************************************************************
 * File : soclib_generic_fifo.h
 * Date : 28/08/2003
 * author : Alain Greiner 
 * This program is released under the GNU public license
 * Copyright : UPMC - LIP6
 *
 * This model describes a generic snchronous FIFO.
 * The two parameters are the number of bit per word (NBIT) and 
 * the number of word (NWORD)
 * - The 4 methods init(), simple_put(), simple_get(), put_and_get()
 * change the internal state of the FIFO, but return no data or status ! 
 * - The 4 methods rok(), wok(), filled_status() and read() return a 
 * status or a data, but does not change the FIFO internal state!
 ***************************************************************************/

#ifndef SOCLIB_GENERIC_FIFO_H
#define SOCLIB_GENERIC_FIFO_H

template <unsigned int NWORD, unsigned int NBIT>
class soclib_generic_fifo {
    //class soclib_generic_fifo : sc_module {
    public:

        //////////////////////////// 
        // internal FIFO registers
        //////////////////////////// 

        sc_signal<sc_uint<NBIT> >   DATA[NWORD];
        sc_signal<int>          PTR;
        sc_signal<int>          PTW;
        sc_signal<int>          STATE;

        ///////////////////////
        ///////////////////////
        //  method init()
        ///////////////////////
        void init()
        {
            PTR = 0;
            PTW = 0;
            STATE = 0;
        }; // end init()

        ///////////////////////
        // method filled_status()
        ///////////////////////

        int filled_status()
        {
            return (int)STATE;
        };
        //end filled_status()

        ///////////////////////
        // method simple_put()
        ///////////////////////

        void simple_put(sc_uint<NBIT> din)
        {
            if (STATE != NWORD) {
                STATE = STATE + 1;
                PTW = (PTW + 1) % NWORD;
                DATA[PTW] = din;
            }
        }; // end simple_put()

        ///////////////////////
        // method simple_get()
        ///////////////////////
        void simple_get()
        {
            if (STATE != 0) {
                STATE = STATE - 1;
                PTR = (PTR + 1) % NWORD;
            }
        }; // end simple_get()

        //////////////////////////
        // method put_and_get()  
        //////////////////////////
        void put_and_get(sc_uint<NBIT> din)
        {
            if (STATE == NWORD) {
                STATE = STATE - 1;
                PTR = (PTR + 1) % NWORD;
            } else if (STATE == 0) {
                STATE = STATE + 1;
                PTW = (PTW + 1) % NWORD;
                DATA[PTW] = din;
            } else {
                PTR = (PTR + 1) % NWORD;
                PTW = (PTW + 1) % NWORD;
                DATA[PTW] = din;
            }
        }; // end put_and_get()

        ////////////////////////
        //  method rok()
        ///////////////////////
        bool rok()
        {
            if(STATE != 0)  return(true);
            else        return(false);
        }; // end rok()

        ////////////////////////
        //  method wok()  
        ///////////////////////
        bool wok()
        {
            if(STATE != NWORD)  return(true);
            else            return(false);
        }; // end wok()

        ////////////////////////
        //  method read()
        ///////////////////////
        sc_uint<NBIT> read()
        {
            return(DATA[PTR]);
        }; // end read()

        ///////////////////////
        //  method rename()
        ///////////////////////
#ifdef NONAME_RENAME
        void rename(char *name)
        {
            char newname[100];

            for (int i=0;i<(int)NWORD;i++)
            {
                sprintf (newname,"%s_DATA_%2.2d",name,i);
                DATA[i].rename(newname);
            }
            sprintf (newname,"%s_PTR",name);
            PTR.rename(newname);
            sprintf (newname,"%s_PTW",name);
            PTW.rename(newname);
            sprintf (newname,"%s_STATE",name);
            STATE.rename(newname);
        }
#endif

        ////////////////////////
        //  method rename()
        ///////////////////////
#if 0
        void rename (const char *newname)
        {
            sc_module::rename (newname);

            char temp[100];

            for (int i=0;i<(int)NWORD;i++)
            {
                sprintf (temp,"DATA_%2.2d",newname,i);
                DATA[i].rename(temp);
            }
            //  PTR.rename ("PTR");
            //  PTW.rename ("PTW");
            //  STATE.rename ("STATE");
        }


        ////////////////////////
        //  constructor()
        ///////////////////////
        soclib_generic_fifo(sc_module_name insname = sc_module_name (sc_gen_unique_name("soclib_generic_fifo")))
            : PTR("PTR"),
            PTW("PTW"),
            STATE("STATE")
    {
#ifdef NONAME_RENAME
        rename (insname);
#endif
    }
#endif

}; // end struct generic_fifo

#endif
