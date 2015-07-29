// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWGR_DISPATCH_HPP
#define FLOWGR_DISPATCH_HPP

#include "freeflow/async.hpp"
#include "freeflow/json.hpp"

#include <string>
#include <unordered_map>

// This module provides utilities for building dispatch tables.


// A dispatch target is a function that takes an I/O handler
// and an argument map and applies some logic.
using Dispatch_target = void (*)(ff::Io_handler&, ff::json::Map const&);


// A dispatch table associates command names with dispatch targets.
using Dispatch_table = std::unordered_map<std::string, Dispatch_target>;


#endif
