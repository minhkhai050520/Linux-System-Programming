#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include "tlpi_hdr.h"


/* =========================
 * HANDLE LINK EVENT
 * =========================
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
    for (attr = IFLA_RTA(ifi); RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
        if (attr->rta_type == IFLA_IFNAME) {
            /* get the interface name (eth0, wlan0...) */
            strncpy(ifname, RTA_DATA(attr), IF_NAMESIZE - 1);
            ifname[IF_NAMESIZE - 1] = '\0'; /* Ensure null-termination */
            break;
        }
    }

    /* Check the message type */
    if (nlh->nlmsg_type == RTM_NEWLINK) {

        /* check flag to know UP or DOWN */
        if (ifi->ifi_flags & IFF_UP)
            printf("[LINK] %s is UP\n", ifname);
        else
            printf("[LINK] %s is DOWN\n", ifname);

    } else if (nlh->nlmsg_type == RTM_DELLINK) {
        printf("[LINK] %s is REMOVED\n", ifname);
    }
    return 0;
}

/* =========================
 * HANDLE ADDRESS EVENT
 * =========================
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
    for (attr = IFA_RTA(ifa); RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {

        if (attr->rta_type == IFA_ADDRESS) {

            unsigned char *ip = RTA_DATA(attr);

            /* Only process IPv4 */
            if (ifa->ifa_family == AF_INET) {

                printf("[ADDR] %s IPv4: %d.%d.%d.%d (%s)\n",
                       ifname,
                       ip[0], ip[1], ip[2], ip[3],
                       (nlh->nlmsg_type == RTM_NEWADDR) ? "ADD" : "DEL");
            }
        }
    }
    return 0;
}

static int listenNetLink(int* fd,
                        int (*callback)(struct nlmsghdr *n, void *arg),
                        void *arg)
{
    char buf[16384];

    /* Setup iovec for recvmsg */
    struct iovec iov = {
        .iov_base = buf,
        .iov_len = sizeof(buf)
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

    /* Receive messages from kernel */
    while(1)
    {
        int status = recvmsg(*fd, &msg, 0);
        
        if (status < 0)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            perror("recvmsg");
            return -1;
        }

        if (status == 0)
        {
            fprintf(stderr, "EOF on netlink socket\n");
            return -1;
        }
        /* Process every nlmsghdr in buffer */
        for (struct nlmsghdr *h = (struct nlmsghdr *)buf;
             NLMSG_OK(h, status);
             h = NLMSG_NEXT(h, status)) {  /* NLMSG_NEXT = move to next header */
            
            /* Call the callback function to process the message */
            int err = callback(h, arg);
            
            /* If callback returns <0, stop listening */
            if (err < 0) return err;
        }
    }
}

int handle_netlink_msg(struct nlmsghdr *nlh, void *arg)
{
    switch (nlh->nlmsg_type) {

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

int main()
{
    struct sockaddr_nl sa;

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    /* Create a raw netlink socket */
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Bind the socket */
    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("Listening for netlink messages...\n");

    if (listenNetLink(&fd, handle_netlink_msg, stdout) < 0)
    {
        perror("listenNetLink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
