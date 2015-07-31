/* To get defns of NI_MAXSERV and NI_MAXHOST */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#include "device.hpp"

// Returns the name of the socket address family given.
char const*
family_name(int family)
{
  switch (family) {
    case AF_PACKET:
      return "AF_PACKET";
    case AF_INET:
      return "AF_INET";
    case AF_INET6:
      return "AF_INET6";    
  }
  return "???";
}


// Creates a list of devices and if they are active it reports their 
// name, device address, address family, socket address(es), flags, and
// any usable packet information. Based on the manpages example for 
// getifaddrs(3).
//
// FIXME: Improve upon the device interface implementation to make this
// less C-like.
int
main(int argc, char** argv)
{
  #if 0
  // The device list.
  flowmgr::device::list lst;
  for (auto iter = lst.begin(); iter != lst.end(); ++iter){
    // Report device and address family names.
    fprintf(stderr, "%-8s\n", iter->name());
    for (auto addr_iter = iter->address_begin(); addr_iter != iter->address_end(); ++addr_iter) {

    }
    if (fam == AF_INET || fam == AF_INET6) {
      char host[NI_MAXHOST];
      size_t size = (fam == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
      // Get the socket address hostname
      int res = getnameinfo(iter->address(), size, host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (res) {
        fprintf(stderr, "error - getnameinfo() : %s\n", gai_strerror(res));
        continue;
      }
      else {        
        // Report flags for the interface.
        fprintf(stderr, " flags: %d\n", iter->flags());
        // Report the socket address hostname
        fprintf(stderr, " %s %s\n", (fam == AF_INET ? "inet" : "inet6"), host);
        if (fam == AF_INET) {
          char mask[INET_ADDRSTRLEN];
          char broad[INET_ADDRSTRLEN];
          // Report the netmask.
          inet_ntop(fam, iter->netmask(), mask, INET_ADDRSTRLEN);
          // Report the broadcast address.
          inet_ntop(fam, iter->broad_address(), broad, INET_ADDRSTRLEN);
          fprintf(stderr, " netmask: %s\tbroadcast: %s\n", mask, broad);
        }        
      }
    }
    else if (fam == AF_PACKET && iter->data()) {
      // Get the packet information from the interface.
      struct rtnl_link_stats* stats = (rtnl_link_stats*)iter->data();
      // Report packet information.
      fprintf(stderr, "\ttx packets = %8u; rx packes = %8u\n"
                      "\ttx bytes   = %8u; rx bytes  = %8u\n",
        stats->tx_packets, stats->rx_packets, stats->tx_bytes, stats->rx_bytes);
    }
  }

  #endif

  return 0;
}