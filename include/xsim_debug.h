#ifndef _XSIM_DEBUG_H_
#define _XSIM_DEBUG_H_

#include <stdio.h>

#ifndef DBG_HDR
#define DBG_HDR __FILE__
#endif

#if (defined HUGE_DEBUG) && !(defined DEBUG)
#define DEBUG
#endif

#define DBG_HDR_SZ "23"


/* Error reporting macro */
#define EMSG(fmt, ...) fprintf(stderr, "[EE] (%-"DBG_HDR_SZ"s) " fmt,	\
			       DBG_HDR, ##__VA_ARGS__);

/* Simple debug messages reporting macro */
#ifdef DEBUG
# define DMSG(fmt, ...) fprintf(stdout, "(%-"DBG_HDR_SZ"s) " fmt,	\
				DBG_HDR, ##__VA_ARGS__);
#else
# define DMSG(a...) do{}while(0)
#endif

/* Heavy debug messages reporting macro */
#ifdef HUGE_DEBUG
# define HMSG(fmt, ...) fprintf(stdout, "(%-"DBG_HDR_SZ"s) " fmt,	\
				DBG_HDR, ##__VA_ARGS__);
#else
# define HMSG(a...) do{}while(0)
#endif

/* Informative messages reporting */
# define IMSG(fmt, ...) fprintf(stdout, "(%-"DBG_HDR_SZ"s) " fmt,	\
				DBG_HDR, ##__VA_ARGS__);

#endif /* _XSIM_DEBUG_H */
