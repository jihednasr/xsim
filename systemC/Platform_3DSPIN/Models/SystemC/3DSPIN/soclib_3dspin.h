/******************************************************************************
 * File : SOCLIB_3DSPIN.h
 * Date : 01/09/2009
 * Author : Hamed (Abbas) Sheibanyrad
 * This program is released under the GNU Public License
 * Copyright : TIMA
 *
 * The 3DSPIN is used to interconnect several subsystems, by a
 * network on chip respecting a three dimensional mesh topology.
 * Each network node is connected to a subsystem by two ports :
 * - IN : the input to the network
 * - OUT: the output from the network
 *
 * This component has five "template" parameters :
 * DATA_SIZE and FIFO_SIZE define the network's FIFOs width and depth
 * Z, Y, and X define the network dimensions (size)
 *
 *****************************************************************************/

#ifndef SOCLIB_3DSPIN_H
#define SOCLIB_3DSPIN_H

#include <math.h>
#include <stdlib.h>
#include <systemc.h>

#include "../Common/soclib_dspin_interfaces.h"
#include "soclib_3dspin_router.h"

template<int DATA_SIZE, 
	 unsigned short FIFO_SIZE, 
	 int Z, 
	 int Y,
         int X>

struct SOCLIB_3DSPIN : sc_module {

  // STRUCTURAL PARAMETERS
  const char* NAME;

  // EXTERNALS PORTS
  sc_in<bool> 	CLK;
  sc_in<bool> 	RESETN;
  DSPIN_IN <DATA_SIZE>  IN  [Z][Y][X];
  DSPIN_OUT<DATA_SIZE>  OUT [Z][Y][X];


  // INTERNAL SIGNALS
  DSPIN_SIGNALS<DATA_SIZE> dspin_req_E [Z+2][Y+2][X+2];
  DSPIN_SIGNALS<DATA_SIZE> dspin_req_N [Z+2][Y+2][X+2];
  DSPIN_SIGNALS<DATA_SIZE> dspin_req_W [Z+2][Y+2][X+2];
  DSPIN_SIGNALS<DATA_SIZE> dspin_req_S [Z+2][Y+2][X+2];
  DSPIN_SIGNALS<DATA_SIZE> dspin_req_U [Z+2][Y+2][X+2];
  DSPIN_SIGNALS<DATA_SIZE> dspin_req_D [Z+2][Y+2][X+2];


  SOCLIB_3DSPIN_ROUTER<DATA_SIZE, FIFO_SIZE> *cluster [Z][Y][X];

  public :

  /////////////////////////////////////////////////////////////////////////////
  //        CONSTRUCTOR
  /////////////////////////////////////////////////////////////////////////////

  SC_HAS_PROCESS(SOCLIB_3DSPIN);

  SOCLIB_3DSPIN(sc_module_name name) {

    int x, y, z;
    char cluster_name[20];
    NAME = (const char*) name;
    
    //// INSTANCIATED COMPONENTS

    for (x = 0; x < X; x++) {
      for (y = 0; y < Y; y++) {
        for (z = 0; z < Z; z++) {
	  sprintf(cluster_name, "cluster[%x][%x][%x]", z, y, x);
	  cluster[z][y][x] = new SOCLIB_3DSPIN_ROUTER<DATA_SIZE, FIFO_SIZE>( cluster_name, (z<<10)|(y<<5)|x );
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    //        NETLIST
    ///////////////////////////////////////////////////////////////////////////

    for (y = 0; y < Y; y++) {
      for (x = 0; x < X; x++) {
        for (z = 0; z < Z; z++) {
	  (cluster[z][y][x])->CLK           (CLK);
	  (cluster[z][y][x])->RESETN        (RESETN);
	  (cluster[z][y][x])->IN[LOCAL]     (IN  [z][y][x]);
	  (cluster[z][y][x])->OUT[LOCAL]    (OUT [z][y][x]);

	  (cluster[z][y][x])->IN[NORTH] (dspin_req_S	[z+1][y+2][x+1]);	
	  (cluster[z][y][x])->IN[SOUTH] (dspin_req_N	[z+1][y][x+1]);	
	  (cluster[z][y][x])->IN[EAST]  (dspin_req_W	[z+1][y+1][x+2]);	
	  (cluster[z][y][x])->IN[WEST]  (dspin_req_E	[z+1][y+1][x]);	
          (cluster[z][y][x])->IN[UP]    (dspin_req_D	[z+2][y+1][x+1]);
          (cluster[z][y][x])->IN[DOWN]  (dspin_req_U 	[z][y+1][x+1]);

	  (cluster[z][y][x])->OUT[NORTH](dspin_req_N	[z+1][y+1][x+1]);	
	  (cluster[z][y][x])->OUT[SOUTH](dspin_req_S	[z+1][y+1][x+1]);	
	  (cluster[z][y][x])->OUT[EAST] (dspin_req_E	[z+1][y+1][x+1]);	
	  (cluster[z][y][x])->OUT[WEST] (dspin_req_W	[z+1][y+1][x+1]);	
          (cluster[z][y][x])->OUT[UP]   (dspin_req_U 	[z+1][y+1][x+1]);
          (cluster[z][y][x])->OUT[DOWN] (dspin_req_D 	[z+1][y+1][x+1]);
	}
      }
    }

  }; // end constructor

  /////////////////////////////////////////////////////////////////////////////
  //        DESTRUCTOR
  /////////////////////////////////////////////////////////////////////////////

  ~SOCLIB_3DSPIN() {
    printf("destruction of the clusters\n");
    for (int x=0; x < X; x++) 
      for (int y=0; y < Y; y++)
        for (int z=0; z < Z; z++)
          delete(cluster[z][y][x]);  
  }; // end destructor

}; // end struct

#endif
