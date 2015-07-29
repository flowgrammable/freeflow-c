// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "packet.h"
#include "util.h"

/* Allocate a packet from the underlying buffer. */
struct fp_packet*
fp_packet_create(unsigned char* data, int size, uint64_t timestamp,
                 void* buf_handle, fp_buf_t buf_dev)
{
  struct fp_packet* packet = fp_allocate(struct fp_packet);
  packet->data = data;
  packet->size = size;
  packet->timestamp = timestamp;
  packet->buf_handle = buf_handle;
  packet->buf_dev = buf_dev;
  return packet;
}


/* Deallocate the packet. Note that this does not
   free the underlying packet data. That is managed
   by the device on which the packet is received. */
void 
fp_packet_delete(struct fp_packet* packet)
{
  fp_deallocate(packet);
}


struct fp_context* 
fp_context_create(struct fp_packet* pkt, struct fp_arrival arr)
{
  struct fp_context* cxt = fp_allocate(struct fp_context);
  cxt->in_port = arr.in_port;
  cxt->in_phy_port = arr.in_phy_port;
  cxt->tunnel_id = arr.tunnel_id;
  cxt->packet = pkt;
  return cxt;
}


void 
fp_context_delete(struct fp_context* cxt)
{
  fp_deallocate(cxt);
}
