
#include "port.h"
#include "port_udp.h"
#include "packet.h"

#include <stdio.h>

int 
main(int argc, char** argv)
{
  if (argc != 2) {
    fprintf(stderr, "usage: test-udp (<host>:<port>|<port>\n");
    return -1;
  }

  np_error_t err = NP_OK;

  /* Create the UDP socket. */
  struct np_device* dev = np_udp_open(argv[1], &err);
  if (!dev)
    return -1;

  /* Build a port on the socket. */
  struct np_port* port = np_port_create(dev);

  /* Recieve a packet and then stop. */
  struct np_packet* pkt = np_port_recv_packet(port);
  printf("* got %d bytes\n", pkt->size);
  np_port_drop_packet(port, pkt);

  /* Clean up. */
  np_port_delete(port);
}
