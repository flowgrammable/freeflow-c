// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "error.h"

#include <string.h>


/* Table of error messages corresponding to error codes. */
static char const* errors[] = {
  "Success",                      /* FP_OK */
  "Unknown error",                /* FP_ERROR */
  "Bad request type",             /* FP_BAD_REQUEST */
  "Bad reply type",               /* FP_BAD_REPLY */
  "Bad operation",                /* FP_BAD_OPERATION */
  "Bad data plane",               /* FP_BAD_DATAPLANE */
  "Data plane already exists",    /* FP_DATAPLANE_EXISTS */
  "Too many data planes",         /* FP_DATAPLANE_LIMIT_EXCEEED */
  "Cannot load pipeline",         /* FP_BAD_PIPELINE */
  "Cannot load pipeline symbols", /* FP_BAD_PIPELINE_MODULE */
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
