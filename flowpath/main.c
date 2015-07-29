// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "util.h"
#include "dataplane.h"
#include "manage.h"

#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>


/* FIXME: Not good! */
struct pollfd fp_poll_set[16];


/* The size of the poll set. */
size_t fp_poll_size;


static void
clear_poll_results()
{
  for (int i = 0; i < fp_poll_size; ++i)
    fp_poll_set[i].revents = 0;
}


/* Flag used to initiate a graceful shutdown. */
bool running;


/* Signal handling function */
void
signal_handler(int sig)
{
  if (running)
    running = false;
}


/* Set the signal mask for this application. */
static void
set_signal_mask()
{
  signal(SIGINT, signal_handler);
  signal(SIGKILL, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGTERM, signal_handler);
}


int 
main(int argc, char* argv[]) 
{
  /* Set the signal masks. */
  set_signal_mask();


  /* FIXME: Load configuration before opening the server. */
  int mgr = fp_mgr_open();
  if (mgr < 1)
    return -1;

  /* Put mgr into the poll loop. */
  fp_poll_set[0].fd = mgr;
  fp_poll_set[0].events = POLLIN;

  /* Set the poll set size. */
  fp_poll_size = 1;
  running = true;
  /* Run the main loop. */
  while (running) {
    clear_poll_results();

    /* Poll for events. */
    if (poll(fp_poll_set, fp_poll_size, 100) < 0) {
      perror("main/poll");
      break;
    }

    /* Did we get a message from flowmgr. */
    if (fp_poll_set[0].revents &= POLLIN) {
      int result = fp_mgr_incoming();
      if (result <= 0)
        break;
    }
  }

  fp_mgr_close();
}
