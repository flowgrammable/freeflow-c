// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "port_udp.h"
#include "packet.h"
#include "port.h"

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


const int ADDR_STR_LEN = 256;
const int PORT_STR_LEN = 16;


/* The initial size of buffers allocated to read UDP ports.
   If the message is larger than this value. */
const int INIT_BUF_SIZE = 4096;


/* Parse the address and port strings from the URI spec. */
static int
parse_udp_name(char const* str, char** addr, char** port, fp_error_t* err)
{
  int len = strlen(str);
  char const* ptr = strchr(str, ':');
  if (ptr) {
    /* If the address was too long, fail. */
    int off = ptr - str;
    if (ADDR_STR_LEN < off) {
      *err = fp_system_error(EINVAL);
      return -1;
    }
    
    /* Copy out the address and null-terminate it. */
    strncpy(*addr, str, off);
    addr[off] = 0;
    
    /* Bump the string pointer so that it points past
       the just-read address. */
    str += off + 1;
    len -= off;
  } else {
    addr = NULL;
  }
  
  /* If the port string was too long, fail. */
  if (PORT_STR_LEN < len) {
    *err = fp_system_error(EINVAL);
    return -1;
  }
  strncpy(*port, str, len);
  return 0;
}


/* Sets the address to the value parsed by the string. */
static int
parse_ipv4_addr(char const* str, struct sockaddr_in* addr, fp_error_t* err)
{
  if (inet_pton(AF_INET, str, addr) < 0) {
    *err = fp_get_system_error();
    return -1;
  }
  return 0;
}


/* Sets the address family and IP address. */
static void
set_ipv4_addr(struct sockaddr_in* addr, uint32_t ip)
{
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = ip;
}


/* The virtual table for all UDP devices. */
static struct fp_device_vtbl udp_vtbl = {
  .recv  = fp_udp_recv,
  .send  = fp_udp_send,
  .drop  = fp_udp_drop,
  .close = fp_udp_close
};


/* Open the UDP port and give it the the given port number. 

   The address argument specifies the hostname and port number to 
   which the socket is bound. Valid formats for this string are:

      <address>:<port>
      <port>

   When only the port is given, the socket is bound to INADDR_ANY. */
struct fp_device*
fp_udp_open(const char* address, fp_error_t* err)
{
  /* Parse the port name. */
  char addr_buf[ADDR_STR_LEN];
  char port_buf[PORT_STR_LEN];
  char* addr = addr_buf;
  char* portno = port_buf;
  if (parse_udp_name(address, &addr, &portno, err) < 0) {
    *err = fp_get_system_error();
    return NULL;
  }

  /* Get an IP address. 
     TODO: Support IPv6. */
  struct sockaddr_in sa;
  if (addr && (parse_ipv4_addr(addr, &sa, err) < 0))
      return NULL;
  else
    set_ipv4_addr(&sa, INADDR_ANY);

  /* Get a port number. 
     TODO: Validate the conversion. */
  sa.sin_port = htons(atoi(portno));

  return fp_udp_open_sockaddr(&sa, err);
}


/* Open a UDP device on the given port. The port number
   must be in host byte order. The socket is bound to
   INADDR_ANY. */
struct fp_device*
fp_udp_open_port(short port, fp_error_t* err)
{
  struct sockaddr_in sa;
  set_ipv4_addr(&sa, INADDR_ANY);
  sa.sin_port = htons(port);
  return fp_udp_open_sockaddr(&sa, err);
}


/* Open a UDP device for the given socket address. */
struct fp_device*
fp_udp_open_sockaddr(struct sockaddr_in* addr, fp_error_t* err)
{
  /* Create the device and initialize its peer address. */
  struct fp_udp_device* dev = fp_allocate(struct fp_udp_device);
  dev->base.vtbl = &udp_vtbl;
  memcpy(&dev->addr, addr, sizeof(struct sockaddr_in));

  /* Open the socket. */
  dev->fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (dev->fd < 0) {
    *err = fp_get_system_error();
    return NULL;
  }

  /* Bind the socket to its address. */
  if (bind(dev->fd, (struct sockaddr*)&dev->addr, sizeof(dev->addr)) < 0) {
    *err = fp_get_system_error();
    return NULL;
  }

  return (struct fp_device*)dev;
}


/* Close and destroy the UDP interface. */
void
fp_udp_close(struct fp_device* device)
{
  struct fp_udp_device* dev = (struct fp_udp_device*)device;
  close(dev->fd);
  fp_deallocate(dev);
}


/* Read from a UDP port. */
struct fp_packet*
fp_udp_recv(struct fp_device* device)
{
  struct fp_udp_device* dev = (struct fp_udp_device*)device;
  
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  /* Receive the message. */
  char buf[INIT_BUF_SIZE];
  int bytes = recvfrom(dev->fd, buf, INIT_BUF_SIZE, 0, 
                       (struct sockaddr*)&addr, &len);
  if (bytes < 0)
    return NULL;
  
  /* TODO: What should we do when the peer closes? 
     Send a 0-byte packet through the pipeline? */
  if (bytes == 0) 
    return NULL;

  /* Copy the buffer so that we guarantee that we don't
     accidentally overwrite it when we have multiple 
     readers. 

     TODO: We should probably have a better buffer management
     framework so that we don't have to copy each time we
     create a packet. */
  unsigned char* data = fp_allocate_n(unsigned char, bytes);
  memcpy(data, buf, bytes);

  /* Allocate a noproto packet. */
  /* TODO: set timestamp */
  return fp_packet_create(data, bytes, 0, NULL, FP_BUF_ALLOC);
}


/* Write to a UDP port and free the packet. */
int
fp_udp_send(struct fp_device* device, struct fp_packet* pkt)
{
  struct fp_udp_device* dev = (struct fp_udp_device*)device;

  int bytes = sendto(dev->fd, pkt->data, pkt->size, 0,
                     (struct sockaddr*)&dev->addr, sizeof(dev->addr));
  if (bytes < 0)
    return bytes;

  /* TODO: What do we do if we send 0 bytes? */
  if (bytes == 0)
    return 0;

  fp_deallocate(pkt->data);
  fp_packet_delete(pkt);
  return bytes;
}


/* Drop a packet, releasing its resources. */
void
fp_udp_drop(struct fp_device* device, struct fp_packet* pkt)
{
  fp_deallocate(pkt->data);
  fp_packet_delete(pkt);
}
