#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "gdm-msgio.h"
#include "debug.h"

/*****************************************************************************/

#define UNIX_SOCKET_PATH                "/tmp/sock_msgio"
#define BUFFER_SIZE                     (256)
#define FD_COUNT                        (10)

/*****************************************************************************/

struct client {
    int sockfd;
    struct sockaddr addr;
    pthread_t thid;
};

/*****************************************************************************/

static void test_msgio(int sockfd)
{
    int i;
    int ret;
    int len;
    ssize_t size;
    struct gdm_msghdr *msg;

    msg = gdm_alloc_msghdr(BUFFER_SIZE, FD_COUNT);

    DBG("msg->buflen = %d", (int)msg->buflen);

    msg->fdcount = FD_COUNT;
    for (i = 0; i < msg->fdcount; i++) {
        len = snprintf(msg->buf, msg->buflen - 1, "/tmp/msgio-file-%04d", i);
        msg->fds[i] = open(msg->buf, O_RDWR | O_CREAT | O_TRUNC, 0644);
        ASSERT(msg->fds[i] > 0);
        DBG("msg->fds[%d] = %d", i, msg->fds[i]);
        msg->buf[len++] = '\n';
        ret = write(msg->fds[i], msg->buf, len);
        DBG("snprintf(%d) write(%d)", len, ret);
    }

    size = gdm_sendmsg(sockfd, msg);
    ASSERT(size > 0);
    DBG("Send msg size = %d", (int)size);

    for (i = 0; i < msg->fdcount; i++) {
        close(msg->fds[i]);
    }

    gdm_free_msghdr(msg);
}

static void process_client(const char *serverPath)
{
    int ret;
    int sockfd;
    struct sockaddr_un server_addr;

    ASSERT(serverPath);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT(sockfd > 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, serverPath);

    ret = connect(sockfd,
            (struct sockaddr *)&server_addr, sizeof(server_addr));
    ASSERT(ret == 0 && "connect() failed");

    DBG("Client connected to server...");

    test_msgio(sockfd);

    close(sockfd);
}

/*****************************************************************************/

int main(int argc, char **argv)
{
    int i;
    for (i = 0; i < 100; i++) {
        process_client(UNIX_SOCKET_PATH);
        usleep(10000);
    }
    return 0;
}
