// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "proto.h"

#include <stdlib.h>
#include <string.h>


/* Transform buf into a request. The buffer must be at least 
   FP_MESSAGE_LEN bytes. Note that this does not allocate 
   memory. s*/
struct fp_request*
fp_make_request(void* buf)
{
  memset(buf, 0, FP_MESSAGE_LEN);
  struct fp_request* req = (struct fp_request*)buf;
  req->type = FP_REQUEST;
  return req;
}


/* Transform buf into a reply. The buffer must be at least 
   FP_MESSAGE_LEN bytes. Note that this does not allocate 
   memory. */
struct fp_reply*
fp_make_reply(void* buf)
{
  memset(buf, 0, FP_MESSAGE_LEN);
  struct fp_reply* rep = (struct fp_reply*)buf;
  rep->type = FP_REPLY;
  return rep;
}
