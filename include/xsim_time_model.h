#ifndef _XSIM_TIME_MODEL_H_
#define _XSIM_TIME_MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif 
    
#define END_OF_SIMULATION 	-1
#define CONTINUE_SIMULATION	 0


/* Must be define by the test */
//extern void resource_init_simulation(xsim_node_t *node, xsim_iface_t **iface, int nb_nodes);
//extern void resource_end_simulation(xsim_node_t *node, xsim_iface_t **iface, int nb_nodes);
//
///* Must return END_OF_SIMULATION or CONTINUE_SIMULATION */
extern int  resource_handle_msg(xsim_node_t *node, xsim_msg_t *msg, xsim_iface_t *iface);
extern int  resource_simulation(xsim_node_t *node, xsim_msg_list_t **NIC_msg_list_output, 
		sim_time *next_time_for_resource);
//extern void resource_wake_up();
//
//
///* already define */
//extern int  son_body_model4(xsim_t *xsim, int node_id, sim_time limit_simulation_time, 
//        const char *output_measure);
//extern int  son_body_model1(xsim_t *xsim, int node_id, sim_time limit_simulation_time, 
//        const char *output_measure);
//extern int sc_son_body_model1(xsim_t *xsim, int node_id, sim_time limit_simulation_time, 
//        const char *output_measure);
//
///* To define, useful only with sc_son_body_model1 */
//extern int xsim_sc_node_main (int ncycles, xsim_node_t *node);


#ifdef __cplusplus
}
#endif 

#endif
