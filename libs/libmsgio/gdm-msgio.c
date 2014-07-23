#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 

#include "gdm-msgio.h"
#include "debug.h"

/*****************************************************************************/

struct gdm_msghdr *gdm_alloc_msghdr(ssize_t buflen, ssize_t fdcount)
{
    struct gdm_msghdr *hdr;

    ASSERT(buflen <= GDM_MSGIO_MAX_BUF_SIZE);
    ASSERT(fdcount <= GDM_MSGIO_MAX_FD_COUNT);

    hdr = calloc(1, sizeof(*hdr));
    ASSERT(hdr);

    if (buflen > 0) {
        hdr->buf = (char *)calloc(1, buflen);
        ASSERT(hdr->buf);
        hdr->buflen = buflen;
    }

    if (fdcount > 0) {
        struct cmsghdr *cmsg;
        cmsg = (struct cmsghdr *)calloc(1, CMSG_SPACE(sizeof(int) * fdcount));
        ASSERT(cmsg);
        hdr->priv = (void *)cmsg;
        hdr->fds = (int *)CMSG_DATA(cmsg);
        hdr->fdcount = fdcount;
    }

    return hdr;
}

void gdm_free_msghdr(struct gdm_msghdr *hdr)
{
    ASSERT(hdr);

    if (hdr->buflen > 0) {
        ASSERT(hdr->buf);
        free(hdr->buf);
    }

    if (hdr->fdcount > 0) {
        ASSERT(hdr->priv);
        free(hdr->priv);
    }

    free(hdr);
}

ssize_t gdm_sendmsg(int sockfd, struct gdm_msghdr *hdr)
{
    struct msghdr msg;
    struct iovec iov;
    struct cmsghdr *cmsg;

    ASSERT(sockfd > 0);
    ASSERT(hdr);
    ASSERT((hdr->buflen > 0 && hdr->buf) || (hdr->fdcount > 0 && hdr->fds));

    memset(&msg, 0, sizeof(msg));

    if (hdr->buflen > 0) {
        iov.iov_base = hdr->buf;
        iov.iov_len = hdr->buflen;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
    }

    if (hdr->fdcount > 0) {
        msg.msg_control = hdr->priv;
        msg.msg_controllen = CMSG_SPACE(sizeof(int) * hdr->fdcount);
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int) * hdr->fdcount);
    }

    return sendmsg(sockfd, &msg, 0);
}

struct gdm_msghdr *gdm_recvmsg(int sockfd)
{
    ssize_t size;
    struct msghdr msg;
    struct iovec iov;
    struct cmsghdr *cmsg;
    struct gdm_msghdr *hdr;

    hdr = gdm_alloc_msghdr(GDM_MSGIO_MAX_BUF_SIZE, GDM_MSGIO_MAX_FD_COUNT);
    ASSERT(hdr);

    iov.iov_base = hdr->buf;
    iov.iov_len = hdr->buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = hdr->priv;
    msg.msg_controllen = CMSG_SPACE(sizeof(int) * hdr->fdcount);

    size = recvmsg(sockfd, &msg, 0);

    // modify to real recv size
    hdr->buflen = size;
    
    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg) {
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
            hdr->fdcount =
                (cmsg->cmsg_len - CMSG_ALIGN(sizeof(*cmsg))) / sizeof(int);
            hdr->fds = (int *)CMSG_DATA(cmsg);
        }
    }

    return hdr;
}
