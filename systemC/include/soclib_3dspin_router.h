#ifndef SOCLIB_3DSPIN_ROUTER_H
#define SOCLIB_3DSPIN_ROUTER_H


#include <systemc.h>


#include <soclib_dspin_interfaces.h>
#include <soclib_generic_fifo.h>
#include <xsim_sc_wrapper_msg_to_send.h>
#include <xsim_sc_wrapper_msg_to_recv.h>

#include <xsim_iface.h>

template<int DATA_SIZE>
struct SOCLIB_3DSPIN_ROUTER : sc_module {

    // EXTERNAL PORTS
    sc_in<bool>  CLK;
    sc_in<bool>  RESETN;
    DSPIN_IN<DATA_SIZE>  	IN;
    DSPIN_OUT<DATA_SIZE>	OUT;

    sc_in<bool>  SENDER_CLK;
    sc_out<bool> WRAPPER_CLK;

    // INSTANCE NAME & COORDINATES
    const	char*	NAME;
    int		XLOCAL;
    int		YLOCAL;
    int		ZLOCAL;

    Xsim_sc_wrapper_msg_to_recv<DATA_SIZE> *recv_wrapper;
    Xsim_sc_wrapper_msg_to_send<DATA_SIZE> *send_wrapper;

    ////////////////////////////////////////
    //           Constructor   
    ////////////////////////////////////////

    SC_HAS_PROCESS(SOCLIB_3DSPIN_ROUTER);

    SOCLIB_3DSPIN_ROUTER (sc_module_name name, int ident, xsim_iface_t *ptr_iface, int *address_table)
    {
        int	x =  ident & 0x0000001F;
        int	y = (ident & 0x000003e0) >> 5;
        int	z = (ident & 0x00007c00) >> 10;

        char wrapper_recv[30];
        char wrapper_send[30];
        sprintf (wrapper_recv, "wrapper_recv[%x][%x][%x][%x]",z,y,x, ptr_iface->x_id);
        sprintf (wrapper_send, "wrapper_send[%x][%x][%x][%x]",z,y,x, ptr_iface->x_id);
        recv_wrapper = new Xsim_sc_wrapper_msg_to_recv<DATA_SIZE>(wrapper_recv, ident, ptr_iface);
        send_wrapper = new Xsim_sc_wrapper_msg_to_send<DATA_SIZE>(wrapper_send, ident, ptr_iface, address_table);

        recv_wrapper->CLK       (CLK);
        recv_wrapper->RESETN    (RESETN);
        recv_wrapper->OUT       (OUT);

        send_wrapper->CLK       (CLK);
        send_wrapper->RESETN    (RESETN);
        send_wrapper->IN        (IN);
        send_wrapper->WRAPPER_CLK (WRAPPER_CLK);
        send_wrapper->SENDER_CLK  (SENDER_CLK);

        NAME = (const char *) name;
        XLOCAL = x;
        YLOCAL = y;
        ZLOCAL = z;

        printf("Successful instanciation of SOCLIB_3DSPIN_ROUTER [%x][%x][%x] : %s \n", z, y, x, NAME);

    }; // end constructor

    ~SOCLIB_3DSPIN_ROUTER() {
        delete recv_wrapper;
        delete send_wrapper;
    }

}; // end struct 3DSPIN_ROUTER

#endif 
