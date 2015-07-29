// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PORT_H
#define FLOWPATH_PORT_H

/* The port module defines the port abstraction and its
   associated properties and operations. A port typically
   corresponds to a kernel net device. In user space,
   a port can be constructed to represent virtually any
   kind of device (e.g., a pcap file). In other words, a
   noproto port is simply an abtstraction that provides
   some model of the underlying port object.

   Note that ports are not inherently responsible for sending
   or receiving data. The functions fp_packet_arrive() and 
   fp_packet_depart() are used to move packets between data 
   plane and the device the port is modeling. 


   TODO: Define data types for the special virtual ports. 
   below. */

#include "util.h"
#include "types.h"
#include "hash.h"


struct fp_port;
struct fp_context;


/* TODO: factor out the properties of physical ports. 
   For example, a wired eth port has different physical
   properties than a wifi port. 

   TODO: Make the port state a bitfield? */

#define FP_MAC_ADDR_LEN 48
#define FP_MAX_NAME_LEN 256

/* Reserved ports. */
#define FP_PORT_MAX_ID     0xffffff00
#define FP_PORT_DROP       0xfffffff7
#define FP_PORT_LOCAL      0xfffffff8
#define FP_PORT_IN_PORT    0xfffffff9
#define FP_PORT_NORMAL     0xfffffffa
#define FP_PORT_FLOOD      0xfffffffb
#define FP_PORT_ALL        0xfffffffc
#define FP_PORT_CONTROLLER 0xfffffffd
#define FP_PORT_TABLE      0xfffffffe
#define FP_PORT_ANY        0xffffffff


struct fp_device;
struct fp_port;


/* Types of function pointers for ports. */
typedef struct fp_packet* (*fp_device_recv_fn)(struct fp_device*);
typedef int (*fp_device_send_fn)(struct fp_device*, struct fp_packet*);
typedef void (*fp_device_drop_fn)(struct fp_device*, struct fp_packet*);
typedef void (*fp_device_close_fn)(struct fp_device*);


/* The type of the virtual table for device types. 

  - recv -- A pointer to a function that constructs a packet
    from the underlying device. The function has the following
    signature:

        struct fp_packet* recv(void* device);

    The device argument is a pointer to the underlying
    deviece. 
  
  - send -- Called to push a packet to an output source.
    number of bytes written. 

  - drop -- Called when a packet will be dropped from the
    dataplane. This is responsible for releasing any resources
    associated with the packet.

  - close -- Reclaim resources and dispose of the port
    object. */
struct fp_device_vtbl
{
  fp_device_recv_fn recv;
  fp_device_send_fn send;
  fp_device_drop_fn drop;
  fp_device_close_fn close;
};


/* The "base" class of all device structures. The device
   structure contains the virtual function table for all
   derived device types. 

   This must always be the first sub-object of a device. */
struct fp_device
{
  struct fp_device_vtbl* vtbl;
};


/* A port logical or physical port. 

  - device -- The abstracted device object.  */

struct fp_port 
{
  fp_port_id_t  id;
  unsigned char addr[FP_MAC_ADDR_LEN];
  char          name[FP_MAX_NAME_LEN];

  /* State */
  int no_recv   : 1;
  int no_fwd    : 1;
  int no_pkt_in : 1;
  int link_down : 1; /* Port down? */
  int live      : 1; /* Separate from link down? */

  struct fp_device* device;
};


struct fp_port* fp_port_create(struct fp_device*);
void fp_port_delete(struct fp_port*);
int fp_port_output(struct fp_port*, struct fp_context*);


/* Receive a packet from a port. */
static inline struct fp_packet*
fp_port_recv_packet(struct fp_port* port)
{
  return port->device->vtbl->recv(port->device);
}


/* Send a packet on a port. */
static inline int
fp_port_send_packet(struct fp_port* port, struct fp_packet* pkt)
{
  return port->device->vtbl->send(port->device, pkt);
}


/* Drop a packet from the processing pipeline. */
static inline void
fp_port_drop_packet(struct fp_port* port, struct fp_packet* pkt)
{
  port->device->vtbl->drop(port->device, pkt);
}


/* Returns true if the id indicates a reserved port. */
static inline bool
fp_is_reserved_port(fp_port_id_t id)
{
  return id > FP_PORT_MAX_ID;
}


#endif
