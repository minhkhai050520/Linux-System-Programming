#include <string.h>
#include "config.h"

/* Global config instance */
Config config;

/* Read configuration*/
int read_config(void)
{
    strncpy(config.socket_path, SOCKET_PATH, sizeof(config.socket_path) - 1);
    config.socket_path[sizeof(config.socket_path) - 1] = '\0';
    strncpy(config.log_ident, LOG_IDENT, sizeof(config.log_ident) - 1);
    config.log_ident[sizeof(config.log_ident) - 1] = '\0';
    return 0;
}