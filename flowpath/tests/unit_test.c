//Context: Basic datatype of a dataplane (key: Key, pkt: Packet, output: PortID) 
//(struct np_context, ex: arrival.c; packet is in packet.c)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "arrival.h" // also includes types.h
#include "dataplane.h" // also includes types.h and util.h
#include "departure.h"
#include "packet.h" // contains np_context, np_packet and also includes types.h
#include "instruction.h" // don't think I need this in here
#include "port.h" // also includes util.h

struct np_debug_device {
  FILE* file;
};

void 
np_debug_write(void* device, struct np_packet* packet)
{
  struct np_debug_device* debug = (struct np_debug_device*)device;
  fprintf(debug->file, "packet of size: %d\n", packet->size);
}

int main(int argc, char* argv[]) {
	struct np_dataplane* dp;
	struct np_packet* packet;
	struct np_context* ctx = np_allocate(struct np_context);
	struct np_base_key* key;
	//in_port is of type np_port_id_t (which can be found in types.h), and is just
	// an unsigned 16 bit value. The same goes for in_phy_port
	//--- TESTING packet (within scope of CONTEXT objective)
	packet->data = 'C';
	packet->size = 0xffffffff; // size = -1  
	printf("[TEST 1 np_packet] data: %c, size: %d\n", packet->data, packet->size);
	packet->data = 'Z';
	packet->size = 0;
	printf("[TEST 2 np_packet] data: %c, size: %d\n", packet->data, packet->size);
	//--- TESTING arrival.c (covers the entire scope of ARRIVAL objective) ---
	key->inPort = 12345;
	key->in_phy_port = 1;
	// Inputs: dataplane, packet, in_port, in_phy_port
	// Expected outputs: np_context*, which contains a struct np_packet* packet, int time, and int out_port
	ctx = np_stage_arrival_exec(dp, packet, key->inPort, key->in_phy_port);
	printf("[TEST arrival.c] time: %d, out_port: %d\n", ctx->time, ctx->out_port);
	//--- TESTING context  (covers the CONTEXT objective, as does the arrival.c and departure.c test portions)---
	ctx->time = 20;
	ctx->out_port = 50;
	printf("[TEST np_context] time: %d, out_port: %d\n", ctx->time, ctx->out_port);
	
	//--- TESTING port (covers the PORT objective) ---
	struct np_port* cport;
	struct np_port dport;
	cport->id = 0;
	dport.id = 1;
	dp = np_make_dataplane(2); // make a dataplane with two ports
	np_add_port(dp, cport);
	np_add_port(dp, &dport);
	struct np_packet* test_pkt;
	test_pkt = dport.*recv(dport.*device); // testing the recv function of port
	np_port_output(&dport, ctx); // testing the send function of port
	//--- TESTING departure.c (covers the entire scope of EGRESS objective) ---
	// test 1: set ctx->out_port to NP_PORT_DROP. Should print out "Notice: dropping packet at departure"
	ctx->out_port = NP_PORT_DROP;
	np_stage_departure_exec(dp, ctx);
	// test 2 (happy path): set dp->nports = 10
	ctx = np_stage_arrival_exec(dp, packet, key->inPort, key->in_phy_port);
	ctx->out_port = 9;
	//dp->ports[ctx->out_port] blah blah blah
	np_stage_departure_exec(dp, ctx);
	// test 3 (error): set ctx->out_port  = 12
	ctx = np_stage_arrival_exec(dp, packet, key->inPort, key->in_phy_port);
	ctx->out_port = 12; // exceeds the total # of ports available in dp
	np_stage_departure_exec(dp, ctx);
}
