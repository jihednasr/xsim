#ifndef _XSIM_SC_TOPOLOGY_H_
#define _XSIM_SC_TOPOLOGY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

void xsim_topology_init(char *topology_file, int nb_nodes);
void xsim_topology_free(int nb_nodes);

int  xsim_topology_travel_time(uint32_t src, uint32_t dest);

void xsim_topology_load_info(char *file, int nb_node);

#ifdef __cplusplus
}
#endif

#endif
