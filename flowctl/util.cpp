// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "util.hpp"

#include "freeflow/unix.hpp"

#include <iostream>


using namespace ff;


// The global configuration.
extern Config cfg;


// Check that the the number of given arguments (argc) 
// matches the number expected (arity).
bool
check_argument_arity(int argc, int arity)
{
  if (argc != arity) {
    char const* str;
    if (argc < arity)
      str = "few";
    if (argc > arity)
      str = "many";
    print(std::cerr, "error: too {} arguments "
                     "(expected {}, got {})\n", str, arity, argc);
    return false;
  }
  return true;
}


// Returns true if argc is at lest arity, if not greater. Emits
// an error message when false.
bool
check_argument_arity_at_least(int argc, int arity)
{
  if (argc < arity) {
    print(std::cerr, "error: too few arguments "
                     "(expected at least {}, got {})\n", arity, argc);
    return false;
  }
  return true;
}


namespace
{

// Return the command functoin to be invoked for the
// given target. Emits an error if no such target exists.
Command 
lookup_command(Command_map& map, char const* tgt)
{
  auto iter = map.find(tgt);
  if (iter == map.end()) {
    print(std::cerr, "error: invalid targt\n");
    return nullptr;
  }
  return iter->second;
}


} // namespace


// A helper function for each of the commands below. This checks
// that there is at least one argument (the target), and the looks
// up an action corresponding to that target. Returns -1 if no
// action can be invoked for any reason.
int
execute_command(Command_map& map, int argc, char** argv)
{
  // FIXME: Do better with diagnostics.
  if (check_argument_arity_at_least(argc, 2))
    if (Command cmd = lookup_command(map, argv[1]))
      return cmd(argc - 1, argv + 1);
  return -1;
}


// Send a command to nomg by connecting, transmitting and
// closing. Also, interpret any result.
//
// Note that this blocks until we receive the entire
// result of the communication.
int 
send_command(std::string const& str, std::string& result)
{
  Unix_socket_address addr = cfg.socket;

  int fd = client_socket(addr);
  if (fd < 0) {
    std::cerr << std::strerror(errno) << '\n';
    return -1;
  }

  // Send the command.
  if (send(fd, str.data(), str.size()) < 0) {
    std::cerr << "error: " << std::strerror(errno) << '\n';
    return -1;
  }

  // And consume data until the socket is closed.
  constexpr int len = 1 << 16;
  char buf[len];
  int bytes = recv(fd, buf);
  if (bytes < 0) {
    std::cerr << "error: " << std::strerror(errno) << '\n';
    return -1;
  }
  if (bytes > 0) {
    result.assign(buf, buf + bytes);

    // Look at the result. If we it's not "ok", then print
    // the received error.
    //
    // TODO: Pass the result back as JSON.s
    if (result != "ok")
      std::cerr << result << '\n';
  }

  // Close the connection.
  close(fd);
  return 0;
}


// See above. The resulting data is discarded.
int 
send_command(std::string const& str)
{
  std::string buf;
  return send_command(str, buf);
}
