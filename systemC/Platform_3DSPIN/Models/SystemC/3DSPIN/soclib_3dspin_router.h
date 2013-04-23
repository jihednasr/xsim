/**************************************************************************
 * File : 3DSPIN_ROUTER.h
 * Date : 01/09/2009
 * authors : Alain Greiner & Romain Michard & Hamed (Abbas) Sheibanyrad
 * This program is released under the GNU public license
 * Copyright : UPMC - LIP6, TIMA
 *
 * The 3DSPIN router is used to interconnect several subsystems, by a
 * network on chip respecting a three dimensional mesh topology.
 * There is one 3DSPIN router for each sub-system. Each router is connected
 * to seven entities :
 * - the NORTH router, 
 * - the SOUTH router,
 * - the EAST router, 
 * - the WEST router,
 * - the UP router, 
 * - the DOWN router,
 * - the LOCAL sub-system.
 *
 * The 3DSPIN router contains a bi-synchronous FIFO on each input port,
 * but for sake of simplicity and due to the fact that in system-level 
 * simulations we don't need to take physical issues into account, 
 * this SystemC model is fully synchronous. 
 *
 * This component has two "template" parameters : DATA_SIZE 
 * and FIFO_SIZE define the FIFO width and depth.
 * It has one "constructor" parameters : IDENT define the router
 * local coordinates (X = 5 LSB bits / Y = 5 next bits / Z = 5 MSB bits).
 *
 * The destination coordinates are contained in the right byte of the
 * 3DSPIN packet header : |ZZZZZ|YYYYY|XXXXX| (5 bits for X, 5 bits for Y, 
 * and 4 bits for Z).
 *
 * The routing algorithm is X-first, and is totally determistic, in
 * order to garanty the "in order delivery property.
 * The allocation policy for a given output port requested by several
 * input ports is round-robin.
 **************************************************************************/

#ifndef SOCLIB_3DSPIN_ROUTER_H
#define SOCLIB_3DSPIN_ROUTER_H

#include <systemc.h>
#include "../Common/soclib_dspin_interfaces.h"
#include "../Common/soclib_generic_fifo.h"

  enum {
    NORTH = 0,
    SOUTH = 1,
    EAST  = 2,
    WEST  = 3,
    UP    = 4,
    DOWN  = 5,
    LOCAL = 6
  };

template<int DATA_SIZE,
	 unsigned short FIFO_SIZE>

