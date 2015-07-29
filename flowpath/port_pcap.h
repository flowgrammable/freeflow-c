// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PORT_PCAP_H
#define FLOWPATH_PORT_PCAP_H

#include <pcap.h>

#include "packet.h"
#include "port.h"
#include "util.h"

/* This module implements a virtual network device (or port)
   for captured network data.

   TOOD: Allow capture ports to be output ports.

   TODO: pcap also operates live. Extend this facility to
   allow the instantiation of data planes on top a live
   network connection. */

/* Receives error messages from pcap. */
char err_buf[PCAP_ERRBUF_SIZE];

//struct fp_port;
//struct fp_packet;

/* A virtual network device that reads from a pcap file. */
struct fp_capture_device
{
  pcap_t*            handle;
  struct pcap_pkthdr header;
};

struct fp_port*   fp_capture_open(const char* file, int pnum);
void              fp_capture_close(struct fp_port* port);
struct fp_packet* fp_capture_read(void* device);

#endif
