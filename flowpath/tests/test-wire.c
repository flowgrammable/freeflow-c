
#include "dataplane.h"
#include "pipeline.h"
#include "port.h"
#include "port_udp.h"

#include "proto.h"
#include "error.h"

#include <stdio.h>


/* Create a UDP port. */
struct fp_port*
make_port(short p)
{
  fp_error_t err = 0;
  struct fp_device* dev = fp_udp_open_port(p, &err);
  if (fp_error(err)) {
    fprintf(stderr, "error: %s\n", fp_strerror(err));
    return NULL;
  }
  struct fp_port* port = fp_port_create(dev);
  return port;
}


/* Create a UDP port and add it ot the data plane. Returns
   the port. */
struct fp_port*
add_port(struct fp_dataplane* dp, short p)
{
  /* Make the port. */
  struct fp_port* port = make_port(p);
  if (!port)
    return NULL;

  /* Add the port ot the data plane. */
  fp_error_t err;
  fp_dataplane_add_port(dp, port, &err);
  if (fp_error(err)) {
    fprintf(stderr, "%s\n", fp_strerror(err));
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

  fp_error_t err;
  struct fp_dataplane* dp = fp_dataplane_create(name, path, &err);
  if (fp_error(err)) {
    fprintf(stderr, "error: %s\n", fp_strerror(err));
    return -1;
  }


  /* Create the ports. */
  struct fp_port* port1 = add_port(dp, 5000);
  if (!port1)
    return -1;
  struct fp_port* port2 = add_port(dp, 5001);
  if (!port2)
    return -1;

  /* Start the data plane. */
  bool ok = true;
  err = fp_dataplane_start(dp);
  if (fp_error(err)) {
    fprintf(stderr, "error: %s\n", fp_strerror(err));
    ok = false;
  }
  
  /* Stop the data plane. */
  err = fp_dataplane_stop(dp);
  if (fp_error(err)) {
    fprintf(stderr, "error: %s\n", fp_strerror(err));
    ok = false;
  }

  /* Clean up the data plane. */
  fp_dataplane_delete(dp, &err);

  return ok ? 0 : -1;
}
