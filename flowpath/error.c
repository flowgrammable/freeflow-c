// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "error.h"

#include <string.h>


/* Table of error messages corresponding to error codes. */
static char const* errors[] = {
  "Success",                      /* NP_OK */
  "Unknown error",                /* NP_ERROR */
  "Bad request type",             /* NP_BAD_REQUEST */
  "Bad reply type",               /* NP_BAD_REPLY */
  "Bad operation",                /* NP_BAD_OPERATION */
  "Bad data plane",               /* NP_BAD_DATAPLANE */
  "Data plane already exists",    /* NP_DATAPLANE_EXISTS */
  "Too many data planes",         /* NP_DATAPLANE_LIMIT_EXCEEED */
  "Cannot load pipeline",         /* NP_BAD_PIPELINE */
  "Cannot load pipeline symbols", /* NP_BAD_PIPELINE_MODULE */
};


/* Return the error string associated with the value.  */
char const*
fp_strerror(int n)
{
  if (n < 0)
    return strerror(-n);
  else
    return errors[n];
}
