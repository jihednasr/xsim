
/**************************************************************
 * File : dspin_interfaces.h
 * Date : 11/1/2005
 * Authors : A.Greiner & Ivan MIRO
 * It is released under the GNU Public License.
 * Copyright : UPMC/LIP6
 * 
 * This file defines the ports and signals 
 * data types for the distributed DSPIN network.
 * This is used by the dspin_router, dspin_vci_initiator_wrapper
 * and dspin_vci_target_wrapper components.
***************************************************************/

#ifndef SOCLIB_DSPIN_INTERFACES_H
#define SOCLIB_DSPIN_INTERFACES_H

#include <systemc.h>


/***  DSPIN Flags   ***/

enum{
	DSPIN_BOP 	= 0x1,
	DSPIN_EOP 	= 0x2,
	DSPIN_ERR	= 0x4,
	DSPIN_PAR	= 0x8
};

/***  DSPIN Signals ***/

template<int DATA_SIZE>

struct DSPIN_SIGNALS {
sc_signal<sc_uint<DATA_SIZE> >  DATA;    // data
sc_signal<bool>                 WRITE;   // write command 
sc_signal<bool>                 READ;    // read command
};


/***  DSPIN OUT Ports ***/

template<int DATA_SIZE>

struct DSPIN_OUT {
sc_out<sc_uint<DATA_SIZE> >     DATA;    // data
sc_out<bool>                    WRITE;   // valid data
sc_in<bool>                     READ;    // data accepted

void operator () (DSPIN_SIGNALS<DATA_SIZE> &sig) {
DATA         (sig.DATA);
WRITE        (sig.WRITE);
READ         (sig.READ);
};

void operator () (DSPIN_OUT<DATA_SIZE> &port) {
DATA         (port.DATA);
WRITE        (port.WRITE);
READ         (port.READ);
};

}; 


/*** DSPIN IN Ports ***/

template<int DATA_SIZE>

struct DSPIN_IN {
sc_in<sc_uint<DATA_SIZE> >     DATA;     // data
sc_in<bool>                    WRITE;    // valid data
sc_out<bool>                   READ;     // data accepted

void operator () (DSPIN_SIGNALS<DATA_SIZE> &sig) {
DATA         (sig.DATA);
WRITE        (sig.WRITE);
READ         (sig.READ);
};

void operator () (DSPIN_IN<DATA_SIZE> &port) {
DATA         (port.DATA);
WRITE        (port.WRITE);
READ         (port.READ);
};


}; 

#endif

