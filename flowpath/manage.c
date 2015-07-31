// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "manage.h"
#include "dataplane.h"
#include "port.h"
#include "port_udp.h"
#include "proto.h"

#include "error.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


/* The file for communicating with flowmgr. */
static int mgr_;


/* FIXME: Make this configurable. */
static const char* mgr_socket_ = "/tmp/flowpath-socket";


/* Open a connection to the flowmgr manager channel. Returns
   a file descriptor for the connected manager, or -1
   if the connection failed. */
int 
fp_mgr_open()
{
  /* Initialize the flowmgr connection. */
  mgr_ = socket(AF_UNIX, SOCK_STREAM, 0);
  if (mgr_ < 0) {
    perror("fp_mgr_open/socket");
    return -1;
  }

  /* Create the socket address. */
  struct sockaddr_un peer;
  memset(&peer, 0, sizeof(peer));
  peer.sun_family = AF_UNIX;
  strcpy(peer.sun_path, mgr_socket_);

  /* Connect to the required address. */
  if (connect(mgr_, (struct sockaddr*)&peer, sizeof(peer)) < 0) {
    perror("fp_mgr_open/connect");
    return -1;
  }

  fprintf(stderr, "[flowpath] connected to flowmgr\n");

  return mgr_;
}


/* Close the flowmgr channel. */
int
fp_mgr_close()
{
  fprintf(stderr, "[flowpath] disconnecting from flowmgr\n");
  return close(mgr_);
}


/* Adds a new data plane.  */
static int
fp_on_dataplane_add(struct fp_request const* req, struct fp_reply* rep)
{
  fprintf(stderr, "[flowpath] add dataplane\n");

  struct fp_dataplane_add_arguments const* args = 
    (struct fp_dataplane_add_arguments const*)req->data;
  

  if (fp_dataplane_lookup(args->name))
    rep->result = FP_DATAPLANE_EXISTS;
  else
    fp_dataplane_create(args->name, args->type, &rep->result);
  
  return rep->result;
}


/* Deletes a data plane.*/
static int
fp_on_dataplane_del(struct fp_request const* req, struct fp_reply* rep)
{
  fprintf(stderr, "[flowpath] delete dataplane\n");

  struct fp_dataplane_del_arguments const* args = 
    (struct fp_dataplane_del_arguments const*)req->data;

  struct fp_dataplane* dp = fp_dataplane_lookup(args->name);
  if (dp) 
    fp_dataplane_delete(dp, &rep->result);
  else
    rep->result = FP_BAD_DATAPLANE;
  return rep->result;
}


/* Adds a port. 

   FIXME: Add the port the poll set in the main loop. */
static int
fp_on_port_add(struct fp_request const* req, struct fp_reply* rep)
{
  // struct fp_port* port = fp_netmap_open(msg->device, msg->id);
  // fp_dataplane_add_port(port);
  fprintf(stderr, "[flowpath] add port\n");

  struct fp_port_add_arguments const* args =
    (struct fp_port_add_arguments const*)req->data;

  struct fp_dataplane* dp = fp_dataplane_lookup(args->name);
  if (dp) {
    fp_error_t derr = 0;
    struct fp_device* dev;
    
    /* Based on the port 'type' create the appropriate device. */
    if (!strcmp(args->type, "udp")) {
      dev = fp_udp_open_port(atoi(args->device), &derr);
    } 
    else if (!strcmp(args->type, "eth")) {

    } 
    else if (!strcmp(args->type, "netmap")) {

    }  
    else if (!strcmp(args->type, "dpdk")) {

    }

    /* Check for device creation errors. */
    if (fp_error(derr)) {
      fprintf(stderr, "error: %s\n", fp_strerror(derr));
      return -1;
    }
    
    /* Create and add the port. */
    struct fp_port* port = fp_port_create(dev);
    fp_dataplane_add_port(dp, port, &rep->result);
    if (fp_error(rep->result)) {
      fprintf(stderr, "error: %s\n", fp_strerror(rep->result));
      return -1;
    }
  }
  else
    rep->result = FP_BAD_DATAPLANE;

  return rep->result;
}


