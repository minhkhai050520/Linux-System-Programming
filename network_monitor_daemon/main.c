#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include "daemon.h"
#include "pid_file.h"
#include "signal_handler.h"
#include "config.h"
#include "syslog_logger.h"
#include "netlink_monitor.h"
#include "unix_socket.h"

/* Global flag for running state */
volatile sig_atomic_t running = 1;

void usage(const char *prog_name);

/* Main event loop using select() for multiplexing */
static int event_loop(int nl_sock, int server_sock)
{
    fd_set readfds;
    int max_fd;
    char buffer[16384];

    syslog_log(LOG_INFO, "Entering main event loop");

    while (running)
    {
        FD_ZERO(&readfds);
        FD_SET(nl_sock, &readfds);
        FD_SET(server_sock, &readfds);

        max_fd = (nl_sock > server_sock) ? nl_sock : server_sock;

        /* Wait for activity on sockets (1 second timeout for signal handling) */
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            if (running)
                syslog_log(LOG_ERR, "select() error");
            continue;
        }

        /* Handle netlink socket - network events */
        if (FD_ISSET(nl_sock, &readfds))
        {
            /* Setup iovec for recvmsg */
            struct iovec iov = {
                .iov_base = buffer,
                .iov_len = sizeof(buffer)
            };

            /* Netlink socket address */
            struct sockaddr_nl nladdr = {.nl_family = AF_NETLINK};

            /* Message header for recvmsg */
            struct msghdr msg = {
                .msg_name = &nladdr,
                .msg_namelen = sizeof(nladdr),
                .msg_iov = &iov,
                .msg_iovlen = 1
            };

            int status = recvmsg(nl_sock, &msg, 0);

            if (status < 0)
            {
                /* Interrupted System Call/Resource Temporarily Unavailable error */
                if (errno == EINTR || errno == EAGAIN)
                    continue;
                syslog_log(LOG_ERR, "recvmsg error");
                return -1;
            }
            if (status == 0)
            {
                syslog_log(LOG_ERR, "EOF on netlink socket");
                return -1;
            }

            for (struct nlmsghdr *nlh = (struct nlmsghdr *)buffer; 
                    NLMSG_OK(nlh, status); 
                    nlh = NLMSG_NEXT(nlh, status))
            {
                if (nlh->nlmsg_type == NLMSG_DONE) break;
                if (nlh->nlmsg_type == NLMSG_ERROR) continue;
                int err = process_netlink_message(nlh);

                if (err < 0)
                {
                    syslog_log(LOG_ERR, "Error processing netlink message");
                    return -1;
                }
            }
        }

        /* Handle UNIX socket - CLI commands */
        if (FD_ISSET(server_sock, &readfds))
        {
            int client_sock = accept(server_sock, NULL, NULL);
            if (client_sock >= 0)
            {
                if(handle_client_connection(client_sock) == -1)
                {
                    syslog_log(LOG_ERR, "Error handling client connection");
                }
            }
            else if (running)
            {
                syslog_log(LOG_ERR, "accept() error");
            }
        }
    }

    syslog_log(LOG_INFO, "Exiting main event loop");
    return 0;
}

int main(int argc, char *argv[])
{
    int nl_sock = -1;
    int server_sock = -1;

    if (argc < 2)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Read configuration */
    if (read_config() == -1)
    {
        syslog_log(LOG_ERR, "Failed to read configuration");
        return EXIT_FAILURE;
    }

    /* Initialize syslog */
    init_syslog();

    if (strcmp(argv[1], "start") == 0)
    {
        /* Daemonize the process */
        if (daemonize(0) == -1)
        {
            syslog_log(LOG_ERR, "Failed to daemonize\n");
            return EXIT_FAILURE;
        }

        /* Write PID file */
        if (write_pid_file() == -1)
        {
            syslog_log(LOG_ERR, "Failed to write PID file");
            return EXIT_FAILURE;
        }

        /* Setup signal handlers */
        if (setup_signal_handlers() == -1)
        {
            syslog_log(LOG_ERR, "Failed to setup signal handlers");
            return EXIT_FAILURE;
        }

        /* Initialize netlink socket for network monitoring */
        nl_sock = netlink_socket_init();
        if (nl_sock == -1)
        {
            syslog_log(LOG_ERR, "Failed to initialize netlink socket");
            return EXIT_FAILURE;
        }

        /* Initialize UNIX domain socket server for CLI */
        server_sock = unix_socket_server_init();
        if (server_sock == -1)
        {
            syslog_log(LOG_ERR, "Failed to initialize UNIX socket server");
            close(nl_sock);
            return EXIT_FAILURE;
        }

        /* Main event loop using select() for socket multiplexing */
        event_loop(nl_sock, server_sock);

        /* Cleanup */
        close(nl_sock);
        close(server_sock);
        unix_socket_cleanup();
        remove_pid_file();
    }
    else
    {
        usage(argv[0]);
    }

    close_syslog();
    return EXIT_SUCCESS;
}

void usage(const char *prog_name)
{
    printf("Usage: %s {start}\n", prog_name);
}