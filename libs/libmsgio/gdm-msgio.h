#ifndef __GDM_MSGIO_H__
#define __GDM_MSGIO_H__

#include <sys/types.h>

/*****************************************************************************/

// max buffer size per 1 sendmsg
#define GDM_MAX_BUF_SIZE                (4096)
// max transfer file descriptor count per 1 sendmsg
#define GDM_MAX_FD_COUNT                (16)

/*****************************************************************************/

struct gdm_msghdr {
    char *buf;
    ssize_t buflen;
    int *fds;
    ssize_t fdcount;
    // do not use. internally only;
    void *priv;
};

/*****************************************************************************/

/**
 * allocate buffers for user data and/or file descriptors.
 * Must be (buflen > 0 || fdcount > 0)
 */
struct gdm_msghdr *gdm_alloc_msghdr(ssize_t buflen, ssize_t fdcount);
void gdm_free_msghdr(struct gdm_msghdr *hdr);

ssize_t gdm_sendmsg(int sockfd, struct gdm_msghdr *hdr);
struct gdm_msghdr *gdm_recvmsg(int sockfd);

#endif  /*__GDM_MSGIO_H__*/
