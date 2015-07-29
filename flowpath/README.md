
## Configuration

You may have the following packages installed:
- pcap
- netmap
Note that without any of those packeges installed only the
main library is built. By default, building with pcap is
enabled, and netmap is not.

Configuration can be changed using ccmake or the CMake GUI. Both
pcap and netmap builds can be enabled/disabled in the configuration
via the NOPROTO_USE_PCAP and NOPROTO_USE_NETMAP options. If netmap 
is installed in a non-standard location, you should explicitly set 
its install path in the NETMAP_SOURCE_DIR variable.

Configuration and building is done in the usual way:

    cmake ..
    ccmake . # Optionally modify the configuration
    make


## Overview

The noproto data plane defines a reconfigurable and programmable
packet processing pipeline. 

The project currently builds two artifacts:

- libnoproto implements the pipeline
- noproto-sim is a program that defines a simple pcap -> stdout
  pipeline that writes prints the size of each packet in
  the program.

To build the software, you must have libpcap installed and in
your search paths.

## Pipeline simulator

The noproto-sim program configures a simple virtual data plane
that connects a pcap input port to a simple debugging port.
Note that both ports are virtual ports.

Running the program is simple:

    noproto-sim input.pcap

This queues each packet in the input file and runs it through
the packet processing pipeline.

Packet traces can be found from the following sites:
  - https://www.bro.org/community/traces.html

TODO: It would be great if the input program also took a matching
specification and applied that as a filter to the packets in
the trace. More complicated flows could also be described.


## Data plane architecture

The data plane is comprised of a number of different abstractions.
The are:
  - the data plane itself
  - ports (input and output)
  - queues
  - buffers 
  - flow tables
  - flow entries
  - instructions
  - actions
  - group actions
  - meters
  - meter bands
  - the processing pipeline

## Pipeline architecture

The packet processing pipeline is comprised of pipeline stages,
each of which contains one or more input queues and is connected
to zero or more other stages.

In general, the following stages are:
  - packet arrival
  - key extraction
  - table choice
  - flow selection
  - instruction execution
  - group action application
  - flow metering
  - packet departure

Each stage is defined within a separate module.
