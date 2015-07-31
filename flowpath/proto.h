// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_PROTO_H
#define FLOWPATH_PROTO_H


/* This moule defines the message format communications between
   a flowpath instance and a flowmgr instance.

   TODO: How do we extend this protocol to enable application
   specific codes to be sent across? */


#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* The size of all messages sent or received though the dpmp channel. */
#define FP_MESSAGE_LEN 1024


/* Maximim string lengths. */
#define FP_STRING_MAX_LEN 128


/* The kinds of messages in the dpmp configuration
   protocol. We currently support only requests and
   replies. */

#define FP_REQUEST  0  /* Client initiated request. */
#define FP_REPLY    1  /* Server initiated response. */


/* The kinds of requests and replies. These are
   used to determine the arguments and results for
   each kind of request. */

/* Data plane configuration. */
#define FP_DATPLANE_ADD   1
#define FP_DATPLANE_DEL   2
#define FP_DATPLANE_SET   3
#define FP_DATPLANE_SHOW  4
#define FP_DATPLANE_STAT  5
#define FP_DATPLANE_LIST  6
/* Pipeline configuration. */
#define FP_DECODER_GET    10
#define FP_DECODER_SET    11
/* Port configuration. */
#define FP_PORT_ADD       20
#define FP_PORT_DEL       21
#define FP_PORT_GET       22
#define FP_PORT_SET       23
#define FP_PORT_LIST      24
/* Flow table configuration. */
#define FP_TABLE_ADD      31
#define FP_TABLE_DEL      32
#define FP_TABLE_GET      33
#define FP_TABLE_SET      34


/* The kinds of result requests. */
#define FP_SUCCESS                   0
#define FP_FAILURE                   1  /* Unspcified error. */
#define FP_BAD_REQUEST               2  /* Request with wrong type. */
#define FP_BAD_REPLY                 3  /* Reply with wrong type. */
#define FP_BAD_OPERATION             4  /* Unknow operation. */
#define FP_BAD_DATAPLANE             5  /* No such data plane. */
#define FP_DATAPLANE_EXISTS          6  /* Data plane already exists. */
#define FP_DATAPLANE_LIMIT_EXCEEED   7  /* Too many data planes. */
#define FP_BAD_PIPELINE              8  /* Cannot load pipeline module. */
#define FP_BAD_PIPELINE_MODULE       9  /* Cannot resolve pipeline symbols. */


/* -------------------------------------------------------------------------- */
/*                          Data plane requests                               */

/* Data plane properties. */
#define FP_DATAPLANE_STATE 1 /* Up/down */


/* Data plane property values. */
#define FP_DATAPLANE_STATE_UP   1 /* Up */
#define FP_DATAPLANE_STATE_DOWN 0 /* Down */


/* Add data plane arguments. The data plane type denotes
   the module that is loaded to define the pipeline. */
struct fp_dataplane_add_arguments
{
  char name[FP_STRING_MAX_LEN];
  char type[FP_STRING_MAX_LEN];
};


/* Remove data plane arguments. Only the data plane name
   is needed for this operation. */
struct fp_dataplane_del_arguments
{
  char name[FP_STRING_MAX_LEN];
};


/* Arguments for querying a data plane. This is used for
   requests to delete, show, and query stats for a
   data plane.  */
struct fp_dataplane_query_arguments
{
  char name[FP_STRING_MAX_LEN];
};


/* The arguments for setting a property. See property
   values above. */
struct fp_dataplane_set_arguments
{
  uint8_t property;
  union
  {
    int state;
  } u;
};


/* Arguments for listing data planes. */
struct fp_dataplane_list_arguments
{
};

/* -------------------------------------------------------------------------- */
/*                            Port requests                                   */

/* Add port arguments. These identify the data plane
   to which the port is added and the name of the device
   being added as a port. */
struct fp_port_add_arguments
{
  char name[FP_STRING_MAX_LEN];
  char type[FP_STRING_MAX_LEN];
  char device[FP_STRING_MAX_LEN];
  char options[FP_STRING_MAX_LEN];
};


/* The result of adding a port is the unique id of
   the added port. */
