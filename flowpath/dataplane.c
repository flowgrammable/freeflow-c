// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved
  
#include "dataplane.h"
#include "pipeline.h"
#include "port.h"
#include "proto.h"


#define FP_DATAPLANE_MAX 64

/* Table of data planes. 
   FIXME: This should probably be a hash table. */
static struct fp_dataplane* dps_[FP_DATAPLANE_MAX];


/* Allocate a new data plane in the first empty slot
   in  the data plane table. */
static struct fp_dataplane*
allocate_dataplane()
{
  int i = 0;
  for (; i < FP_DATAPLANE_MAX; ++i) {
    if (!dps_[i]) {
      dps_[i] = fp_allocate(struct fp_dataplane);
      dps_[i]->id = i;
      return dps_[i];
    }
  }
  return NULL;
}


/* Release the slot in the data plane table. */
static inline void
deallocate_dataplane(struct fp_dataplane* dp)
{
  int slot = dp->id;
  fp_deallocate(dps_[slot]);
  dps_[slot] = NULL;
}


/* Return the slot for a data plane with the given name.
   Returns -1 if no sucu data plane exists. */
static struct fp_dataplane*
find_dataplane(char const* name)
{
  int i = 0;
  for (; i < FP_DATAPLANE_MAX; ++i)
    if (dps_[i] && !strcmp(dps_[i]->name, name))
      return dps_[i];
    
  return NULL;
}


/* Create a new data plane object with the given name
   and type. If the dataplane cannot be allocated, this
   returns null.

   The type of the dataplane determines the default
   "shape" of the pipeline. That "shape" is determined
   by loading a DLL that implements a set of stages
   to support different kinds of applications. */
struct fp_dataplane*
fp_dataplane_create(char const* name, char const* type, fp_error_t* err)
{  
  /* Don't create data planes with the same name. */
  if (fp_dataplane_lookup(name)) {
    *err = FP_DATAPLANE_EXISTS;
    return NULL;
  }

  /* Create the data plane. We need to pass this to the
     pipeline constructor (assuming it's valid). */
  struct fp_dataplane* dp = allocate_dataplane();
  if (!dp) {
    *err = FP_DATAPLANE_LIMIT_EXCEEED;
    return NULL;
  }
  dp->name = name;
  dp->type = type;

  /* Load a pipeline based on the type of dataplane. 

     FIXME: Set an error condition if we can't load
     the pipeline. */
  dp->pipeline = fp_pipeline_load(dp, type, err);
  if (!dp->pipeline) {
    fp_dataplane_delete(dp, err);
    return NULL;
  }

  /* Initialize the port table. */
  dp->ports.table = fp_chained_hash_table_new(17, fp_uint_hash, fp_uint_eq);
  
  /* Initialize an empty set of flow tables. */
  dp->tables.table = fp_chained_hash_table_new(17, fp_uint_hash, fp_uint_eq);

  /* This will be updated if key is changed. */
  dp->key_size = sizeof(struct fp_base_key); 

  return dp;
}


/* Remove the dataplane from the list of managed
   dataplanes and reclaim its resources. 

   TODO: We have to destroy ports and tables, and all
   such other things.

   TODO: Can we remove a dataplane that has packets?
   Running applications?
   */
void
fp_dataplane_delete(struct fp_dataplane* dp, fp_error_t* err)
{
  /* Release the pipeline. */
  if (dp->pipeline)
    fp_pipeline_unload(dp, err);

  /* Clear the port table. */
  fp_chained_hash_table_delete(dp->ports.table);

  /* Clear the flow tables. */
  fp_chained_hash_table_delete(dp->tables.table);

  /* Release the slot from the database table. */
  deallocate_dataplane(dp);
}


/* Get the data plane with the given index. */
struct fp_dataplane*
fp_dataplane_lookup(char const* name)
{
  return find_dataplane(name);
}


/* Add a port to the data plane and notify the pipeline about
   the addition. */
void
fp_dataplane_add_port(struct fp_dataplane* dp, struct fp_port* port, fp_error_t* err)
{
  /* TODO: Can insertion ever fail? */
  struct fp_chained_hash_table* t = dp->ports.table;
  fp_chained_hash_table_insert(t, port->id, (uintptr_t)port);
  *err = dp->pipeline->add_port(dp, port);
}


/* Remove the port from the data plane. This does not destroy
   the port object.

   TODO: Do we need to stop the port, release memory, 
   clear its queues? What? */
