// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "socket.hpp"

#include <netinet/in.h>

namespace ff
{

// Create an unbound stream socket.
int
stream_socket(Family af, int proto)
{
  return ::socket(af, SOCK_STREAM, proto);
}


// Create an unbound datagram socket.
int
datagram_socket(Family af, int proto)
{
  return ::socket(af, SOCK_DGRAM, proto);
}


} // namespace ff
