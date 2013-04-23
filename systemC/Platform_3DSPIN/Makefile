PROG=simulation_2d_mesh_nodes_x_y

# configuration for sarrazin laptop
#TARGET_ARCH  		= linux
#SYSTEMC		 		= /home/sarrazin/ENSIMAG/PFE/SystemC/systemc-2.2.0

# configuration for nil 
TARGET_ARCH = linux64
SYSTEMC	    = /opt/systemc-2.3
#/altamaha/home/sarrazin/systemc-2.2.0

CC          		= g++
CFLAGS      		= -Wall -Wno-deprecated -L. -L$(SYSTEMC)/lib-$(TARGET_ARCH) -I. -I$(SYSTEMC)/include -g
CFLAGS             += -D_POSIX_C_SOURCE=199309L
LDFLAGS             = -lrt

$(PROG): system.cpp  
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ system.cpp -lsystemc -lm -lbfd -liberty 2>&1 | c++filt

clean :
	-rm -f *.o 
	-rm -f res_*
	-rm -f *.txt 
	-rm -f simulation_systemc.x 
	-rm -f core*
	-rm -f trace_file.vcd
	-rm -f TTY_* 
	-rm -f code-*.cc code-*.so
	
