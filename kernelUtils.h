#ifndef __KERNEL_UTILS__
#define __KERNEL_UTILS__

#include <linux/netlink.h>

#define NETLINK_TEST_PROTOCOL 31

static inline char*
netlink_get_msg_type(__u16 nlmsg_type) {

        switch(msg_type) {
                case NLMSG_NOOP:
                        return "NLMSG_NOOP";
                case NLMSG_ERROR:
                        return "NLMSG_ERROR";
                case NLMSG_DONE:
                        return "NLMSG_DONE";
                case NLMSG_OVERRUN:
                        return "NLMSG_OVERRUN";
                default:
                        return "NLMSG_UNKNOWN";
        }
}

#endif
