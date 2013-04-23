#ifndef _XSIM_SC_3DSPIN_CONFIG_H_
#define _XSIM_SC_3DSPIN_CONFIG_H__

/***  DSPIN Flags   ***/
enum header{
    DSPIN_BOP 	= 0x1,
    DSPIN_EOP 	= 0x2,
    DSPIN_ERR	= 0x4,
    DSPIN_PAR	= 0x8
};


#define NUM_X_MAX               4
#define NUM_Y_MAX               4
#define NUM_Z_MAX               4

#define MIN_PER                 40
#define MAX_PER                 41

#define MAX_PACKET_LENGTH       16
#define MIN_PACKET_LENGTH       16	// the minimum length is 3

#define DSPIN_DATA_SIZE         36
#define FIFO_SIZE               8

#define ADDR_MASK               0x7FFF
#define FIRST_BYTE_MASK         0x0FF000000
#define SECOND_BYTE_MASK        0x000FF0000
#define THIRD_BYTE_MASK         0x00000FF00
#define FOURTH_BYTE_MASK        0x0000000FF

#define TIMING                  SC_PS
#define BASE_TIME               1
#define SC2XSIM(variable_sc_for_time)       ((variable_sc_for_time).value())
#define XSIM2SC(variable_xsim_for_time)     sc_time((double)(variable_xsim_for_time), TIMING)

#define PACKET_SIZE             4

#endif
