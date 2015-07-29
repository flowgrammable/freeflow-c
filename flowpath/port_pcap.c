// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "port_pcap.h"

/* Create a capture device for the given trace. */
struct fp_port* 
fp_capture_open(const char* file, int pnum)
{
  /* Build the device. */
  struct fp_capture_device* device = fp_allocate(struct fp_capture_device);
  device->handle = pcap_open_offline(file, err_buf);
  if (!device->handle) {
    fprintf(stderr, "error: %s", err_buf);
    fp_deallocate(device);
    return NULL;
  }

  /* Build the port. */
  struct fp_port* port = fp_allocate(struct fp_port);
  port->id = pnum;
  port->device = device;
  port->input = fp_capture_read;
  return port;
}

/* Close and destroy the pcap port. The object pointed to by port
   is no longer valid after this operation. */
void 
fp_capture_close(struct fp_port* port)
{
  struct fp_capture_device* device = (struct fp_capture_device*)port->device;
  pcap_close(device->handle);
  fp_deallocate(device);
  fp_deallocate(port);
}

/* Read the next packet from the pcap file. */
struct fp_packet* 
fp_capture_read(void* device)
{
  unsigned char const* src;
  unsigned char* dst;
  struct fp_capture_device* pcap;
  struct fp_packet* packet;
  
  pcap = (struct fp_capture_device*)device;
  src = pcap_next(pcap->handle, &pcap->header);
  if (!src)
    return NULL;

  /* Copy the buffer and create a new packet for it. */
  dst = (unsigned char*)malloc(pcap->header.len);
  memcpy(dst, src, pcap->header.len);

  /* Allocate a noproto packet. */
  packet = fp_make_packet(dst, pcap->header.len);
  return packet;
}
