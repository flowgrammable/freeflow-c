// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "pipeline.h"

#include <dlfcn.h>


/* The type of function used to instantiate a pipeline
   object from a module. Returns -1 on error. */
typedef fp_error_t (*pipeline_ctor)(struct fp_pipeline*);


/* A helper function for releasing resources on failure. */
static inline void
delete_pipeline(struct fp_pipeline* p)
{
    dlclose(p->pipeline_module);
    fp_deallocate(p);
}


/* Load the given DLL as a pipeline provider. Returns NULL if
   the pipeline cannot be loaded. */
struct fp_pipeline*
fp_pipeline_load(struct fp_dataplane* dp, char const* dll, fp_error_t* err)
{
  assert(dll != NULL);
  fprintf(stderr, "[noproto] loading pipeline '%s'\n", dll);

  /* Initialize the pipeline. */
  struct fp_pipeline* p = fp_allocate(struct fp_pipeline);
  memset(p, 0, sizeof(struct fp_pipeline));
  
  /* Open the module. */
  p->pipeline_module = dlopen(dll, RTLD_NOW);
  if (!p->pipeline_module) {
    *err = FP_BAD_PIPELINE;
    fp_deallocate(p);
    return NULL;
  }
  fprintf(stderr, "[noproto] pipeline module loaded\n");

  /* Find the pipeline constructor. */
  pipeline_ctor init = dlsym(p->pipeline_module, "pipeline_init");
  if (!init) {
    *err = FP_BAD_PIPELINE_MODULE;
    delete_pipeline(p);
    return NULL;
  }
  fprintf(stderr, "[noproto] resolved pipeline construcor\n");

  /* Instantiate the pipeline. */
  *err = init(p);
  if (*err != FP_OK) {
    *err = FP_BAD_PIPELINE_MODULE;
    delete_pipeline(p);
    return NULL;
  }
  dp->pipeline = p;

  /* Load pipeline resources. */
  fprintf(stderr, "[noproto] initialize pipeline\n");
  *err = p->load(dp);
  if (*err != FP_OK) {
    delete_pipeline(p);
    return NULL;    
  }

  return p;
}


/* Unload and destroy the pipeline. 

   TODO: When unloading the pipeline from a adta
*/
void
fp_pipeline_unload(struct fp_dataplane* dp, fp_error_t* err)
{
  assert(dp != NULL);

  struct fp_pipeline* p = dp->pipeline;

  /* Unload resources. */
  p->unload(dp);

  /* Close the pipeline handle. */
  if (p->pipeline_module)
    dlclose(p->pipeline_module);

  fp_deallocate(p);
  
  dp->pipeline = NULL;
}
