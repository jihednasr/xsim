#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#include <xsim.h>
#include <xsim_node.h>
#include <xsim_iface.h>
#include <xsim_targets.h>

#define NB_NODES   10 
#define NB_X        1

#define MESS	"Hello world!"

int broadcast(int node_id, xsim_iface_t *iface) 
{
	xsim_msg_t *msg = xsim_msg_new();
	for (int i=0 ; i< NB_NODES ; i++) {
		xsim_targets_set(&(msg->targets), i);
	}
	strcpy((char*)(&(msg->payload)), MESS);
	fprintf(stderr, "[%d] I will now send a message !!!\n", node_id);
	xsim_iface_send(iface, msg);

	return 0;
}

int
son_body(xsim_t *xsim, int node_id){

  xsim_node_t    *node  = NULL;
  xsim_iface_t   *iface = NULL; 
  xsim_msg_t     *msg   = NULL;

  node  = xsim_node_init(xsim, node_id);
  iface = xsim_iface_new(node, 0);


  broadcast(node_id, iface);

  /*
   * Body here
   */

  int nb_rec = 0;
  while(nb_rec < NB_NODES) {
	while(!(msg = xsim_iface_recv(iface))) ;
	if (strcmp((char*)(msg->payload), MESS)) {
	      	fprintf(stderr, "[%d] Error for the payload received !!!\n", node_id);
	}	
	nb_rec++;
	xsim_msg_free(msg);
  }
		  

  xsim_iface_free(iface);
  xsim_node_free(node);

  return 0;
}

int
main(int argc, char **argv) {

  xsim_t *xsim       = NULL;
  int     pid        = 0;
  int     status     = 0;
  int     spawned    = 0;
  int     terminated = 0;
  int     ret        = 0;

  if(argc != 1) {
    fprintf(stderr, "Usage: %s\n", argv[0]);
  }

  /*
   * Create the structure
   */
  xsim = xsim_alloc(NB_X, NB_NODES, NULL);

  /*
   * Setup comm
   */
  xsim_init(xsim);

  /*
   * Body here
   */
  while( spawned < NB_NODES) {
    pid = fork();
    switch(pid){
    case -1: /* error */
      goto on_fork_error;
      
    case 0:  /* son */
      fprintf(stderr, "Child %d born\n", spawned);
      son_body(xsim, spawned);
      goto child_end;
      
    default: /* father */
      spawned++;
      break;
    }
  }

  do{
    ret = wait(&status);
    
    if( WEXITSTATUS(status) )
      fprintf(stderr, "Got an error\n");
    
    terminated++;
  }while(ret > 0); /* no problem: ID 0 is init :-) */
  terminated--; /* one for the last loop */

  if( (errno == ECHILD) && (terminated == spawned) ) {
    fprintf(stderr, "Got all my childs\n");
  } else {
    fprintf(stderr, "Something went wrong : handle it (%d / %d)\n", terminated, spawned);
  }
  xsim_fini(xsim);  

 child_end:
  xsim_free(xsim);  
  return EXIT_SUCCESS;

  /*
   * Error handling
   */
 on_fork_error:
  /* send a signal here */
  while( terminated < spawned ){
    wait(&status);
    
    if( WEXITSTATUS(status) )
      fprintf(stderr, "Got an error\n");
    
    terminated++;
  }
  xsim_fini(xsim);  
  xsim_free(xsim);  
  return EXIT_FAILURE;


}
