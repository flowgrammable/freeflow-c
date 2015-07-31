// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "port_netmap.h"
#include "packet.h"
#include "port.h"


/* The virtual table for all Netmap devices. */
static struct fp_device_vtbl netmap_vtbl = {
  .recv  = fp_netmap_recv,
  .send  = fp_netmap_send,
  .drop  = fp_netmap_drop,
  .close = fp_netmap_close
};


/* Attempt to open port in netmap mode */
struct fp_device*
fp_netmap_open(const char* name)
{
  /* Build the device. */
  struct fp_netmap_device* dev = fp_allocate(struct fp_netmap_device);
  dev->base.vtbl = &netmap_vtbl;

  dev->handle = nm_open(name, NULL, 0, NULL);
  if (dev->handle == NULL) {
    fprintf(stderr, "error: unable to open netmap device %s\n", name);
    fp_deallocate(dev);
    return NULL;
  }

  return (struct fp_device*)dev;
}


/* Close and destroy the Netmap port. The object pointed to by port
   is no longer valid after this operation. */
void
fp_netmap_close(struct fp_device* device)
{
  struct fp_netmap_device* dev = (struct fp_netmap_device*)device;
  nm_close(dev->handle);
  fp_deallocate(dev);
}


/* Read from a Netmap device, creating a new packet.
 
   TODO: avoid extra copy by holding netmap buffer.  Also 
   investigate zero copy. */
struct fp_packet*
fp_netmap_recv(struct fp_device* device)
{
  struct fp_netmap_device* dev = (struct fp_netmap_device*)device;
  unsigned char const* src;
  unsigned char* dst;
  struct fp_packet* packet;

  src = nm_nextpkt(dev->handle, &dev->header);
  if (src == NULL)
    return NULL; /* nothing waiting */

  /* Copy the buffer and create a new packet for it. */
  dst = (unsigned char*)malloc(dev->header.len);
  memcpy(dst, src, dev->header.len);

  /* Allocate a flowpath packet. */
  /* TODO: replace dev_handle(NULL) with netmap buffer */
  /* TODO: retrieve timestamp from ring */
  packet = fp_packet_create(dst, dev->header.len, 0
                            NULL, FP_BUF_NETMAP);
  return packet;
}


/* Write to a Netmap device.

   TODO: avoid extra copy by swapping netmap buffer.  Investigate
   zero copy. */
int
fp_netmap_send(struct fp_device* device, struct fp_packet* pkt)
{
  struct fp_netmap_device* dev = (struct fp_netmap_device*)device;
  int bytes = nm_inject(dev->handle, pkt->data, pkt->size);
  fp_deallocate(pkt->data);
  fp_packet_delete(pkt);
  return bytes;
}


/* Drop a packet from a netmap interface. */
void
fp_netmap_drop(struct fp_device* device, struct fp_packet* pkt)
{
  fp_deallocate(pkt->data);
  fp_packet_delete(pkt);
}



#if 0
/* Mimics netmap's nm_nextpkt function.  This version does not
 * release buffer in order to avoid extra copies.  This has the
 * disadvantage of potentially holding up the ring if service times
 * are not equal.
 *
 * TODO: explore swapping buffers with a tx and rx ring with a 3rd buffer.
 *       This may prevent packets with exceptionally long service time from
 *       holding up the entire ring...
 */
static u_char*
fp_netmap_nextpkt(struct nm_desc* d, struct nm_pkthdr* hdr) {
  int ring_i = d->cur_rx_ring;

  do {
    /* compute current ring to use */
    struct netmap_ring* ring = NETMAP_RXRING(d->nifp, ring_i);
    if (!nm_ring_empty(ring)) {
      u_int buf_i = ring->cur;
      u_int buf_idx = ring->slot[buf_i].buf_idx;
      u_char* buf_ptr = (u_char*)NETMAP_BUF(ring, buf_idx);

      // __builtin_prefetch(buf);
      hdr->ts = ring->ts;
      hdr->len = hdr->caplen = ring->slot[buf_i].len;
      ring->cur = nm_ring_next(ring, buf_i);
      /* we could postpone advancing head if we want
       * to hold the buffer. This can be supported in
       * the future.
       */
//      ring->head = ring->cur;
      d->cur_rx_ring = ring_i;
      return buf_ptr;
    }
    ring_i++;
    if (ring_i > d->last_rx_ring)
      ring_i = d->first_rx_ring;
  } while (ring_i != d->cur_rx_ring);

  return NULL; /* nothing found */
}
#endif
