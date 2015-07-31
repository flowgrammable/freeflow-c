// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PACKET_H
#define FLOWPATH_PACKET_H

#include "types.h"


#define FP_MAX_KEY_LEN   64
#define FP_MAX_VALUE_LEN 128

typedef enum {FP_BUF_NADK, FP_BUF_NETMAP, FP_BUF_ALLOC} fp_buf_t;


/* A packet is a datagram containing layered protocol information
   and application data.

   The packet structure is effectively a reference to a buffer
   maintained by an underlying interface. Most interfaces with
   which flowpath is designed to work expose mechanisms for
   avoiding copies when a packet is received. The port module
   that creates the packet is fully responsible for the
   management of its memory.

   TODO: Instead of forcing a single array, a linking of segments 
   would be more ideal. Then allow egress to perform a gather 
   operation. */
struct fp_packet
{
  unsigned char* data; /* Packet buffer. */
  int            size; /* Number of bytes. */
  uint64_t       timestamp;  /* Time of packet arrival */
  void*          buf_handle; /* [optional] port-specific buffer handle */
  fp_buf_t       buf_dev;    /* [optional] owner of buffer handle (dev*) */
};


struct fp_packet* fp_packet_create(unsigned char*, int, uint64_t,
                                   void*, fp_buf_t);
void              fp_packet_delete(struct fp_packet*);


/* The arrival type saves information about the arrival
   of a packet on a port. There are several piecees of
   information in the arrival context:

   - in_port -- The port number on which the packet arrived.
     This could indicate either a physical or logical port.

   - in_phy_port -- If the in_port designates a logical port,
     then this reports the underlying physical port on which
     the packet was received.

   - tunnel_id -- If the in_port designates a logical port,
     the tunnel_id may be set to indicate a tunnel for
     e.g., GRE tunneling.

   - timestamp -- When the packet was received.

   TODO: This might actually be a set of macros that can
   access dedicated registers, etc. */
struct fp_arrival
{
  fp_port_id_t in_port;
  fp_port_id_t in_phy_port;
  int          tunnel_id;
};



/* FIXME: What am I? */
struct fp_base_key 
{
  fp_port_id_t in_port;
  fp_port_id_t in_phy_port;
};



/* A packet subkey is a bitstring representing information
   gathered from the Key (and thus, extracted from a packet).
   The purpose of the subkey is to gather relevant information
   from the Key for a Table lookup.

   TODO: Small-object implementation. Most subkeys will probably
   be small (2 or 3 headers worth of information).   Choose a 
   default size for the key (say, 64 bytes?) to allow most extracted 
   data to reside in non-dynamically allocated memory.  

   Currently, the subkey is strictly a simple buffer. That is,
   there is no dynamic allocation. Add this when needed. 

   TODO: In order to support multiple flow tables with different
   key size requirements, we even make this a union of several
   fixed-size keys that can be assigned into registers of
   varying size.

   TODO: If we use subkeys to search on wildcard matches, how
   do we encode wildcards? We probably need a different
   key structure for that.
*/
struct fp_packet_subkey
{
  int           size;
  unsigned char data[FP_MAX_KEY_LEN];
};


/* A packet context wraps a packet and provides information
   about its receipt and processing. 
   */
struct fp_context 
{
  struct fp_packet* packet;      /* The packet data. */
  
  /* Arrival metadata. */
  fp_port_id_t in_port;
  fp_port_id_t in_phy_port;
  int          tunnel_id;
//  int          timestamp;

  /* Actions.

     TODO: The output and next table should be implicit in
     the action set. */
  fp_port_id_t out_port;    /* The output port. */
  int table;       /* The current table */

  
  /* Configurable elements. */
  /* Important: the size of the Key is configurable */
  struct fp_base_key* key;

};


struct fp_context* fp_context_create(struct fp_packet*, struct fp_arrival);
void               fp_context_delete(struct fp_context*);


#endif
