#ifndef CONFIG_H
#define CONFIG_H

#define SOCKET_PATH "/run/network_monitor_app/network_monitor.sock"
#define LOG_IDENT "network_monitor_daemon"

/* Configuration structure */
typedef struct {
    char socket_path[256];
    char log_ident[256];
} Config;

/* Global config instance */
extern Config config;

/* Function to read configuration */
int read_config(void);

#endif /* CONFIG_H */