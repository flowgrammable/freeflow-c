// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "port.h"
#include "port_udp.h"
#include "packet.h"


/* This counter is used to allocate virtual port numbers.
   Note that the initial virtual port number is one larger
   than the number of physical ports on the system. */
fp_port_id_t port_alloc;


/* Allocate a new port id. 

   FIXME: In a long-running system, this will eventually
   run out of identifiers. Use a free-id list to reclaim
   released identifiers. */
inline static fp_port_id_t
allocate_port_id()
{
  return ++port_alloc;
}


/* Release a previously allocated port id.

   FIXME: Implement me. */
inline static void
release_port_id(fp_port_id_t id)
{
}


/* Create a port over the given device. */
struct fp_port*
fp_port_create(struct fp_device* dev)
{
  struct fp_port* port = fp_allocate(struct fp_port);
  port->id = allocate_port_id(); 
  port->device = dev;
  return port;
}


/* Close the port and reclaim its resources. */
void
fp_port_delete(struct fp_port* port)
{
  release_port_id(port->id);
  port->device->vtbl->close(port->device);
  fp_deallocate(port);
}


/* Send the packet context to the port.

   TODO: If the port is blocked or not forwarding, do not send. 
   This should really be part of fp_port_send. */
int
fp_port_output(struct fp_port* port, struct fp_context* cxt)
{
  return fp_port_send_packet(port, cxt->packet);
}

