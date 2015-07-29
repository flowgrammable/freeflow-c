// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PIPELINE_H
#define FLOWPATH_PIPELINE_H

#include "error.h"
#include "dataplane.h"
#include "packet.h"


/* A pipeline is a parameterized packet processing function
   whose behavior is defined entirely with an exteranlly
   loaded module (shared object). A pipeline has several major
   features:

   TODO: Document the members of this struct.


   Not every pipeline has a decoder. For example, simple
   hub pipelines do not decode packets.

   TODO: The current design implements a single stage
   decoder that gathers all information needed to operate
   on the packet. How can we modify the design to allow
   for multi-stage (or on-demand) decodeing?

   ## Notes ##
   We don't use a vtbl approach here because there are
   likely to be only a few of these objects. We can
   sacrifice the space for an extra indirection. */
struct fp_pipeline 
{
  void* pipeline_module; /* The pipeline's DLL. */
  void* pipeline_object; /* Pipeline data. */

  struct fp_dataplane* dp; /* The owning dataplane. */

  /* Initialization. */
  fp_error_t (*load)(struct fp_dataplane*);
  fp_error_t (*unload)(struct fp_dataplane*);
  
  /* Configuration. */
  fp_error_t (*add_port)(struct fp_dataplane*, struct fp_port*);
  fp_error_t (*del_port)(struct fp_dataplane*, struct fp_port*);

  /* State manipulation. */
  fp_error_t (*start)(struct fp_dataplane*);
  fp_error_t (*stop)(struct fp_dataplane*);

  /* Processing Element's Interface. */
  void (*insert)(struct fp_dataplane*, struct fp_packet*, struct fp_arrival);
};


struct fp_pipeline* fp_pipeline_load(struct fp_dataplane*, char const*, fp_error_t*);
void                fp_pipeline_unload(struct fp_dataplane*, fp_error_t*);


#endif
