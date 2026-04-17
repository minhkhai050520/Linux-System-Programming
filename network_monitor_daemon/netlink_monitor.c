#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "netlink_monitor.h"
#include "syslog_logger.h"

// Process netlink messages and log network events
void process_netlink_message(struct nlmsghdr *nlh) {
    struct ifinfomsg *ifi = NLMSG_DATA(nlh);
    char ifname[IF_NAMESIZE] = {0};

    // Get interface name
    if_indextoname(ifi->ifi_index, ifname);

    switch (nlh->nlmsg_type) {
        case RTM_NEWLINK:
            syslog_log(LOG_INFO, "Interface %s is UP", ifname);
            break;
        case RTM_DELLINK:
            syslog_log(LOG_INFO, "Interface %s is DOWN", ifname);
            break;
        case RTM_NEWADDR:
            syslog_log(LOG_INFO, "IP address added to interface %s", ifname);
            break;
        case RTM_DELADDR:
            syslog_log(LOG_INFO, "IP address removed from interface %s", ifname);
            break;
        default:
            break;
    }
}

// Initialize netlink socket for monitoring network events
int netlink_socket_init(void) {
    struct sockaddr_nl sa;
    int nl_sock;

    // Create netlink socket
    nl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_sock < 0) {
        syslog_log(LOG_ERR, "Failed to create netlink socket");
        return -1;
    }

    // Bind socket
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
    if (bind(nl_sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        syslog_log(LOG_ERR, "Failed to bind netlink socket");
        close(nl_sock);
        return -1;
    }

    return nl_sock;
}