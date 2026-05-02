#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <signal.h>
#include "netlink_monitor.h"
#include "syslog_logger.h"

/*
 * HANDLE LINK EVENT
 */
int handle_link(struct nlmsghdr *nlh)
{
    /* Get the main data part of the message */
    struct ifinfomsg *ifi = NLMSG_DATA(nlh);

    /* attr is used to parse the fields inside the message */
    struct rtattr *attr;

    /* length of the attribute part */
    int len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));

    if (len < 0)
        return -1;

    char ifname[IF_NAMESIZE] = {0};

    /* Iterate through all attributes */
    for (attr = IFLA_RTA(ifi); RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
    {
        if (attr->rta_type == IFLA_IFNAME)
        {
            /* get the interface name (eth0, wlan0...) */
            strncpy(ifname, RTA_DATA(attr), IF_NAMESIZE - 1);
            ifname[IF_NAMESIZE - 1] = '\0'; /* Ensure null-termination */
            break;
        }
    }

    /* Check the message type */
    if (nlh->nlmsg_type == RTM_NEWLINK)
    {

        /* check flag to know UP or DOWN */
        if (ifi->ifi_flags & IFF_UP)
            syslog_log(LOG_INFO, "[LINK] %s is UP", ifname);
        else
            syslog_log(LOG_INFO, "[LINK] %s is DOWN", ifname);

    } else if (nlh->nlmsg_type == RTM_DELLINK)
    {
        syslog_log(LOG_INFO, "[LINK] %s is REMOVED", ifname);
    }
    return 0;
}

/*
 * HANDLE ADDRESS EVENT
 */
int handle_addr(struct nlmsghdr *nlh)
{
    struct ifaddrmsg *ifa = NLMSG_DATA(nlh);
    struct rtattr *attr;

    int len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa));

    if (len < 0)
        return -1;

    char ifname[IF_NAMESIZE] = {0};

    /* Convert interface index -> name (e.g.: 2 -> eth0) */
    if_indextoname(ifa->ifa_index, ifname);

    /* Iterate through attributes to get IP */
    for (attr = IFA_RTA(ifa); RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
    {

        if (attr->rta_type == IFA_ADDRESS)
        {
            unsigned char *ip = RTA_DATA(attr);

            /* Only process IPv4 */
            if (ifa->ifa_family == AF_INET)
            {

                syslog_log(LOG_INFO, "[ADDR] %s IPv4: %d.%d.%d.%d (%s)",
                           ifname,
                           ip[0], ip[1], ip[2], ip[3],
                           (nlh->nlmsg_type == RTM_NEWADDR) ? "ADD" : "DEL");
            }
        }
    }
    return 0;
}

/* Process netlink messages and log network events */
int process_netlink_message(struct nlmsghdr *nlh)
{
    switch (nlh->nlmsg_type)
    {
        case RTM_NEWLINK:
        case RTM_DELLINK:
        {
            int err = handle_link(nlh);
            if (err < 0)
                return err;
            break;
        }
        case RTM_NEWADDR:
        case RTM_DELADDR:
        {
            int err = handle_addr(nlh);
            if (err < 0)
                return err;
            break;
        }
        default:
            /* ignore events we don't care about */
            break;
    }
    return 0;
}

/* Initialize netlink socket for monitoring network events */
int netlink_socket_init(void)
{
    struct sockaddr_nl sa;

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    /* Create netlink socket */
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd == -1)
    {
        syslog_log(LOG_ERR, "Failed to create netlink socket");
        return -1;
    }

    /* Bind the socket */
    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1)
    {
        syslog_log(LOG_ERR, "Failed to bind netlink socket");
        close(fd);
        return -1;
    }

    return fd;
}