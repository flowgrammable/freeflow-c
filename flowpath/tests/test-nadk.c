
#include "dataplane.h"
#include "pipeline.h"
#include "port.h"
#include "port_nadk.h"

#include "dpmp/dpmp.h"

#include <stdio.h>
#include <signal.h>

typedef enum {TEST_REFLECT=0, TEST_XREFLECT=1,
              TEST_LOOPBACK=2, TEST_XLOOPBACK=3,
              TEST_DATAPLANE=4} test_type_t;
static inline void reflect_test(struct np_port*, struct np_port*);
static inline void xreflect_test(struct np_port*, struct np_port*);
static inline void loopback_test(struct np_port*, struct np_port*);
static inline void xloopback_test(struct np_port*, struct np_port*);
static inline void dataplane_test(struct np_dataplane*,
                                  struct np_port*, struct np_port*);

static volatile sig_atomic_t received_sigint;
/*===== sigproc =====*/
/**
handles SIGINT signal(When the app is stopped using Ctrl C the IPC is removed)
handles SIGTERM signal(when app process is nicely killed)
handles SIGHUP signal(When the parent telnet connection is broken)

@param	signum	[IN] The received signal.
*/
static void sigproc(int signum, siginfo_t *info, void *ptr)
{
  static volatile sig_atomic_t fatal_error_in_progress;

  /* Since this handler is established for more than one kind of signal,
  it might still get invoked recursively by delivery of some other kind
  of signal.  Use a static variable to keep track of that. */
  if (fatal_error_in_progress)
    raise (signum);
  fatal_error_in_progress = 1;

  printf("\nERR:SIGNAL(%d) is received, and the APP processing is"
      " going to stop\n", signum);

  received_sigint = 1;

  /*Since it is a process exit handler, for graceful exist from
    the current process, set the handler as default */
  if ((signum == SIGSEGV) || (signum == SIGILL) || (signum == SIGBUS)) {
    printf("\tNADK may not be cleaned up\n");
    signal(signum, SIG_IGN);
    if (raise(signum) != 0) {
      printf("Raising the signal %d", signum);
      return;
    }
  }
  printf("\nSignal Handler Finished\n");
}

static void catch_signal(int snum, struct sigaction act, struct sigaction tact)
{
  if (sigaction(snum, NULL, &tact) == -1) {
    printf("\nSignal registration failed\n");
    exit(EXIT_FAILURE);
  }
  if (tact.sa_handler != SIG_IGN) {
    if (sigaction(snum, &act, NULL) == -1) {
      printf("\nSignal registration failed\n");
      exit(EXIT_FAILURE);
    }
  }
}

/**
Installing signal handler to handle the Ctrl C and other kill handlers
Please note that this does not handle the SIGKILL or kill -9 and other
ungracefull crashes.
*/
static void install_signal_handler(void)
{
  struct sigaction action, tmpaction;
  /* Asynchronous signals that result in attempted graceful exit */
  /* Set up the structure to specify the new action. */
  printf("Installing signal handler...\n");
  sigemptyset(&action.sa_mask);
  action.sa_sigaction = &sigproc;
  action.sa_flags = SA_SIGINFO;
  catch_signal(SIGHUP, action, tmpaction);
  catch_signal(SIGINT, action, tmpaction);
  catch_signal(SIGQUIT, action, tmpaction);
  catch_signal(SIGTERM, action, tmpaction);
  catch_signal(SIGSEGV, action, tmpaction);
  catch_signal(SIGABRT, action, tmpaction);
}


/* Create a NADK port. */
struct np_port* 
make_port(short p)
{
//  np_error_t err = 0;
  struct np_device* dev = np_nadk_open(p);
  if (dev == NULL) {
//    fprintf(stderr, "error: %s\n", np_strerror(err));
    return NULL;
  }
  struct np_port* port = np_port_create(dev);
  return port;
}


/* Create a NADK port and add it ot the data plane. Returns the port. */
struct np_port*
add_port(struct np_dataplane* dp, short p)
{
  /* Make the port. */
  struct np_port* port = make_port(p);
  if (!port)
    return NULL;

  /* Add the port ot the data plane. */
  np_error_t err;
  np_dataplane_add_port(dp, port, &err);
  if (np_error(err)) {
    fprintf(stderr, "%s\n", np_strerror(err));
    return NULL;
  }

  return port;
}


