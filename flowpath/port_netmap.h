// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PORT_NETMAP_H
#define FLOWPATH_PORT_NETMAP_H

/* This module abstracts a Netmap network device (or port) */

#include "util.h"
#include "port.h"

#include <net/netmap_user.h>

struct fp_packet;
struct fp_port;
struct nm_pkthdr;
struct nm_desc;


/* A Netmap network device */
struct fp_netmap_device 
{
  struct fp_device base;   /* Base class sub-object. */
  struct nm_desc*  handle; /* Netmap handle. */
  struct nm_pkthdr header; /* same as pcap_pkthdr */
};


struct fp_device* fp_netmap_open(const char*);
void              fp_netmap_close(struct fp_device*);
struct fp_packet* fp_netmap_recv(struct fp_device*);
int               fp_netmap_send(struct fp_device*, struct fp_packet*);
void              fp_netmap_drop(struct fp_device*, struct fp_packet*);

#endif
