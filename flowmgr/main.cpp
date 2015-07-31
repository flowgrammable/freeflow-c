// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "control.hpp"
#include "switch.hpp"

#include "freeflow/ip.hpp"
#include "freeflow/poll.hpp"

#include <cerrno>
#include <cstring>
#include <csignal>

#include <unordered_map>



using namespace ff;


// The configuration object stores config data
//
// TODO: Move this into a separate module.
struct Config
{
  std::string control_socket   = "/tmp/control-socket";
  std::string switch_socket = "/tmp/switch-socket";
};


// Disable command line parsing becaues the freeflow CLI
// parser relies (through a 3rd party library) on std::regex,
// which is broken in GCC-4.8.
#if 0
// Create the command line options for the flowmgr server.
cli::Options
make_options()
{
  cli::Options options("flowmgr");
  options.add_options()
    ("flowctl-socket",
        "path to the flowctl server socket", cli::value<std::string>())
    ("flowpath-socket",
        "path to the flowpath server port", cli::value<std::string>());
  return options;
}


// Parse the command line options.
bool
parse_command_line(Config& config, int argc, char** argv)
{
  cli::Options options = make_options();
  try {
    options.parse(argc, argv);

    // Get the flowctl socket path.
    if (options.count("flowctl-socket"))
      config.flowctl_socket = options["flowctl-socket"].as<std::string>();

    // Get the flowpath socket path.
    if (options.count("flowpath-socket"))
      config.flowpath_socket = options["flowpath-socket"].as<std::string>();

  } catch (std::exception& err) {
    std::cerr << err.what() << '\n';
    std::cerr << options.help() << '\n';
    return false;
  }
  return true;
}
#endif

// Global running flag to allow for a graceful shutdown.
bool running;

// The global io handler map.
Io_map ios;

// The poll set used in the main loop.
Poll_set ps;


// Signal handler to terminate polling loop in main.
void
signal_hanlder(int)
{
  if (running)
    running = false;
}

// Register an I/O handler.
void
register_handler(Io_handler* io)
{
  ios.insert({io->fd(), io});
  ps.emplace_back(io->fd(), POLLIN);
}


// Unregister an I/O handler.
void
unregister_handler(Io_handler const* io)
{
  ios.erase(io->fd());
  ps.erase(ps.file(io->fd()));
  delete io;
}


// Process I/O events for the polled file. If the handler
// indicates closure, insert the object into remove set.
void
process_events(Poll_file& pf, Io_set& rm)
{
  Io_handler* io = ios.find(pf.fd)->second;
  if (pf.can_read())
    if (!io->on_input())
      rm.insert(io);
}

// Setup signal handlers to allow for a more graceful shutdown.
inline void
set_signal_masks()
{  
  signal(SIGINT, signal_hanlder);
  signal(SIGKILL, signal_hanlder);
  signal(SIGHUP, signal_hanlder);
  signal(SIGTERM, signal_hanlder);
}


int 
main(int argc, char** argv)
{
  Config config;

  // FIXME: Re-enable command line parsing.
  // if (!parse_command_line(config, argc, argv))
  //   return -1;

  // Setup kill signal handler.
  set_signal_masks();

  // Unlink previous sockets.
  unlink(config.control_socket);
  unlink(config.switch_socket);

  // Initialize communication channels.
  Io_handler* ctrl = new Control_server(config.control_socket);
  Io_handler* swi = new Switch_server(config.switch_socket);

  // Initialize io/handlers
  register_handler(ctrl);
  register_handler(swi);

  ps.reset();
  running = true;
  // Make loop.
  while (running) {

    // Poll for events
    poll(ps, 100);

    // Process events.
    Io_set rm;
    for (std::size_t i = 0; i < ps.size(); ++i) {
      Poll_file& pf = ps[i];
      process_events(pf, rm);
    }

    // Remove closed handlers.
    for (const Io_handler* h : rm)
      unregister_handler(h);

    // Reset all the read events.
    ps.reset();
  }

  // Clean up.

  // Unlink previous sockets.
  unlink(config.control_socket);
  unlink(config.switch_socket);

  // Free memory.
  delete ctrl;
  delete swi;
}
