
## Pipeline composition

A pipeline is composed of stages, starting with ingress and
terminating with egress. There are several different ways
of linking stages, but the simplest is simply a function
that calls each stage in sequence. For example, the pipeline
for a simple forwarding wire might be:

    def wire(Packet p, Arrival a) {
      Context c = ingress(p, a);
      // Set the output port for c.
      egress(c);
    }

Here, `wire` is a function representing the pipeline. It is
invoked for each packet that arrives in the system. These are
the same arguments for the ingress stage.

(Note: I don't know if, in the `wire` pipeline, the ingress
stage is responsible for setting the output port or there
is an intermediate stage. You could define this is a trivial
table lookup, and action set:

    wire[internal.in_port == 1] { output port 2; }
    wire[internal.in_port == 2] { output port 1; }

But that's a little more work than a simple if statement.
Hmmm... maybe there's an opportunity to optimize tables as 
if statements or jump tables?)


The pipeline calls `ingress` to create the context, performs
some custom action to set the output port, and then calls 
`egress` to send it to an output port. 

A more complex pipeline that includes a single table might
be defined thusly:

    def wire(Packet p, Arrival a) {
      Context c = ingress(p, a);

      c = decode(c);  // Decode the packet
      c = match(c);   // Lookup an entry in the table
      c = apply(c);   // Apply any actions to the packt

      egress(c);
    }

The first stage after ingress is the decode phase. This
extracts all of the information from the packet needed
for table matching and the set of actions supported by
the data plane. 

After information has been extracted, we match the packet
in the table in order to find a corresponding flow entry,
and then apply the actions indicated by that flow (e.g., 
writing ECN fields, decrementing TTL, etc., setting the
output port). The packet is then emitted to its correspind
port.

This is effectively an OpenFlow 1.0 pipeline.

(Note: If matching does not return a flow table entry, then we 
need to drop the packet unless a default rule is defined.)

