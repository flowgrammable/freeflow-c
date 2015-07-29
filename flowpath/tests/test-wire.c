
#include "dataplane.h"
#include "pipeline.h"
#include "port.h"
#include "port_udp.h"

#include "dpmp/dpmp.h"

#include <stdio.h>


/* Create a UDP port. */
struct np_port* 
make_port(short p)
{
  np_error_t err = 0;
  struct np_device* dev = np_udp_open_port(p, &err);
  if (np_error(err)) {
    fprintf(stderr, "error: %s\n", np_strerror(err));
    return NULL;
  }
  struct np_port* port = np_port_create(dev);
  return port;
}


/* Create a UDP port and add it ot the data plane. Returns
   the port. */
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
  if (argc != 3) {
    fprintf(stderr, "usage: ./test-wire <name> <pipeline>\n\n");
    fprintf(stderr, " Arguments:\n");
    fprintf(stderr, "    name       the symbolic name of the dataplane\n");
    fprintf(stderr, "    pipeline   the absolute path to a pipeline module\n");
    return -1;
  }

  char const* name = argv[1];
  char const* path = argv[2];

  np_error_t err;
  struct np_dataplane* dp = np_dataplane_create(name, path, &err);
  if (np_error(err)) {
    fprintf(stderr, "error: %s\n", dpmp_strerror(err));
    return -1;
  }


  /* Create the ports. */
  struct np_port* port1 = add_port(dp, 5000);
  if (!port1)
    return -1;
  struct np_port* port2 = add_port(dp, 5001);
  if (!port2)
    return -1;

  /* Start the data plane. */
  bool ok = true;
  err = np_dataplane_start(dp);
  if (np_error(err)) {
    fprintf(stderr, "error: %s\n", np_strerror(err));
    ok = false;
  }
  
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
