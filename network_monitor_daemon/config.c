#include <string.h>
#include "config.h"

/* Global config instance */
Config config;

/* Read configuration*/
int read_config(void)
{
    /* socket path */
    strncpy(config.socket_path, SOCKET_PATH, sizeof(config.socket_path) - 1);
    config.socket_path[sizeof(config.socket_path) - 1] = '\0';

    /* PID file */
    strncpy(config.pid_file, PID_FILE, sizeof(config.pid_file) - 1);
    config.pid_file[sizeof(config.pid_file) - 1] = '\0';

    /* Log identifier */
    strncpy(config.log_ident, LOG_IDENT, sizeof(config.log_ident) - 1);
    config.log_ident[sizeof(config.log_ident) - 1] = '\0';
    return 0;
}