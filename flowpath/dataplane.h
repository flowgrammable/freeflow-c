// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_DATAPLANE_H
#define FLOWPATH_DATAPLANE_H

/* The data plane module defines the primary interface
   to the management of virtual data planes. A data
   plane owns a packet processing pipeline and its
   associated resources. */

#include "util.h"
#include "error.h"
#include "types.h"
#include "hash.h"

struct fp_packet;
struct fp_context;
struct fp_port;


/* A port table is a collection of named ports. This is a
   resource managed by the data plane. */
struct fp_ports
{
  struct fp_chained_hash_table* table;
};


/* The set of all tables in the pipeline.
   
   TODO: This is probably not the right abstraction
   for managing tables. But it will work for now. */
struct fp_tables
{
  struct fp_chained_hash_table* table;
};


/* Processing state for the dataplane. */
#define FP_DATAPLANE_UP 0x01


/* A dataplane object. */
struct fp_dataplane
{
  struct fp_dataplane* next;

  int         id;    /* Unique data plane id. */
  int         state; /* The current state of the dataplane. */
  char const* name;  /* Data plane name. */
  char const* type;  /* Data plane type. */

  struct fp_ports  ports;
  struct fp_tables tables;

  struct fp_pipeline* pipeline;

  /* Configuration parameters for Modular stages. */
  size_t key_size;   /* Number of bytes of user-defined Key */
} fp_dataplane_t;


struct fp_dataplane* fp_dataplane_create(char const*, char const*, fp_error_t*);
void                 fp_dataplane_delete(struct fp_dataplane*, fp_error_t*);
struct fp_dataplane* fp_dataplane_lookup(char const*);

void            fp_dataplane_add_port(struct fp_dataplane* dp, struct fp_port*, fp_error_t*);
void            fp_dataplane_remove_port(struct fp_dataplane* dp, struct fp_port*, fp_error_t*);
struct fp_port* fp_dataplane_get_port(struct fp_dataplane*, fp_port_id_t);
void            fp_dataplane_list_ports(struct fp_dataplane*, struct fp_port**, fp_error_t*);

fp_error_t fp_dataplane_start(struct fp_dataplane*);
fp_error_t fp_dataplane_stop(struct fp_dataplane*);


/* Returns true if the dataplane is up and running. */
static inline bool
fp_dataplane_is_up(struct fp_dataplane const* dp)
{
  return dp->state & FP_DATAPLANE_UP;
}


/* Put the dataplane in the up state. This should only ever be called
   from the start() method of a pipeline module. */
static inline void
fp_dataplane_up(struct fp_dataplane* dp)
{
  dp->state |= FP_DATAPLANE_UP;
}


/* Put the dataplane in the down state. This should only ever be called
   from the start() method of a pipeline module. */
static inline void
fp_dataplane_down(struct fp_dataplane* dp)
{
  dp->state &= ~FP_DATAPLANE_UP;
}


#endif
