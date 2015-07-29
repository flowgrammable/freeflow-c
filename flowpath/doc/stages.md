

## Pipeline stages:

A typical stage is modeled as a stateful function with this
form:

    stage(Context) -> Context

That is, it is a function that takes a context and produces
a new context. By "stateful" we mean that the function can
modify the global state of the data plane, and also possibly
some local state specific to that function.

From an implementation perspective, these functions will not
generally consume and produce new packet contexts. Instead
these functions will operate directly on the context, and
simply return the same object that it was given.

Global data plane state is just that: a pointer to the global
data plane object. Local state can be modeled by local static
variables if needed.


## Built-in Pipeline stages:

There are two built-in stages: ingress and egress.  These
stages are special in that they manage the packet Context,
the handle to the packet and metadata as it flows through
the pipeline.

### Ingress:

Ingress is the entry-point to the pipeline.  The signature 
for ingress is:

    ingress(Packet, Arrival) -> Context

That is, ingress is given a packet and some arrival data (in
port, tunnel, time stamp, etc).  The ingress stage is
is unique in that it allocates a new packet Context.  All other
stages will simpliy modify this Context as processing is
performed on the packet.

Note that the ingress stage does not poll an associated
port. The data plane's main loop is responsible for polling,
and constructing packets, and giving them to the pipeline
for processing.

#### Ingress: Allocating a Context

Because the Context has elements which are configurable in size (Key, SubKey, Action Set, etc.), the size of these elements must be reported
to the Ingress stage after pipeline re-definition.  This allows Ingress
to allocate the proper size for a Context without requring it know any
specifics about the configurable-length structures.

### Egress

The signature for egress is:

    egress(Context) -> void

The primary function of the egress stage is to translate
the packet referred to send the packet out of one (or more)
port(s). The packet, having been sent, no longer requires
a context, so the context is destroyed.


## Modular Pipeline stages:

Modular pipeline stages are re-definable, providing run-time 
programmability to the the data plane.  The heart of the pipeline
and its functionality is defined through these stages.

    stage(Context) -> Context

When processing packets, all of these stages interact with the
fundamental structure a packet Context.  The packet context contains
any packet metadata including the Key.

The following stages are examples of the flexibility provided by 
a modular stage framework.

### Decoding stage

Packet decoding typically occurs early in the pipeline, often
immediately after ingress, and forms a Key containing relevant fields
from the packet (see packet Key below).


### Table selection and matching


### Instruction execution


### Metering


### Group actions


### Action application


### Enryption


### Compression


### Fragmentation




## General Data Plane Types

### Packet Context

The packet context is the master structure with respect to a
packet.  The context owns the packet and all metadata about
it.  The context is allocated and initialized by the ingress stage.
The packet context structure is defined as:

    struct Context {
      // Known-length elements:
      Packet pkt;  // handle to packet itself
      Time ts;  // timestamp of packet arrival (should be optional?)
      TableID nextTable;  // table id of next table to perform lookup
      
      // Actions (hack):
      PortID out;  // id of logical port to output packet.

      // Configurable-length elements:
      Key key;  // contains all Fields and Metadata needed for lookups
      SubKey subkey;  // Max(table subkey size); subset of Key for lookup
    }

Elements listed in the packet Context are required.  Optional fields
are contained within the configurable-length Key and SubKey structures.
The Key structure contains all fields and metatdata (if desired) which
are used to make processing decisions.

The configurable-length structures are only accessible by the
modular-stages.  The data plane, ingress, and egress stages would not
know how to interpret these structures as they are configurable.
The only exception includes the required fields of the Key structure
(see: Packet Key).

Todo:
To simplify the Context for milestone 1, the output action is limited
to a single port, specified by the PortID (output logical port) variable.
Actions need to be defined...


#### Packet Key

The packet key contains all fields of packet that was
extracted during packet decode, which will be needed for lookups.
The packet key potentially contains metadata fields which are used
for lookups, but originate from processing stages and not packet fields.
The packet key structure is defined as:

    struct Key {
      // Required elements:
      PhyPort inPhy;  // the physical port which the packet arrived on
      PortID inPort;  // the logical (w.r.t. dp) port which packet arrived

      // Configurable elements:
      Fields field;     // configurable number of extracted fields
      Fields metadata;  // configurable number of non-extracted fields
    }

As of right now, the only required elements within the key is the input
physical and logical port IDs.  The field and metadata elements may
have zero or more entries, as required by the pipeline.

Upon changing the Key through pipeline re-definition, the data plane
will have to notify the ingress stage to allocate the appropriate amount
of space for the Key, in the Context.

Note: The Key may be optimized for space, access latency, or ease of
building SubKey.

#### Subset Key (SubKey) for Table Lookup

The sub-key element within the packet context is simply a location
for building the table's key from a subset of the fields in the master
Key.

Note: To simplify things for milestone 1, we only build one SubKey
at a time; thus, the size of the SubKey must be sufficient for the
largest key required by a table.

### Port

The port data structure provides a uniform interface for ingress
and egress stages.  The Port structure is defined as:

    struct Port {
      // PCAP-like Interface:
      int send(void* dev, Packet* pkt);  // used by egress to queue packet
      Packet* recv(void* dev);  // used by main loop to get next packet
      
      // Port Configuration:
      PortID id;	// Logical Port ID used by the Data Plane
      EthAddr addr;	 // Ethernet MAC address used by the port
      bool up;  // used to up/down port
      bool no_recv;
      bool no_fwd;
      // QueuePolicy policy;  // drop policy of port for ingress?
      
      // Port Information:
      PhyPort phyID;  // Physical Port ID used by the Data Plane
      char name[];    // Name of Linux file handle
      
      // Port Status:
      // negotiated speed?
      // other physical properties: link down, duplex, etc?
    }

Currently, the send() and recieve() interface is provided to
simplify the interface.  Egress will use the send() interface to output
a packet through a port.  The main loop will use the receive() interface
to get the next packet and pass to the Ingress stage.


