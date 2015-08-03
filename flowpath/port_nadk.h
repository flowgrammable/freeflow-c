// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PORT_NADK_H
#define FLOWPATH_PORT_NADK_H

/* This module abstracts a NADK network device (or port) */

#include "util.h"
#include "port.h"


struct fp_packet;
struct fp_port;

/* forward declaration of nadk structures */
struct nadk_dev;
struct nadk_mbuf;


/* A NADK network device */
struct fp_nadk_device
{
  struct fp_device base;    /* Base class sub-object. */
  struct nadk_dev* handle;  /* NADK device handle. */
  struct fp_ring*  rx_ring; /* Local RX ring for batch receive */
  struct fp_ring*  tx_ring; /* Local TX ring for batch receive */
  short            dprc_id; /* NADK dprc device number */
};


struct fp_device* fp_nadk_open(short);
void              fp_nadk_close(struct fp_device*);
struct fp_packet* fp_nadk_recv(struct fp_device*);
int               fp_nadk_send(struct fp_device*, struct fp_packet*);
void              fp_nadk_drop(struct fp_device*, struct fp_packet*);


/* nadk routines temporarily exposed */
struct fp_packet* fp_nadk_recv_single(struct fp_device*);
void fp_nadk_cleanup();
struct fp_nadk_resources* fp_nadk_init(const char*);
int fp_nadk_loopback(struct fp_device*);
int fp_nadk_xloopback(struct fp_device*, struct fp_device*);

#endif
