// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "flow.h"
#include "util.h"


/* FIXME: Do something better with this. */

struct fp_flow_table*
fp_flow_table_new(int match, int size)
{
  struct fp_flow_table* table = allocate(struct fp_flow_table);
  return table;
}

/* Add the given flow to the flow table. 

   TODO: Define a collision strategy. 

   FIXME: Actually do something useful. */
void
fp_flow_add(struct fp_flow_table* table, struct fp_flow* flow)
{
  table->flow = flow;
}

/* Remove the given flow from the flow table. 

   FIXME: Actually do something useful. */
void
fp_flow_remove(struct fp_flow_table* table, struct fp_flow* flow)
{
  table->flow = NULL;
}

/* Search the flow table for the lowest priority entry that
   matches the current packet context. */

struct fp_flow* 
fp_match(struct fp_flow_table* table, struct fp_context* cxt)
{
  /* FIXME: This is not a search. */
  return table->flow;
}

