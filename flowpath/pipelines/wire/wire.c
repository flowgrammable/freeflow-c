
#include "dp/dataplane.h"
#include "dp/pipeline.h"
#include "dp/port.h"
#include "dp/packet.h"

#include "dpmp/dpmp.h"

#include <stdio.h>


/* The wrie pipeline is a simple 2-port configuraiton. */
struct wire
{
  struct fp_port* ports[2];
};


/* Return a pointer to the wire objet for the pipeline. */
static inline struct wire*
get_wire(struct fp_dataplane* dp)
{
  return (struct wire*)dp->pipeline->pipeline_object;
}


/* Set the output port id for the packet context. Packets
   received on the first port will be sent through to the 
   second and packets received on the second port will be
   sent through the first.

   If the wire is not fully configured (i.e., one or both
   ports are unset), the packet is not forwarded. */
static fp_error_t
wire_route(struct wire* w, struct fp_context* cxt)
{
  if (!w->ports[0] || !w->ports[1]) {
    cxt->out_port = FP_PORT_DROP; // something is wrong, with setup...
    return FP_ERROR;
  }

  // Forward the packet.
  if (cxt->in_port == w->ports[0]->id)
    cxt->out_port = w->ports[1]->id;
  else
    cxt->out_port = w->ports[0]->id;
  return FP_OK;
}


/* Establish context for the packet, and initialize
   it's out port. */
static struct fp_context*
wire_ingress(struct fp_dataplane* dp, 
             struct fp_packet* pkt, 
             struct fp_arrival arr)
{
//  fprintf(stderr, "[wire] ifput from port(arrival struct) %d\n", arr.in_port);
  struct fp_context* cxt = fp_context_create(pkt, arr);
  return cxt;
}


/* Queue the packet on its output port and release
   the context. */
static void
wire_egress(struct fp_dataplane* dp, struct fp_context* cxt)
{
//  fprintf(stderr, "[wire] output to port %d\n", cxt->out_port);
  struct fp_port* port = fp_dataplane_get_port(dp, cxt->out_port);
  fp_port_output(port, cxt);
  fp_context_delete(cxt);
}


/* Called to configure the pipeline before it processes any packets. 
   This allocates the wire data, allowing the data plane to be
   configured. */
static fp_error_t
wire_load(struct fp_dataplane* dp)
{
  fprintf(stderr, "[flowpath] loading 'wire'\n");
  struct wire* w = fp_allocate(struct wire);
  w->ports[0] = w->ports[1] = NULL;
  dp->pipeline->pipeline_object = w;
  fprintf(stderr, "[flowpath] loaded 'wire'\n");
  return FP_OK;
}


/* Called prior to unloading a pipeline. This is responsible for 
   deallocting resources that are acquired by load. Note that this must 
   not deallocate the pipeline instance. 

   TODO: Should this module be responsible for deallocating the
   pipeline instance? */
static fp_error_t
wire_unload(struct fp_dataplane* dp)
{
  fprintf(stderr, "[flowpath] unloading 'wire'\n");
  fp_deallocate(dp->pipeline->pipeline_object);
  fprintf(stderr, "[flowpath] unloaded 'wire'\n");
  return FP_OK;
}


/* Add the port p to the wire. If the wire already has two
   ports, the port is not added and the return value indicates
   an error. */
static fp_error_t
wire_add_port(struct fp_dataplane* dp, struct fp_port* p)
{
  struct wire* w = get_wire(dp);
  if (!w->ports[0])
    w->ports[0] = p;
  else if (!w->ports[1])
    w->ports[1] = p;
  else
    return FP_ERROR; /* FIXME: Find a better error code. */
  return FP_OK;
}


/* Remove the given wire from the port. If the port is not
   attached to the wire, this returns an error. */
static fp_error_t
wire_del_port(struct fp_dataplane* dp, struct fp_port* p)
{
  struct wire* w = get_wire(dp);
  if (w->ports[0] == p)
    w->ports[0] = NULL;
  else if (w->ports[1] == p)
    w->ports[1] = NULL;
  else
    return FP_ERROR; /* FIXME: Find a better error code. */
  return FP_OK;
}


/* Make the wire active. In order for wire to be active,
   it must have two end points. */
static fp_error_t
wire_start(struct fp_dataplane* dp)
{
  /* FIXME: Be more precise with the error result. */
  struct wire* w = get_wire(dp);
  if (!w->ports[0] || !w->ports[1])
    return FP_ERROR;

  /* TODO: Anything else? */
  return FP_OK;
}


/* Make the wire inactive. When not active, the wire is
   not processing */
static fp_error_t
wire_stop(struct fp_dataplane* dp)
{
  /* TODO: Is there anything we actually have to do here? */
  return FP_OK;
}



/* Insert a packet into the pipeline. */
static void 
wire_insert(struct fp_dataplane* dp,
            struct fp_packet* pkt, struct fp_arrival arr)
{
  struct wire* w = get_wire(dp);
  struct fp_context* cxt = wire_ingress(dp, pkt, arr);
  wire_route(w, cxt);  // currently ignorning return message...
  wire_egress(dp, cxt);
}


/* Initialize the pipeline with the processing entry points. */
fp_error_t
pipeline_init(struct fp_pipeline* p)
{
  p->load   = wire_load;
  p->unload = wire_unload;
  p->add_port = wire_add_port;
  p->del_port = wire_del_port;
  p->start  = wire_start;
  p->stop   = wire_stop;
  p->insert = wire_insert;
  return FP_OK;
}
