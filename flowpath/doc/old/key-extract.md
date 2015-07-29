
# Programmable extraction of keys from packets

## Flow tables

In our model a flow table is an abstract associative data type;
it associates a program to be executed with a key that summarizes
some aspects of the packet. 


Each flow table in an SDN program is essentially defined as a
list of conditions that match values in a packet. For example
a table might be configured with the following rules:

    in_port = *, eth.type == 0x0800, ipv4.src = 192.168.0.*
    in_port = *, eth.type == 0x86dd

The fist matches all traffic sent from hosts with an IPv4 address
that matches the pattern "192.168.0.*". The second rule matches
all IPv6 packets.

Note that the `eth.type` condition is necessary to ensure that
the subsequent condition is not evaluated in a way that would
result in undefined behavior. That is, asking what the IPv4 source
address of an ICMP packet is logically unsound. Furthermore, we
note that with a higher level programming language, we can 
infer these kinds of "branching" conditions given a simple list
of protocol/field names and values. That inference is beyond the
scope of these notes.

These rules could be installed in a single table, or in different
tables. Regardless of their source, taken together they fully
describe the input needs of the packet processing pipeline. For any
flow table to execute any of these matches, we need to extract all
of this information 

