
## Introduction

We want a fully programmable, protocol independent data plane based on the 
architecture described by flowsim. That architecture defines a pipleline 
of processing elements or stages, each of which performs actions on
a packet as it flows through a switch.

By protocol independent, we mean that the data plane should have as little
knowledge of networking protocols as possible. That is, instead of hard
coding knowledge of protocols and their layering, the data plane should
be externally programmed to recognize those protocols, their fields,
and the packet structure itself.

The flowsim architecture defines a (reconfigurable) sequence of processing
stages. Each stage defines a set of operations on packets. Those stages
include:

- Key extraction
- Flow matching
- Execute actions
- Group actions
- Metering actions

Each phase acts on a packet context. Based on the kind of information
in the packet (source address, destination address, protocol type, etc.), 
different actions can be taken. For example, an administrator might route 
all HTTP traffic to a virtual LAN. The kinds of information extracted
from packets and the kinds of actions on packets require specific
knowledge of protocols and their layering. To do this, each stage must
be able to execute small programs to perform their actions.

Some questions:

- Why is protocol independence a good thing? 
- What are the costs of protocol dependence? 
- How do we estimate the costs of protocol independence?

## Solution overview

In order for the data plane to execute programs, we need to define a
programming language to execute. We have a requirement that the language 
executed be able to be translated to a native architecture to achieve
optimal performance. We have the following options:

- AST (abstract syntax tree) interpretation
- IS (instruction set) interpretation
- native code execution

An AST-based interpreter does not readily map to hardware, and so
we do not opt to execute this within the data plane. However, this
would be an effective approach for writing the kinds of programs
we want to execute (see below).

Native code execution may be difficult. Because the program runs in
the kernel, we would have to execute these programs either in supervisor
mode, which can be dangerous. We could run the programs in protected
mode, but it's not clear exactly how that works. So there's a learning
curve involved.

This also limits our approach. If we had a native execution environment,
then we could not extend the ISA, since that potentially requires modifying
the hardware in order to execute it.

An IS-based interpreter is the best option for now. We can define
a simple interpreter of an existing and extensible IS. For this,
we choose RISC-V since it is extensible. Also, we aren't restricted
by existing hardware for the kinds of instructions that we want.
We could opt to interpret new instructions as we need them.

Choosing an IS-based interpreter also does not preclude subsequent
native implementations. In fact, the data plane module that implement
each stage could have a native architecture equivalent that would
set up the appropriate protection modes and simply jump to the given
instruction.

We further note that RISC-V can be emitted by LLVM. This gives us
a low-level AST for specifying executable programs for the data
plane, so we get the best of both worlds later on. Even better, we
can define high-level languages that emit LLVM for subsequent
optimization later... so the best of all worlds?

## Instructions

Because the data plane has no native understanding of protocols,
a program must be able to express the full range of options required
to parse a protocol packets. At a minimum, this includes a basic
set of ALU operations, branches, jumps, stores, and loads.

We have the following instruction requirements:

- A program must be able to extract packet data into a key buffer
- A program must be able to indicate its transition to a new stage
- A program must be able to refer to information in its context

There is a design question here:

  Must we extend an IS to define these instructions, or can we
  simply define the data plane as a reconfigurable virtual
  machine using, say LLVM, and then transate LLVM code to
  an existing ISA.

The former option initially seems easer. The second option is far
more flexible.


## Stages and processors

Each stage really defines its own virtual machine. For example,
the key extraction stage does not modify the packet structure
or content, we simply read protocol field.s


## Switching programs

A "switching program" is the complete set of instructions for a
data plane and its processing stages.

What is the program translation model?

How do we load a switching program?

