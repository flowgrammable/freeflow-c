// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PORT_UDP_H
#define FLOWPATH_PORT_UDP_H

/* This module abstracts a Netmap network device (or port)
   for captured network data.

   TOOD: Allow ports to be output ports. */

#include "util.h"
#include "port.h"
#include "error.h"

#include <sys/socket.h>
#include <netinet/in.h>


struct fp_packet;
struct fp_port;
struct nm_pkthdr;
struct nm_desc;


/* A UDP socket that can act as a logical port. There is no
   tunnel id associated with this port.

   Note that because UDP sockets are connectionless, data 
   sent through this device may not be delivered.

   TODO: Support IPv6 IP ports. */
struct fp_udp_device
{
  struct fp_device   base; /* Base class sub-object. */
  struct sockaddr_in addr; /* The configured socket adress. */
  int                fd;   /* The underlying socket. */
};


struct fp_device* fp_udp_open(const char*, fp_error_t*);
struct fp_device* fp_udp_open_port(short, fp_error_t*);
struct fp_device* fp_udp_open_sockaddr(struct sockaddr_in*, fp_error_t*);
void              fp_udp_close(struct fp_device*);
struct fp_packet* fp_udp_recv(struct fp_device*);
int               fp_udp_send(struct fp_device*, struct fp_packet*);
void              fp_udp_drop(struct fp_device*, struct fp_packet*);

#endif
