#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#define BACKLOG 5
/* Create and bind UNIX domain socket server */
int unix_socket_server_init(void);

/* Handle client connection on UNIX socket */
void handle_client_connection(int client_sock);

/* Cleanup UNIX socket */
void unix_socket_cleanup(void);

#endif /* UNIX_SOCKET_H */