#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_

#include <xsim_time.h>

#define   time_window               100
#define   SYNCHRONIZATION_PERIODE   0       /* to remove or to leave at 0 - I don't remember why I thought it could fix some bug */

void      xsim_topology_init(xsim_t *xsim);
void      xsim_topology_free(xsim_t *xsim);

sim_time  xsim_topology_travel_time(uint32_t src, uint32_t dest);

void      xsim_topology_load_info(char *file, int nb_node);

#endif
