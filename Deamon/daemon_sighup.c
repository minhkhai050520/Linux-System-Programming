#include <sys/stat.h>
#include <fcntl.h>
#include "become_daemon.h"
#include "tlpi_hdr.h"
#include <signal.h>   // sig_atomic_t, sigaction, SIGHUP, SA_RESTART
#include <syslog.h>   // openlog, syslog, LOG_*
#include <unistd.h>   // sleep
#include <stdlib.h>   // exit

// static const char *LOG_FILE = "/tmp/ds.log";
static const char *CONFIG_FILE = "/tmp/ds.conf";

static volatile sig_atomic_t hupReceived = 0;

static void sighupHandler(int sig)
{
    hupReceived = 1;
}

int main (int argc, char* argv[])
{
    const int SLEEP_TIME = 15;
    int count = 0;
    int unslept;
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sighupHandler;

    if (sigaction(SIGHUP, &sa, NULL) == -1)
        errExit("sigaction");

    if (becomeDaemon(0) == -1)
        errExit("becomeDaemon");

    openlog("khai", LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon started");

    unslept = SLEEP_TIME;

    for(;;)
    {
        unslept = sleep(unslept);

        if (hupReceived) {
            syslog(LOG_INFO, "sighup is enable");
            hupReceived = 0;
        }

        if (unslept == 0)
        {
            count++;
            syslog(LOG_INFO, "Main loop count: %d", count);
            unslept = SLEEP_TIME;
        }
    }
}
