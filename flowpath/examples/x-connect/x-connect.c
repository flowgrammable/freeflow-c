
#include <string.h>

// general dataplane types:
#include "dataplane.h"
#include "pipeline.h"
#include "packet.h"
#include "port.h"
#include "types.h"

// local headers
#include "util.h"
#include "key.h"  // definition of Key struct

#define DEBUG
#ifdef DEBUG
#include <stdio.h>  // temporary for debug printf's
#endif

static bool LittleEndian;

// Simple example stage for cross-connect.  First iteration, all packets
// in port 1 go out port 2, and all packets in port 2 go out port 1.
// Arguments:
// - dp: pointer to dataplane configuration object
// - s: pointer to stage itself
void nps_exec(struct fp_dataplane* dp, struct fp_context* ctx) {
	// Ensure Context is valid:
	if (ctx == NULL) {
#ifdef DEBUG
		printf("WARNING: xConnect recieved NULL Context!\n");
#endif
	}

	// Make output decision based on input logical port:
	if (ctx->key->inPort == 0) {
		ctx->out_port = 1;
	}
	else if (ctx->key->inPort == 1) {
		ctx->out_port = 0;
	}
	else {
#ifdef DEBUG
		printf("WARNING: xConnect input logical port != {0,1}!\n");
#endif
		ctx->out_port = FP_PORT_DROP;  // a better drop mechanism would be nice
	}
}


// No init is needed for cross-connect, however to demonstrate the
// interface, a simple endianness detector is used to set a static variable.
void nps_init() {
	volatile u16 endianTest = 0xABCD;
	u8 end = *((volatile u8*)(&endianTest));
	LittleEndian = (end == 0xCD);
#ifdef DEBUG
	printf("Initialized x-connect.\n");
#endif
}

