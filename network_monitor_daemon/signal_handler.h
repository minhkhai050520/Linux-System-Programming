#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

// Function to setup signal handlers
int setup_signal_handlers(void);

// Signal handler for SIGTERM
void sigterm_handler(int sig);

// Signal handler for SIGHUP
void sighup_handler(int sig);

#endif // SIGNAL_HANDLER_H