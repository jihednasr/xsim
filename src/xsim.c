/* __HEADER_HERE__ */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <xsim.h>
#include <xsim_error.h>
#include <xsim_internals.h>
#include <xsim_topology.h>

#define DBG_HDR "xsim"
#ifdef XSIM_MAIN_DEBUG
#define DEBUG
#endif /* XSIM_MAIN_DEBUG */

#ifdef XSIM_MAIN_HDEBUG
#define HUGE_DEBUG
#endif /* XSIM_MAIN_HDEBUG */

#include <xsim_debug.h>

xsim_internals_t xsim_int;

xsim_t *
xsim_alloc(uint8_t nb_x, uint16_t nb_nodes, const char *topology) {
  /*
   * Create central structure
   */
  xsim_t *res = malloc(sizeof(xsim_t));

  res->nb_x     = nb_x;
  res->nb_nodes = nb_nodes;

  if (topology != NULL) {
      res->topology = malloc(sizeof(char) * (strlen(topology)+1));
      strcpy(res->topology, topology);
  } else {
      res->topology = NULL;
  }

  /*
   * Setup the topology
   */
  xsim_topology_init(res);

  return res;
}

int
xsim_init(xsim_t *xsim __attribute__((__unused__))) {

  int shm_id = 0;
  int cause  = 0;
  xsim_msg_box_t *box;

  /*
   * Create shared memory
   */
  shm_id = shmget(IPC_PRIVATE, xsim_msg_box_size(), IPC_CREAT | 0640);

  if (shm_id == -1) {
    EMSG("shmget: shmget failed");
    cause = XSIM_SHM_ERROR;
    goto get_error;
  }
  xsim_int.shm_id = shm_id;
  xsim->shm_id    = shm_id;

  /*
   * Attach shared memory
   */
  box = shmat(shm_id, (void *)0, 0);
  if (box == (void *)-1) {
    EMSG("shmat: shmat failed");
    cause = XSIM_SHM_ERROR;
    goto at_error;
  }
  DMSG("shmat: FATHER mapped at %p\n", box);
  xsim_int.box   = box;
  xsim->shm_addr = box;

  /*
   * Do initilization of structure
   */
  xsim_msg_box_init(box, xsim->nb_nodes); 

#if 0
  if (shmdt(xsim_int.box) == -1) {
    EMSG("shmdt failed\n");
  }
#endif

  return XSIM_SUCCESS;

 at_error:
  shmctl(shm_id, IPC_RMID, NULL);

 get_error:

  return cause;
}


int
xsim_fini(xsim_t *xsim __attribute__((__unused__))) {

  xsim_msg_box_fini(xsim_int.box);

  if (shmdt(xsim_int.box) == -1) {
    fprintf(stderr, "shmdt failed\n");
  }

  if ((shmctl(xsim_int.shm_id, IPC_RMID, NULL)) == -1) {
    fprintf(stderr, "shmctl: shmctl failed");
  }
  
  return XSIM_SUCCESS;
}


int
xsim_free(xsim_t *xsim) {
  xsim_topology_free(xsim); 
  if (xsim->topology != NULL) {
    free(xsim->topology);
    xsim->topology = NULL;
  }
  free(xsim);
  return XSIM_SUCCESS;
}
