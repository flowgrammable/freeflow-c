// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_ERROR_H
#define FLOWPATH_ERROR_H

/* This module provides tools for managing and diagnosing
   system and dataplane errors. */

#include <assert.h>
#include <stdbool.h>
#include <errno.h>


#define FP_OK                        0  /* No error. */
#define FP_ERROR                     1  /* Unspecified error. */
#define FP_BAD_REQUEST               2  /* Request with wrong type. */
#define FP_BAD_REPLY                 3  /* Reply with wrong type. */
#define FP_BAD_OPERATION             4  /* Unknown operation. */
#define FP_BAD_DATAPLANE             5  /* No such data plane. */
#define FP_DATAPLANE_EXISTS          6  /* Data plane already exists. */
#define FP_DATAPLANE_LIMIT_EXCEEED   7  /* Too many data planes. */
#define FP_BAD_PIPELINE              8  /* Cannot load pipeline module. */
#define FP_BAD_PIPELINE_MODULE       9  /* Cannot resolve pipeline symbols. */


#ifdef __cplusplus
extern "C" {
#endif


/* The error type is the same as the result type for the
   the flowmgr/flowpath communication. When the value is
   negative, this indicates an errno error value. */
typedef int fp_error_t;


char const* fp_strerror(int);


/* Returns true if the given error value indicates success. */
static inline bool
fp_ok(fp_error_t e)
{
  return e == FP_OK;
}


/* Returns true if the given error value indicates failure. */
static inline bool
fp_error(fp_error_t e)
{
  return e != FP_OK;
}


/* Make the sytsem error e into a flowpath error. */
static inline fp_error_t 
fp_system_error(int e)
{
  assert(e >= 0);
  return -e;
}


/* Returns a flowpath error version of the current
   error number. */
static inline fp_error_t
fp_get_system_error()
{
  return fp_system_error(errno);
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif
