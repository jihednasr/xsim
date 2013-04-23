#ifndef XSIM_SC_CLK_H_
#define XSIM_SC_CLK_H_

#include <systemc.h>

SC_MODULE(Xsim_sc_clk) {

    sc_out<bool> signal_clk;
    int id, counter;

    SC_HAS_PROCESS(Xsim_sc_clk);
    Xsim_sc_clk(sc_module_name name, sc_signal<bool> &clk, int node_id) : sc_module(name) {
        signal_clk(clk);
        id = node_id;
        signal_clk = false;
        counter = 0;
        SC_THREAD(CLK_Manager);
    }


    void CLK_Manager() {
        while (true) {
            if (counter == 2000) {
                fprintf(stdout, "************ <%d> TIMING : %5lld **************\n",
                        id, sc_time_stamp().value());
            }
            if (signal_clk == false) {
               DMSG("************ <%d> TIMING : %5lld **************\n",
                        id, sc_time_stamp().value());
            }
            signal_clk = !signal_clk;
            wait(BASE_TIME, TIMING);
            counter++;
        }
    }


};


#endif
