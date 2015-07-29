// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_TABLE_H
#define FLOWPATH_TABLE_H

#define FP_MAX_TABLES           32

#define FP_TABLE_MATCH_EXACT    1
#define FP_TABLE_MATCH_PREFIX   2
#define FP_TABLE_MATCH_WILDCARD 3


struct fp_instruction;
struct fp_context;

/* A flow is an entry in a flow table. Each flow is described
   by a tuple, which includes its priority, counters, associated
   instructions and other data.

   Note that the match that defines the flow is a property
   of the table, not this object. 

   TODO: Implement counters, timeouts, and cookies. */
struct fp_flow
{
  int priority;   /* The priority of the flow. */
  int program;    /* The program associated with the flow. */
};

/* A flow table maintains a mapping of keys to flow entries.
   Flow tables are one of several kinds:
   - exact match (hash table)
   - prefix match (prefix tree)
   - wildcard match (???)
   - others?
  The specific type of flow table is maintained by the underlying
  table object.

  This wrapper contains affiliated counters and
  configuration information.

  The underlying table maps keys to lists of flows where each list
  an array sorted by priority. 

  TODO: Every flow table is explicitly equipped with a table-miss
  flow entry. Table miss does not need to forward to the controller;
  it could drop or forward to a different table. Note that table-
  miss means different things for different kinds of flow table.
  For example, you can't install a table-miss rule in a hash table
  because the hash table matches keys exactly. The default behavior
  is to drop unmatched packets. */
struct fp_flow_table
{
  /* FIXME: This is totally TOTALLY wrong. */
  struct fp_flow* flow;
};

struct fp_flow_table* fp_flow_table_new(int match, int size);

void fp_flow_add(struct fp_flow_table* table, struct fp_flow* flow);
void fp_flow_remove(struct fp_flow_table* table, struct fp_flow* flow);

struct fp_flow* fp_match(struct fp_flow_table* table, 
                         struct fp_context* cxt);


#endif
