#ifndef _XSIM_SC_DEBUG_H_
#define _XSIM_SC_DEBUG_H_


#define EMSG(fmt, ...) fprintf(stderr, "[EE] (systemC) " fmt,	\
			        ##__VA_ARGS__);

#ifdef DEBUG_SYSTEMC
# define DMSG(fmt, ...) fprintf(stdout, "(systemC) " fmt,	\
                    ##__VA_ARGS__);
#else
# define DMSG(a...) do{}while(0)
#endif


#endif
