#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>
#include "unix_socket.h"
#include "config.h"
#include "syslog_logger.h"
#include "pid_file.h"

/* Handle client commands and send responses */
int handle_client_connection(int client_sock)
{
    char buffer[256] = {0};
    ssize_t n;

    n = read(client_sock, buffer, sizeof(buffer) - 1);
    if (n > 0)
    {
        buffer[n] = '\0';
        if (strcmp(buffer, "status\n") == 0)
        {
            syslog_log(LOG_DEBUG, "Client requested status");
            pid_t pid = read_pid_file();
            if (pid == -1)
            {
                write(client_sock, "Daemon is not running\n", 22);
                close(client_sock);
                return -1;
            }
            else
            {
                write(client_sock, "Daemon is running\n", 18);
            }
        }
        else if (strcmp(buffer, "stop\n") == 0)
        {
            // write(client_sock, "Stopping daemon\n", 16);
            syslog_log(LOG_INFO, "Client requested monitor daemon stop");
            pid_t pid = read_pid_file();
            if (pid == -1)
            {
                write(client_sock, "Daemon is not running\n", 22);
                syslog_log(LOG_WARNING, "Stop command received but daemon is not running");
                close(client_sock);
                return -1;
            }
            if (kill(pid, SIGTERM) == -1)
            {
                write(client_sock, "Failed to send stop signal\n", 26);
                syslog_log(LOG_ERR, "Failed to send stop signal to daemon (PID: %d)", pid);
                close(client_sock);
                return -1;
            }
            else
            {
                    write(client_sock, "Stop signal sent to daemon\n", 27);
                    syslog_log(LOG_INFO, "Stop signal sent to daemon (PID: %d)", pid);
            }
        }
        else if (strcmp(buffer, "reload\n") == 0)
        {
            syslog_log(LOG_INFO, "Client requested monitor daemon configuration reload");
            pid_t pid = read_pid_file();
            if (pid == -1)
            {
                write(client_sock, "Daemon is not running\n", 22);
                syslog_log(LOG_WARNING, "Reload command received but daemon is not running");
                close(client_sock);
                return -1;
            }
            if (kill(pid, SIGHUP) == -1)
            {
                write(client_sock, "Failed to send reload signal\n", 28);
                syslog_log(LOG_ERR, "Failed to send reload signal to daemon (PID: %d)", pid);
                close(client_sock);
                return -1;
            }
            else
            {
                write(client_sock, "Reload signal sent to daemon\n", 29);
                syslog_log(LOG_INFO, "Reload signal sent to daemon (PID: %d)", pid);
            }
        }
        else
        {
            write(client_sock, "Unknown command\n", 16);
            syslog_log(LOG_WARNING, "Unknown command from client: %s", buffer);
        }
    }
    close(client_sock);
    return 0;
}

/* Create and bind UNIX domain socket server */
int unix_socket_server_init(void)
{
    struct sockaddr_un sa;
    int server_sock;

    /* Create socket */
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        syslog_log(LOG_ERR, "Failed to create UNIX socket");
        return -1;
    }

    /* Remove existing socket file */
    if (remove(config.socket_path) == -1 && errno != ENOENT)
    {
        syslog_log(LOG_ERR, "Failed to remove existing socket file");
        close(server_sock);
        return -1;  
    }

    /* Bind socket */
    memset(&sa, 0, sizeof(struct sockaddr_un));
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, config.socket_path, sizeof(sa.sun_path) - 1);
    if (bind(server_sock, (struct sockaddr *) &sa, sizeof(struct sockaddr_un)) == -1)
    {
        syslog_log(LOG_ERR, "Failed to bind UNIX socket");
        close(server_sock);
        return -1;
    }

    /* Listen for incoming connections */
    if (listen(server_sock, BACKLOG) == -1)
    {
        syslog_log(LOG_ERR, "Failed to listen on UNIX socket");
        close(server_sock);
        return -1;
    }

    syslog_log(LOG_INFO, "UNIX socket server initialized at %s", config.socket_path);
    return server_sock;
}

/* Cleanup UNIX socket */
void unix_socket_cleanup(void)
{
    if (remove(config.socket_path) == -1 && errno != ENOENT)
        syslog_log(LOG_ERR, "Failed to remove UNIX socket file during cleanup");
}