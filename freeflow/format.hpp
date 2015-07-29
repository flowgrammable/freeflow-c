// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FREEFLOW_FORMAT_HPP
#define FREEFLOW_FORMAT_HPP

#include "contrib/cppformat/format.h"

#include <iosfwd>


namespace ff 
{

// The format facility.
using fmt::format;


// The print facility.
using fmt::print;
using fmt::fprintf;
using fmt::sprintf;


// Text formatters.
using fmt::bin; 
using fmt::oct;
using fmt::hex;
using fmt::pad;


// Import the Writer class as an alternative to stringstream.
using fmt::Writer;


} // namespace

#endif
