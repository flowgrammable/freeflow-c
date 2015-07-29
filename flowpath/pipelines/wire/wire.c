
#include "dp/dataplane.h"
#include "dp/pipeline.h"
#include "dp/port.h"
#include "dp/packet.h"

#include "dpmp/dpmp.h"

#include <stdio.h>


/* The wrie pipeline is a simple 2-port configuraiton. */
struct wire
{
  struct np_port* ports[2];
};


/* Return a pointer to the wire objet for the pipeline. */
static inline struct wire*
get_wire(struct np_dataplane* dp)
{
  return (struct wire*)dp->pipeline->pipeline_object;
}


/* Set the output port id for the packet context. Packets
   received on the first port will be sent through to the 
   second and packets received on the second port will be
   sent through the first.

   If the wire is not fully configured (i.e., one or both
   ports are unset), the packet is not forwarded. */
static np_error_t
wire_route(struct wire* w, struct np_context* cxt)
{
  if (!w->ports[0] || !w->ports[1]) {
    cxt->out_port = NP_PORT_DROP; // something is wrong, with setup...
    return NP_ERROR;
  }

  // Forward the packet.
  if (cxt->in_port == w->ports[0]->id)
    cxt->out_port = w->ports[1]->id;
  else
    cxt->out_port = w->ports[0]->id;
  return NP_OK;
}


/* Establish context for the packet, and initialize
   it's out port. */
static struct np_context*
wire_ingress(struct np_dataplane* dp, 
             struct np_packet* pkt, 
             struct np_arrival arr)
{
//  fprintf(stderr, "[wire] input from port(arrival struct) %d\n", arr.in_port);
  struct np_context* cxt = np_context_create(pkt, arr);
  return cxt;
}


/* Queue the packet on its output port and release
   the context. */
static void
wire_egress(struct np_dataplane* dp, struct np_context* cxt)
{
//  fprintf(stderr, "[wire] output to port %d\n", cxt->out_port);
  struct np_port* port = np_dataplane_get_port(dp, cxt->out_port);
  np_port_output(port, cxt);
  np_context_delete(cxt);
}


/* Called to configure the pipeline before it processes any packets. 
   This allocates the wire data, allowing the data plane to be
   configured. */
static np_error_t
wire_load(struct np_dataplane* dp)
{
  fprintf(stderr, "[noproto] loading 'wire'\n");
  struct wire* w = np_allocate(struct wire);
  w->ports[0] = w->ports[1] = NULL;
  dp->pipeline->pipeline_object = w;
  fprintf(stderr, "[noproto] loaded 'wire'\n");
  return NP_OK;
}


/* Called prior to unloading a pipeline. This is responsible for 
   deallocting resources that are acquired by load. Note that this must 
   not deallocate the pipeline instance. 

   TODO: Should this module be responsible for deallocating the
   pipeline instance? */
static np_error_t
wire_unload(struct np_dataplane* dp)
{
  fprintf(stderr, "[noproto] unloading 'wire'\n");
  np_deallocate(dp->pipeline->pipeline_object);
  fprintf(stderr, "[noproto] unloaded 'wire'\n");
  return NP_OK;
}


/* Add the port p to the wire. If the wire already has two
   ports, the port is not added and the return value indicates
   an error. */
static np_error_t
wire_add_port(struct np_dataplane* dp, struct np_port* p)
{
  struct wire* w = get_wire(dp);
  if (!w->ports[0])
    w->ports[0] = p;
  else if (!w->ports[1])
    w->ports[1] = p;
  else
    return NP_ERROR; /* FIXME: Find a better error code. */
  return NP_OK;
}


/* Remove the given wire from the port. If the port is not
   attached to the wire, this returns an error. */
static np_error_t
wire_del_port(struct np_dataplane* dp, struct np_port* p)
{
  struct wire* w = get_wire(dp);
  if (w->ports[0] == p)
    w->ports[0] = NULL;
  else if (w->ports[1] == p)
    w->ports[1] = NULL;
  else
    return NP_ERROR; /* FIXME: Find a better error code. */
  return NP_OK;
}


/* Make the wire active. In order for wire to be active,
   it must have two end points. */
static np_error_t
wire_start(struct np_dataplane* dp)
{
  /* FIXME: Be more precise with the error result. */
  struct wire* w = get_wire(dp);
  if (!w->ports[0] || !w->ports[1])
    return NP_ERROR;

  /* TODO: Anything else? */
  return NP_OK;
}


/* Make the wire inactive. When not active, the wire is
   not processing */
static np_error_t
wire_stop(struct np_dataplane* dp)
{
  /* TODO: Is there anything we actually have to do here? */
  return NP_OK;
}



/* Insert a packet into the pipeline. */
static void 
wire_insert(struct np_dataplane* dp,
            struct np_packet* pkt, struct np_arrival arr)
{
  struct wire* w = get_wire(dp);
  struct np_context* cxt = wire_ingress(dp, pkt, arr);
  wire_route(w, cxt);  // currently ignorning return message...
  wire_egress(dp, cxt);
}


/* Initialize the pipeline with the processing entry points. */
np_error_t
pipeline_init(struct np_pipeline* p)
{
  p->load   = wire_load;
  p->unload = wire_unload;
  p->add_port = wire_add_port;
  p->del_port = wire_del_port;
  p->start  = wire_start;
  p->stop   = wire_stop;
  p->insert = wire_insert;
  return NP_OK;
}
