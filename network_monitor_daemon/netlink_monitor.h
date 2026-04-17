#ifndef NETLINK_MONITOR_H
#define NETLINK_MONITOR_H

#include <linux/netlink.h>

// Initialize netlink socket for monitoring network events
int netlink_socket_init(void);

// Process netlink message from buffer
void process_netlink_message(struct nlmsghdr *nlh);

#endif // NETLINK_MONITOR_H