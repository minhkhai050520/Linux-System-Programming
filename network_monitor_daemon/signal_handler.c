#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "signal_handler.h"
#include "config.h"
#include "syslog_logger.h"

// Extern declaration for the running flag
extern volatile sig_atomic_t running;

// Setup signal handlers for SIGTERM and SIGHUP
void setup_signal_handlers(void) {
    struct sigaction sa;

    // Setup SIGTERM handler
    sa.sa_handler = sigterm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog_log(LOG_ERR, "Failed to setup SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    // Setup SIGHUP handler
    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        syslog_log(LOG_ERR, "Failed to setup SIGHUP handler");
        exit(EXIT_FAILURE);
    }
}

// Signal handler for SIGTERM - stop the daemon
void sigterm_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    syslog_log(LOG_INFO, "Received SIGTERM, shutting down");
    running = 0;
}

// Signal handler for SIGHUP - reload configuration
void sighup_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    syslog_log(LOG_INFO, "Received SIGHUP, reloading configuration");
    if (read_config() == -1) {
        syslog_log(LOG_ERR, "Failed to reload configuration");
    } else {
        syslog_log(LOG_INFO, "Configuration reloaded");
    }
}