struct SOCLIB_3DSPIN_ROUTER : sc_module {

// EXTERNAL PORTS
sc_in<bool>  CLK;
sc_in<bool>  RESETN;
DSPIN_IN<DATA_SIZE>  	IN [7];
DSPIN_OUT<DATA_SIZE>	OUT[7];


// REGISTERS
sc_signal<bool>	ALLOC_OUT[7];	// output ports allocation state 
sc_signal<bool>	ALLOC_IN [7];	// input ports allocation state 
sc_signal<int>	INDEX_OUT[7];	// index of the connected input FIFO 
sc_signal<int>	INDEX_IN [7];	// index of the connected output FIFO 


soclib_generic_fifo<FIFO_SIZE,DATA_SIZE>  FIFO[7];	// output fifos

// INSTANCE NAME & COORDINATES
const	char*	NAME;
int		XLOCAL;
int		YLOCAL;
int		ZLOCAL;


// MNEMONICS

/////////////////////////////////////////////
//           Transition()
/////////////////////////////////////////////
 
void Transition()
{
	int			i,j,k;
	int			xreq,yreq,zreq;
	bool			fifo_in_write [7];	// control signals
//	bool    		fifo_in_read  [7];	// for the input fifos
	sc_uint<DATA_SIZE>	fifo_in_data  [7];
	bool			fifo_out_write[7];	// control signals
	bool    		fifo_out_read [7];	// for the output fifos
	sc_uint<DATA_SIZE>	fifo_out_data [7];
	bool			req [7][7];		// REQ[i][j] signals from
							// input i to output j
if(RESETN == false) {

	for(i = 0 ; i < 7 ; i++) {
		ALLOC_IN[i] = false;
		ALLOC_OUT[i] = false;
		INDEX_IN[i] = 0;
		INDEX_OUT[i] = 0;
		FIFO[i].init();
	}

} else {

	// fifo_in_write[i] and fifo_in_data[i]
	for(i = 0 ; i < 7 ; i++) {
		fifo_in_write[i] = IN[i].WRITE.read();
		fifo_in_data[i] = IN[i].DATA.read();
	}

	// fifo_out_read[i] 
	for(i = 0 ; i < 7 ; i++) 
		fifo_out_read[i] = OUT[i].READ.read();
	
	
	// req[i][j]  : implement the X first routing algorithm
	for(i = 0 ; i < 7 ; i++) { // loop on the input ports
	        if((fifo_in_write[i] == true) &&
	                (((fifo_in_data[i] >> (DATA_SIZE-4) ) & DSPIN_BOP) == DSPIN_BOP)) {

	                xreq = (int) (fifo_in_data[i] & 0x1F);
	                yreq = (int)((fifo_in_data[i] >> 5) & 0x1F);
	                zreq = (int)((fifo_in_data[i] >> 10) & 0x1F);
	
			if(xreq < XLOCAL) {
			        req[i][LOCAL] = false;
				req[i][NORTH] = false;
				req[i][SOUTH] = false;
				req[i][EAST]  = false;
				req[i][WEST]  = true;
				req[i][UP]    = false;
				req[i][DOWN]  = false;
	                } else if(xreq > XLOCAL) {
	                        req[i][LOCAL] = false;
	                        req[i][NORTH] = false;
	                        req[i][SOUTH] = false;
	                        req[i][EAST]  = true;
	                        req[i][WEST]  = false;
	                        req[i][UP]    = false;
	                        req[i][DOWN]  = false;
			} else  if(yreq > YLOCAL) {
	                        req[i][LOCAL] = false;
	                        req[i][NORTH] = true;
	                        req[i][SOUTH] = false;
	                        req[i][EAST]  = false;
	                        req[i][WEST]  = false;
	                        req[i][UP]    = false;
	                        req[i][DOWN]  = false;
	                } else if(yreq < YLOCAL) {
	                        req[i][LOCAL] = false;
	                        req[i][NORTH] = false;
	                        req[i][SOUTH] = true;
	                        req[i][EAST]  = false;
	                        req[i][WEST]  = false;
	                        req[i][UP]    = false;
	                        req[i][DOWN]  = false;
	                } else if(zreq > ZLOCAL) {
	                        req[i][LOCAL] = false;
	                        req[i][NORTH] = false;
	                        req[i][SOUTH] = false;
	                        req[i][EAST]  = false;
	                        req[i][WEST]  = false;
	                        req[i][UP]    = true;
	                        req[i][DOWN]  = false;
	                } else if(zreq < ZLOCAL) {
	                        req[i][LOCAL] = false;
	                        req[i][NORTH] = false;
	                        req[i][SOUTH] = false;
	                        req[i][EAST]  = false;
	                        req[i][WEST]  = false;
	                        req[i][UP]    = false;
	                        req[i][DOWN]  = true;
	                } else  {
			        req[i][LOCAL] = true;
				req[i][NORTH] = false;
				req[i][SOUTH] = false;
				req[i][EAST]  = false;
				req[i][WEST]  = false;
	                        req[i][UP]    = false;
	                        req[i][DOWN]  = false;
			}
		} else {
				req[i][LOCAL] = false;
				req[i][NORTH] = false;
				req[i][SOUTH] = false;
				req[i][EAST]  = false;
				req[i][WEST]  = false;
	                        req[i][UP]    = false;
	                        req[i][DOWN]  = false;
		}
	} // end loop on the inputs
				
	// fifo_in_read[i]
//	for(i = 0 ; i < 7 ; i++) { // loop on the inputs
//		if(ALLOC_IN[i] == true) {
//			fifo_in_read[i] = FIFO[INDEX_IN[i]].wok();
//		} else {
//			fifo_in_read[i] = false;
//		}
//	} // end loop on the inputs
	
	// fifo_out_write[j] and fifo_out_data[j]
	for(j = 0 ; j < 7 ; j++) { // loop on the outputs
		if(ALLOC_OUT[j] == true) {
	                fifo_out_write[j] = fifo_in_write[INDEX_OUT[j]];
	                fifo_out_data[j]  = fifo_in_data[INDEX_OUT[j]];
		} else {
			fifo_out_write[j] = false;
		}
	} // end loop on the outputs
	
	// ALLOC_IN, ALLOC_OUT, INDEX_IN et INDEX_OUT : implements the round-robin allocation policy
	for(j = 0 ; j < 7 ; j++) { // loop on the outputs
		if(ALLOC_OUT[j] == false) { // possible new allocation
		  
			//Routage par round-robin
			for(k = INDEX_OUT[j]+1 ; k < (INDEX_OUT[j] + 8) ; k++) { // loop on the inputs
				i = k % 7;
				if(req[i][j] == true ) {
					ALLOC_OUT[j] = true;
					INDEX_OUT[j] = i;
					ALLOC_IN[i] = true;
					INDEX_IN[i] = j;
					break;
				}
			} // end loop on the inputs
		} else { // possible desallocation
	
	          if((((fifo_in_data[INDEX_OUT[j]] >> (DATA_SIZE-4) ) & DSPIN_EOP) == DSPIN_EOP) && (FIFO[j].wok() == true ) && fifo_in_write[INDEX_OUT[j]]) {
	
				ALLOC_OUT[j] = false;
				ALLOC_IN[INDEX_OUT[j]] = false;
			}
		}
	} // end loop on the outputs
	
	//  FIFOS
	for(i = 0 ; i < 7 ; i++) {
		if((fifo_out_write[i] == true ) && (fifo_out_read[i] == true ))
			{FIFO[i].put_and_get(fifo_out_data[i]);}
		if((fifo_out_write[i] == true ) && (fifo_out_read[i] == false))
			{FIFO[i].simple_put(fifo_out_data[i]);}
		if((fifo_out_write[i] == false) && (fifo_out_read[i] == true ))
			{FIFO[i].simple_get();}
	}

} // end else RESETN

};  // end Transition()

/////////////////////////////////////////////
//           GenMoore()
/////////////////////////////////////////////
 
void GenMoore()
{
int	i;

// input ports : READ signals
for(i = 0 ; i < 7 ; i++) { 
        if(ALLOC_IN[i] == true) {
                IN[i].READ  = FIFO[INDEX_IN[i]].wok();
        } else {
                IN[i].READ  = false;
        }
}

// output ports : DATA & WRITE signals
for(i = 0 ; i < 7 ; i++) { 
	OUT[i].DATA = FIFO[i].read(); 
	OUT[i].WRITE = FIFO[i].rok();
}

}; // end GenMoore()

////////////////////////////////////////
//           Constructor   
////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_3DSPIN_ROUTER);

SOCLIB_3DSPIN_ROUTER (sc_module_name name, int ident)
{

int 	x =  ident & 0x0000001F;
int	y = (ident & 0x000003e0) >> 5;
int	z = (ident & 0x00007c00) >> 10;

SC_METHOD(Transition);
sensitive_pos << CLK;

SC_METHOD(GenMoore);
sensitive_neg << CLK;

NAME = (const char *) name;
XLOCAL = x;
YLOCAL = y;
ZLOCAL = z;

printf("Successful instanciation of SOCLIB_3DSPIN_ROUTER [%x][%x][%x] : %s \n", z, y, x, NAME);

}; // end constructor

~SOCLIB_3DSPIN_ROUTER() {
}

}; // end struct 3DSPIN_ROUTER

#endif 
