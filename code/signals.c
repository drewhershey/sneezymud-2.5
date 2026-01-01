#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

#include "comm.h"
#include "utils.h"

static void shutdown_request(int tmp) {
  vlog("Received USR2 - shutdown request");
  Shutdown = 1;
}

/* kick out players etc */
static void hupsig(int tmp) {
  vlog("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0); /* something more elegant should perhaps be substituted */
}

static void logsig(int tmp) { vlog("Signal received. Ignoring."); }

static void checkpointing(int tmp) {
  if (tics == 0) {
    vlog("CHECKPOINT shutdown: tics not updated");
    abort();
  } else {
    tics = 0;
  }
}

void signal_setup(void) {
  struct itimerval itime{};
  struct timeval interval{};

  signal(SIGUSR2, shutdown_request);

  /* just to be on the safe side: */

  signal(SIGHUP, hupsig);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, hupsig);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);

  /* set up the deadlock-protection */

  interval.tv_sec = 900; /* 15 minutes */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, nullptr);
  signal(SIGVTALRM, checkpointing);
}
