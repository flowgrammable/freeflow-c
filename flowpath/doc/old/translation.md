
What does it mean to configure a data plane?

It means to provide a configuration file that instantiates the
pipeline and loads the programs that are be executed by its different
stages.


Where does this configuration file from?

Presumably it is provided by a central controller, along with the
programs that are executed by the various processing stages.


What are the programs?

Executable sequences of instructions. Exactly what language this
is, or what format they are in is unclear. What is clear is that
we have a limited set of instructions that can inspect and
manipulate packets, and also govern its flow through the pipeline.

Note that the instruction set must be a fairly complete ISA.
Packet introspection requires basic ALU operations for packet
decoding and length checks. It also requires branching in order
to extract the key, and if we have completely general extraction
programs, then we also need jumps (loops). Additionally, we read
and write to the packet, to its surrounding context, and to
various registers to communicate with the dataplane/kernel (i.e.,
system calls).

There are essentially two options for this instruction set:
byte code or native code.

Ultimately, we want a byte code that encodes an intermediate
representation.
