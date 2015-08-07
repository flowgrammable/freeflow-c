// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef TG_FRAME_HPP
#define TG_FRAME_HPP

#include <cstring>


// The traffic generator namespace.
namespace Traffic
{

// The frame namespace.
namespace Frame
{


using Ipv4_address = unsigned char[4];  // An IP v4 address.
using Ipv6_address = unsigned char[16]; // An IP v6 address.
using Mac_address = unsigned char [6];  // A MAC address.


// Encapsulates the possible choices for a header source and destination
// addresses.
union Address
{
  Ipv4_address ipv4;
  Ipv6_address ipv6;
  Mac_address   mac;
};


// The ethernet frame class. Contains an ethernet header and trailer, as well
// as the payload.
class Ethernet
{
  using Preamble      = unsigned char[7];   // Ethernet frame Preamble.
  using Delimiter     = unsigned char;      // Frame delimiter.
  using Optional      = unsigned char[4];   // Optional tag.
  using Type          = unsigned char[2];   // EtherType or 802.3 Length.
  using Payload       = unsigned char*;     // Frame Payload.
  using FrameCheck    = unsigned char[4];   // Frame Check or 32bit CRC.
  using InterFrameGap = unsigned char[12];  // Gap between ethernet frames.
public:

  // Ethernet header struct. Contains the preamble, delimiter, destination &
  // source addresses, optional tags, and the ethertype (or length).
  struct Header
  {
    Preamble  pream = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
    Delimiter delim = { 0xAB };
    Address   dst;
    Address   src;
    Optional  opts;
    Type      type;

    Header(Address& destination, Address& source, Optional& optional, Type& t)
    { 
      std::memcpy(dst.mac, destination.mac, sizeof(destination.mac));
      std::memcpy(src.mac, source.mac, sizeof(source.mac));
      std::memcpy(opts, optional, sizeof(optional));
      std::memcpy(type, t, sizeof(t));
    }
  };


  // Ethernet trailer struct. Contains the framecheck or CRC and inter-frame gap.
  struct Trailer
  {
    FrameCheck    chk;
    InterFrameGap gap;

    Trailer(FrameCheck& check)
    { 
      std::memcpy(chk, check, sizeof(check));
      std::memset(gap, 0, 12 * sizeof(unsigned char));
    }
  };

  // Should we allow for a default constructed ethernet frame?
  Ethernet() = delete;

  Ethernet(Header& hdr, Payload& pay, Trailer& trl)
    : header(hdr), data(pay), trailer(trl)
  {
    length = sizeof(hdr) + sizeof(pay) + sizeof(trl);
  }

  ~Ethernet()
  { 
    delete data;
  }


private:
  Header  header;
  Payload data;
  Trailer trailer;
  size_t  length;
};

} // end namespace frame

} // end namespace traffic

#endif