/* Deletes a port. */
static int
fp_on_port_del(struct fp_request const* req, struct fp_reply* rep)
{
  fprintf(stderr, "[flowpath] del port\n");

  struct fp_port_del_arguments const* args = 
    (struct fp_port_del_arguments const*)req->data;

  /* Check if data plane exists. Remove the port from the data plane using
     the PID passed through the request arguments. */
  struct fp_dataplane* dp = fp_dataplane_lookup(args->name);
  if (dp)
    fp_dataplane_remove_port(dp, fp_dataplane_get_port(dp, (fp_port_id_t)args->pid), &rep->result);
  else
    rep->result = FP_BAD_DATAPLANE;

  return rep->result;
}

/* Returns the list of ports being used by the data plane. */
static int
fp_on_port_list(struct fp_request const* req, struct fp_reply* rep)
{
  fprintf(stderr, "[flowpath] list ports\n");
  /* Get the port list arguments. */
  struct fp_port_list_arguments const* args = 
    (struct fp_port_list_arguments const*)req->data;

  /* Check if data plane exists. */
  struct fp_dataplane* dp = fp_dataplane_lookup(args->name);
  if (dp) {
    size_t size = ((struct fp_chained_hash_table*)dp->ports.table)->size;
    struct fp_port** ports = fp_allocate_n(struct fp_port*, size);
    fp_dataplane_list_ports(dp, ports, &rep->result);
  }

  return rep->result;
}


static int
fp_on_request(struct fp_request const* req, struct fp_reply* rep)
{
  switch (req->kind) {
  case FP_DATPLANE_ADD:
    return fp_on_dataplane_add(req, rep); 
  
  case FP_DATPLANE_DEL:
    return fp_on_dataplane_del(req, rep);

  case FP_PORT_ADD:    
    return fp_on_port_add(req, rep);

  case FP_PORT_DEL:    
    return fp_on_port_del(req, rep);

  case FP_PORT_LIST:
    return fp_on_port_list(req, rep);
  
  case FP_DECODER_SET:
    fprintf(stderr, "set decoder: not implemented\n");
    return FP_FAILURE;
  
  case FP_TABLE_ADD:
    fprintf(stderr, "add table: not implemented\n");
    return FP_FAILURE;
  case FP_TABLE_DEL:
    fprintf(stderr, "delete table: not implemented\n");
    return FP_FAILURE;

  default:
    fprintf(stderr, "warning: operation not supported\n");
    return 1;
  }
}


/* Read and process an incoming flowmgr messgae. */
int
fp_mgr_incoming()
{
  /* Memory for the input and output message buffers. 

     TODO: If we're careful, we could actually manage
     these as static buffers, an not stack allocate
     them each time. There would be threading issues
     if this function is ever called from multiple
     threads. */
  char inbuf[FP_MESSAGE_LEN];
  char outbuf[FP_MESSAGE_LEN];

  /* Read into the buffer. */
  int inbytes = recv(mgr_, &inbuf, FP_MESSAGE_LEN, 0);
  fprintf(stderr, "[flowpath] message from flowmgr\n");
  if (inbytes < 0) {
    perror("fp_incoming/recv");
    return -1;    
  }

  /* Set up the request and reply. */
  struct fp_request const* req = (struct fp_request const*)inbuf;
  struct fp_reply*  rep = (struct fp_reply*)outbuf;
  rep->type = FP_REPLY;
  rep->kind = req->kind;
  rep->result = FP_OK;

  /* If it's a request, handle the request. */
  if (req->type == FP_REQUEST)
    rep->result = fp_on_request(req, rep);
  else
    rep->result = FP_BAD_REQUEST;

  fprintf(stderr, "[flowpath] finished request (%s)\n", fp_strerror(rep->result));

  /* Send the reply. 

     TODO: We may not  want to send the entire buffer.
     Or do we really care? */
  fprintf(stderr, "[flowpath] sending reply to flowmgr\n");
  int outbytes = send(mgr_, &outbuf, FP_MESSAGE_LEN, 0);
  if (outbytes <= 0) {
    if (outbytes < 0) {
      perror("fp_mgr_incoming/send");
      return -1;
    }
    else
      return 0;
  }

  return 1;
}

