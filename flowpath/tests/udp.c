
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

  fp_error_t err = FP_OK;

  /* Create the UDP socket. */
  struct fp_device* dev = fp_udp_open(argv[1], &err);
  if (!dev)
    return -1;

  /* Build a port on the socket. */
  struct fp_port* port = fp_port_create(dev);

  /* Recieve a packet and then stop. */
  struct fp_packet* pkt = fp_port_recv_packet(port);
  printf("* got %d bytes\n", pkt->size);
  fp_port_drop_packet(port, pkt);

  /* Clean up. */
  fp_port_delete(port);
}