void
fp_dataplane_remove_port(struct fp_dataplane* dp, struct fp_port* port, fp_error_t* err)
{
  /* TODO: Is the success of the pipeline removal a necessary
     condition for removing from the data plane's port table? */
  *err = dp->pipeline->del_port(dp, port);
  if (err == FP_OK) {
    struct fp_chained_hash_table* t = dp->ports.table;
    fp_chained_hash_table_remove(t, port->id);
  }  
}

/* Returns a list of port objects that the data plane is currently using. */
void
fp_dataplane_list_ports(struct fp_dataplane* dp, struct fp_port** ports, fp_error_t* err)
{
  struct fp_chained_hash_table* t = dp->ports.table;  
  int i;
  for (i = 0; i < t->size; i++) {
    ports[i] = (struct fp_port*)t->data[i]->value;
  }
}


/* Returns a pointer to the port with the given id or NULL
   if no such port exists. */
struct fp_port*
fp_dataplane_get_port(struct fp_dataplane* dp, fp_port_id_t p)
{
  struct fp_chained_hash_table* t = dp->ports.table;
  struct fp_chained_hash_entry* ent = fp_chained_hash_table_find(t, p);
  if (ent)
    return (struct fp_port*)ent->value;
  else
    return NULL;
}


/* Start the data plane. */
fp_error_t
fp_dataplane_start(struct fp_dataplane* dp)
{
  if (fp_dataplane_is_up(dp))
    return FP_OK;

  fp_error_t err = dp->pipeline->start(dp);
  if (fp_ok(err))
    fp_dataplane_up(dp);
  return err;
}


fp_error_t
fp_dataplane_stop(struct fp_dataplane* dp)
{
  if (!fp_dataplane_is_up(dp))
    return FP_OK;

  fp_error_t err = dp->pipeline->start(dp);
  if (fp_ok(err))
    fp_dataplane_up(dp);
  return err;
}



#if 0


/* Return the port object for the given port id, or null
   if there is no port with that id.

   The port id shall not be a reserved port; there are 
   no port objects associated with those ids.
 */
struct fp_port*
fp_get_port(struct fp_dataplane* dp, int id)
{
  assert(!fp_is_reserved_port(id));
  if (id >= dp->nports)
    return NULL;
  return dp->ports[id];
}

/* Add a flow table. 

   TODO: We might be able to reuse tables that have the same match
   and are under-utilized.
 */
struct fp_flow_table*
fp_add_flow_table(struct fp_dataplane* dp, int match, int size)
{
  /* Find an available table. */
  int n;
  struct fp_flow_table* table;

  for (n = 0; n < dp->ntables; ++n) {
    if (!dp->tables[n])
      break;
  }
  if (n == dp->ntables)
    return NULL;

  /* Create the new table */
  table = fp_flow_table_new(match, size);
  dp->tables[n] = table;
  return table;
}

/* Return the table with index n.
 */
struct fp_flow_table* 
fp_get_flow_table(struct fp_dataplane* dp, int n)
{
  assert(0 <= n && n < dp->ntables);
  return dp->tables[n];
}

struct fp_program* 
fp_get_program(struct fp_dataplane* dp, int n)
{
  assert(0 <= n && n < dp->nprograms);
  return dp->programs[n];
}


/* Inserts a packet into the data plane. The arrival stage
 * allocates and initializes the packet context.  The Modular
 * pipeline is then executed.  Finally, the packet is output
 * from the departure stage.
 */
void
fp_pipeline_process(struct fp_dataplane* dp, struct fp_packet* pkt,
                  fp_port_id_t in_port, fp_phy_id_t in_phy_port)
{
  struct fp_context* ctx = fp_stage_arrival_exec(dp, pkt, in_port, in_phy_port);
  fp_pipeline_exec(dp, ctx);
  fp_stage_departure_exec(dp, ctx);
}


/* Pushes packet through each Modular stage in the pipeline.  Ingress and
 * egreess are handled outside of this function.

   TODO: currently assumes a linear pipeline where every stage is executed
         one after another (without branching).
 */
void
fp_pipeline_exec(struct fp_dataplane* dp, struct fp_context* ctx)
{
  int i;
  struct fp_pipeline* pipe = dp->pipe;

  for (i = 0; i < pipe->nstages; ++i) {
    struct fp_stage* stage = pipe->stages[i];
    stage->exec(dp, ctx);
  }
}
#endif