int 
main(int argc, char** argv)
{
  if (argc != 5) {
    fprintf(stderr, "usage: ./test-nadk <name> <pipeline>\n\n");
    fprintf(stderr, " Arguments:\n");
    fprintf(stderr, "    name       the symbolic name of the dataplane\n");
    fprintf(stderr, "    pipeline   the absolute path to a pipeline module\n");
    fprintf(stderr, "    dprc       the name of the NADK resource container\n");
    fprintf(stderr, "    test_type  0: Reflect; 1: XRefelct; "
                    "2:Loopback, 3:XLoopback, 4: Dataplane\n");
    return -1;
  }

  char const* name = argv[1];
  char const* path = argv[2];
  char const* dprc = argv[3];
  const test_type_t test = atoi(argv[4]);

  /* Asynchronous signals that result in attempted graceful exit */
  install_signal_handler();

  np_error_t err;
  struct np_dataplane* dp = np_dataplane_create(name, path, &err);
  if (np_error(err)) {
    fprintf(stderr, "error: %s\n", dpmp_strerror(err));
    return -1;
  }

  /* Hack to initialize NADK framework with passed-in DPRC */
  np_nadk_init(dprc);

  /* Add ports to the dataplane. */
  struct np_port* port1 = add_port(dp, 0);
  if (!port1)
    return -1;
  struct np_port* port2 = add_port(dp, 1);
  if (!port2)
    return -1;

  /* Start the data plane. */
  bool ok = true;
  err = np_dataplane_start(dp);
  if (np_error(err)) {
    fprintf(stderr, "error: %s\n", np_strerror(err));
    ok = false;
  }

  /* Enter test loop until Ctrl-C (SIGINT) is received */
  printf("Entering packet I/O loop until Ctrl-C (SIGINT) is received\n");
  int i = 0;
  while(!received_sigint) {
    // choose which test to run by run-time parameter:
    switch(test) {
    case TEST_REFLECT:
      reflect_test(port1, port2);
      break;
    case TEST_XREFLECT:
      xreflect_test(port1, port2);
      break;
    case TEST_LOOPBACK:
      loopback_test(port1, port2);
      break;
    case TEST_XLOOPBACK:
      xloopback_test(port1, port2);
      break;
    case TEST_DATAPLANE:
      dataplane_test(dp, port1, port2);
      break;
    default:
      printf("Invalid test_type: %d...\n", test);
      received_sigint = 1;
      break;
    }
  }
  printf("Broke out of processing loop\n");
  
  /* Stop the data plane. */
  err = np_dataplane_stop(dp);
  if (np_error(err)) {
    fprintf(stderr, "error: %s\n", np_strerror(err));
    ok = false;
  }

  /* Clean up the data plane. */
  np_dataplane_delete(dp, &err);

  return ok ? 0 : -1;
}


/* Reflect test: all packets recieved on an interfaced are reflected back
 *  on the same interface. */
static inline void
reflect_test(struct np_port* port1, struct np_port* port2) {
  struct np_packet* pkt;
  pkt = np_port_recv_packet(port1);
  if (pkt) {
    np_port_send_packet(port1, pkt);
  }

  pkt = np_port_recv_packet(port2);
  if (pkt) {
    np_port_send_packet(port2, pkt);
  }
}


/* XReflect test: all packets recieved on an interfaced are reflected back
 *  on the other interface (like a wire).
 * - This will also potentially incur a higher penalty compared to the
 *   Reflect test due to swapping mbufs between ports.
 */
static inline void
xreflect_test(struct np_port* port1, struct np_port* port2) {
  struct np_packet* pkt;
  pkt = np_port_recv_packet(port1);
  if (pkt) {
    np_port_send_packet(port2, pkt);
  }

  pkt = np_port_recv_packet(port2);
  if (pkt) {
    np_port_send_packet(port1, pkt);
  }
}


/* Loopback test: all packets recieved on an interface are reflected back
 *  on the same interface.
 * - This does not use the noproto ring or packet structures and will be
 *   the best case senario.
 */
static inline void
loopback_test(struct np_port* port1, struct np_port* port2) {
  np_nadk_loopback(port1->device);
  np_nadk_loopback(port2->device);
}


/* XLoopback test: all packets recieved on an interface are reflected back
 *  on the other interface.
 * - This does not use the noproto ring or packet structures and will be
 *   the best case senario.
 * - This will also potentially incur a higher penalty compared to the
 *   Loopback test due to swapping mbufs between ports.
 */
static inline void
xloopback_test(struct np_port* port1, struct np_port* port2) {
  np_nadk_xloopback(port1->device, port2->device);
  np_nadk_xloopback(port2->device, port1->device);
}


/* Dataplane test: all packets recieved on an interface are reflected back
 *  on the same interface.
 * - This does not use the noproto ring or packet structures and will be
 *   the best case senario.
 */
static inline void
dataplane_test(struct np_dataplane* dp,
               struct np_port* port1, struct np_port* port2) {
  struct np_packet* pkt;
  // Process any packets from Port 1:
  struct np_arrival arrival1 = {port1->id, port1->id, 0};
  pkt = np_port_recv_packet(port1);
  if (pkt) {
    dp->pipeline->insert(dp, pkt, arrival1);
    // TODO: was pkt sucessfully deleted?
  }

  // Process any packets from Port 2:
  struct np_arrival arrival2 = {port2->id, port2->id, 0};
  pkt = np_port_recv_packet(port2);
  if (pkt) {
    dp->pipeline->insert(dp, pkt, arrival2);
    // TODO: was pkt sucessfully deleted?
  }
}

