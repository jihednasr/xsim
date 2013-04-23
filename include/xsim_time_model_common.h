#ifndef _XSIM_TIME_MODEL_COMMON_H_
#define _XSIM_TIME_MODEL_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif 

struct xsim_init_struct 
{
    int   nb_nodes;
    int   nb_ifaces;
    const char *topology;
    const char *measures_output;
    const char *gnuplot_output;
    int   simulation_time;
};
typedef struct xsim_init_struct xsim_init_struct_t;

/* Communication functions */
void broadcast(xsim_targets_t *targets, xsim_iface_t *iface, xsim_msg_type_t type);
void send_message(int dest, xsim_iface_t *iface, uint8_t *payload, uint32_t size);


/* Simulation component */
int  xsim_time_model_message_to_recv(xsim_node_t *node, int iface_nb, xsim_msg_t **msg); /* Must not free msg */
void xsim_time_model_message_to_send(xsim_node_t *node, int iface_nb, xsim_msg_list_t *to_send);
int  xsim_time_model_simulate_composant(xsim_node_t *node, xsim_msg_list_t **to_send, 
        sim_time *next_time_for_resource, int *stabilized);


/* Init and end son_body */
void xsim_time_model_init_son_body(xsim_t *xsim, int node_id, xsim_node_t **node, 
        xsim_msg_list_t ***to_send);
int  xsim_time_model_end_of_simulation(xsim_node_t *node, xsim_msg_list_t **to_send, 
        const char *output_measure);


/* test functions for correctness of the messages order */
void test_correctness_send(xsim_node_t *node, xsim_msg_t *msg, int iface_id);
void test_correctness_recv(xsim_node_t *node, int node_id, xsim_msg_t *msg);


/* Function for the main and main */
void read_argument(int argc, char **argv, xsim_init_struct_t *is);
int xsim_spawner(void (**simulation)(xsim_t*, int), int nb_nodes, int nb_ifaces, const char* topology);

/* Function and handler for SIGSEGV */
void install_handler();
void uninstall_handler();
void handler(int signo);

#ifdef __cplusplus
}
#endif 

#endif