struct fp_port_add_result
{
  uint16_t pid;
};


/* Delete port arguments. These identify the data plane
   and port to be deleted. The result of deleting a
   port is the result code.  */
struct fp_port_del_arguments
{
  char name[FP_STRING_MAX_LEN];
  /* FIXME: This isn't right. */
  char pid[FP_STRING_MAX_LEN];  
};


/* The result of deleting a port is the id of the port. */
struct fp_port_del_result
{
  uint16_t pid;
};


/* List ports arguments. Identifies the data plane name. */
struct fp_port_list_arguments
{
  char name[FP_STRING_MAX_LEN];
};


/* List ports result. Contains a list of ports being used. */
struct fp_port_list_result
{
  char ports[FP_STRING_MAX_LEN];
};


/* -------------------------------------------------------------------------- */
/*                          Request and reply                                 */


/* A configuration request. */
struct fp_request
{
  uint8_t       type;    /* Always FP_REQUEST */
  uint8_t       kind;    /* The kind of message. */
  unsigned char data[0]; /* Message data. */
};


/* A reply to a request. This message includes the
   the result code in addition to any result value. */
struct fp_reply
{
  uint8_t       type;    /* Always FP_REPLY */
  uint8_t       kind;    /* The kind of message. */
  int32_t       result;  /* The result code. */
  unsigned char data[0]; /* Message data. */
};


/* -------------------------------------------------------------------------- */
/*                          Message constructors                              */

struct fp_request* fp_make_request(void*);
struct fp_reply*   fp_make_reply(void*);


/* Return the arguments for adding a data plane. */
inline struct fp_dataplane_add_arguments* 
fp_get_dataplane_add_arguments(struct fp_request* req)
{
  req->kind = FP_DATPLANE_ADD;
  return (struct fp_dataplane_add_arguments*)(req->data);
}


/* Return the arguments for adding a data plane. */
inline struct fp_dataplane_query_arguments* 
fp_get_dataplane_del_arguments(struct fp_request* req)
{
  req->kind = FP_DATPLANE_DEL;
  return (struct fp_dataplane_query_arguments*)(req->data);
}


/* Return the arguments for deleting a data plane. */
inline struct fp_dataplane_set_arguments* 
fp_get_dataplane_set_arguments(struct fp_request* req)
{
  req->kind = FP_DATPLANE_SET;
  return (struct fp_dataplane_set_arguments*)(req->data);
}


/* Return the arguments for showing a data plane. */
inline struct fp_dataplane_query_arguments* 
fp_get_dataplane_show_arguments(struct fp_request* req)
{
  req->kind = FP_DATPLANE_SHOW;
  return (struct fp_dataplane_query_arguments*)(req->data);
}


/* Return the arguments for getting statistics from a data plane. */
inline struct fp_dataplane_query_arguments* 
fp_get_dataplane_stat_arguments(struct fp_request* req)
{
  req->kind = FP_DATPLANE_STAT;
  return (struct fp_dataplane_query_arguments*)(req->data);
}


/* Return the arguments for listing data plane. */
inline struct fp_dataplane_list_arguments* 
fp_get_dataplane_list_arguments(struct fp_request* req)
{
  req->kind = FP_DATPLANE_LIST;
  return (struct fp_dataplane_list_arguments*)(req->data);
}


/* Return the arguments for adding a port to a data plane. */
inline struct fp_port_add_arguments*
fp_get_port_add_arguments(struct fp_request* req)
{
  req->kind = FP_PORT_ADD;
  return (struct fp_port_add_arguments*)(req->data);
}


/* Return the arguments for deleteing a port from a data plane. */
inline struct fp_port_del_arguments*
fp_get_port_del_arguments(struct fp_request* req)
{
  req->kind = FP_PORT_DEL;
  return (struct fp_port_del_arguments*)(req->data);
}


/* Returns the arguments for listing ports in a data plane. */
inline struct fp_port_list_arguments*
fp_get_port_list_arguments(struct fp_request* req)
{
  req->kind = FP_PORT_LIST;
  return (struct fp_port_list_arguments*)(req->data);
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif
