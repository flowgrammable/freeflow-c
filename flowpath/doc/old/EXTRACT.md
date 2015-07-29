
# Programmable Key Extraction (Prokex)

An extended assembly language for low level data plane key extraction. This language will allow for a packets extraction phase to be fully programmable. The instruction set will accomodate the instructions and actions required by the OpenFlow protocol (e.g., output packet, goto table, write metadata).

The extraction will be done in two phases

  1. Cursory Extraction: Extracts Sub-keys from a Packet and store into fields
  2. Table Extraction: Extracts Flow-keys from a Sub-key on a per-table basis

## Assumptions

**Matching:** All the matches that need to be made will be known before 
entering this phase of the pipeline.

## Registers

Read, write, and extract operations have to implied regions of memory: an input buffer and an output buffer. During initial key extraction, the input buffer is the packet, and the output buffer is the packet key. When matching a packet context against a flow table, the input buffer is the packet key and the output buffer is the flow key.

Offsets into these buffers have the range 0 .. 2^32 - 1. When an offset is negative it refers to a register. That identified register is indexed by 1's complement of the offset (e.g., -1 == 0x00, -2 == 0x01, etc.)

Registers will be allocated in physical hardware and in the Packet Context. Registers in a context will essentially travel with the packet through the pipeline. Packet Contexts will be (for now) stored in RAM, as will the registers associated with that packet. Access to these registers would be through offsets from the packet context address.

The registers are:

| Index | Name | Description                           |
| --- | --- | --- |
| 0     | $ip  | The input port of the packet          |
| 1     | $ipp | The input physical port of the packet |
| 2     | $tnl | The tunnel id of the packet           |
| 3     | $op  | The output port of the packet         |
| ...   |      | TODO: Other registers                 |
| 32    | $r0  | The first scratch register            |
| ...   |      |                                       |
| 63    | $r31 | The last scratch register             |

NOTE: The unified address space for offsets and registers is primarily intended to simplify the assembly programming language. We could define additional instructions that either operate on memory or on registers.

## Instructions

To allow for custom instructions built on an existing assembly language, we propose to double the size of an opcode (32 => 64 bits). We use the Highest Order Bit as a flag to indicate if the instruction is a Flow Assembly instruction (custom). The 2nd Highest Order Bit is a flag to indicate if the instruction is to be executed at a later time. The next 30 bits are padding to make alignment better, though this bitfield could be used for other needs as they arise during development. The rest of the instruction (low 32-bits) would be the opcode itself.

A Flow Assembly instruction would need to be expanded in terms of the underlying assembly language (e.g., a Write is really just a Store; a Read is just a Load). This translation would happen at compile time. Doing so allows us to dynamically target the host instruction set and execute code in the native language.

### Op Code
  - Form: `$OP $R0 $R1 ($R2)`

| Field | Size (Bit) |
| --- | --- |
Flow Assembly Instruction Flag | 1
Defer Execution Flag | 1
Padding | 30
Opcode | 32

In C this would look like:
```c
struct opcode_t{
  unsigned flow : 1;
  unsigned dfer : 1;
  unsigned pad  : 30;
  uint32_t code : 32;
} np_op;
```
Where the number after the `:` defines the bitfield width.

### Flow Assembly Instruction Set (Flowsembly)

Below is a working set of commands needed to facilitate a programmable data plane at the packet level.

- Extract from Packet -- Copies a specified sequence of bytes from the source offset in the input buffer ot the destination offset in the output buffer.
  - Form: `ext <src> <len> <dst>`
  The <src> offset may be name a register (e.g., `ext $ip 16 0`)

- Read from packet -- Read a sequence of bytes from the source
  buffer into the a register.
  - Form: `read <src> <len> <dst>`
  The <src> offset may name a register.

- Write to Packet -- Updates a meta-data or field item in a packet
  - Form: `write <dst> <len> <var>`
  The <dst> offset may name a register.

- Halt Execution -- Effectively moves Program Counter in a Packet Context to the end of its instruction set.
  - Form: `halt <id>`
  Where <id> identifies a packet context.

- If/Branch -- Evaluates an expression which determines the next instruction.
  - Form: `if <condition> <truePath> <falsePath>`

- Goto -- An element of any table (Flow Table, Metet, Port, Group)
  - Form: `goto <id>`

- Immediate -- Unsets the flag that would indicate an instructions execution is being delayed.
  - Form: `imdt <inst>`

- Deferred -- Sets a flag in an instruction, indicating its execution is being delayed.
  - Form: `dfer <instr>` 

- Push -- TODO

- Pop -- TODO

- Bind Port -- Add a port to the data plane
  - Form: `bnd <port>`

- Unbind Port -- Removes a port from the data plane (necessary?)
  - Form: `ubnd <port>`


TODO: If an offset is a non-scratch register, do we actually need to define the asssociated length? Probably not in the source language.

TODO: Add deferrable instructions. Potentially any?

## Controller instructions

We also need to provide instructions that modify abstraction tables during the running of the data plane. Eventually, these would be sent (and dynamically translated) from OpenFlow messages, but for now, we should support the ability to invoke them directly from within the data plane.

Broadly speaking, these commands are:

- Add/remove flow
- Add/remove group
- Add/remove meter
- Add/remove port
- Modify flow table (add/remove?)

## Additional Necessities
The follow is a running list of additional requirements needed to make the language usable

- Base Assembly Language -- A language that would allow for modification without violation of licenses. This would also  provide the more basic operations in an assembly language (memory operations, math, etc).
  - RISC V
  - MIPS
  - MMIX
  
- Compiler
