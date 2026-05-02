#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

/* Global flag for running state */
extern volatile sig_atomic_t running;

/* Global flag for config reload */
extern volatile sig_atomic_t reload_config_flag;

/* Function to setup signal handlers */
int setup_signal_handlers(void);

/* Signal handler for SIGTERM */
void sigterm_handler(int sig);

/* Signal handler for SIGHUP */
void sighup_handler(int sig);

#endif /* SIGNAL_HANDLER_H */