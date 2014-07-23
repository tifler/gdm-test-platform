#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#include "gdm-msgio.h"
#include "debug.h"

/*****************************************************************************/

#define UNIX_SOCKET_PATH                "/tmp/sock_msgio"
#define BACKLOG                         (10)

/*****************************************************************************/

struct client {
    int sockfd;
    struct sockaddr addr;
    pthread_t thid;
};

/*****************************************************************************/

static void *client_thread(void *arg)
{
    int i;
    ssize_t len;
    char linebuf[256];
    struct gdm_msghdr *msg;
    struct client *client = (struct client *)arg;

    DBG("Client %d launched.", getpid());

    msg = gdm_recvmsg(client->sockfd);
    ASSERT(msg);

    DBG("msg->buf = %s", msg->buf);
    DBG("msg->buflen = %d", (int)msg->buflen);
    DBG("msg->fdcount = %d", (int)msg->fdcount);
    for (i = 0; i < msg->fdcount; i++) {
        DBG("msg->fds[%d] = %d", i, msg->fds[i]);
        // XXX client에서 write 하면서 position이 이동했음.
        // 따라서 lseek을 통해 처음으로 rewind해야함.
        lseek(msg->fds[i], (off_t)0, SEEK_SET);
        len = read(msg->fds[i], linebuf, sizeof(linebuf));
        linebuf[len] = 0;
        DBG("read(%d bytes) = %s", (int)len, linebuf);
        len = sprintf(linebuf, "Server writing to fds[%d]\n", i);
        write(msg->fds[i], linebuf, len);
    }

    sleep(1);

    for (i = 0; i < msg->fdcount; i++)
        close(msg->fds[i]);

    gdm_free_msghdr(msg);
    close(client->sockfd);
    free(client);

    return NULL;
}

static void process_client(int sockfd,
        const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    struct client *client;

    DBG("Client accepted. sock = %d", sockfd);

    client = calloc(1, sizeof(*client));
    ASSERT(client);

    client->sockfd = sockfd;
    memcpy(&client->addr, addr, addrlen);

    ret = pthread_create(
            &client->thid, (pthread_attr_t *)NULL, client_thread, client);
    ASSERT(ret == 0);

    pthread_detach(client->thid);
}

static int server_loop(const char *serverPath)
{
    int ret;
    int server_sock;
    int client_sock;
    socklen_t client_len;
    struct sockaddr_un server_addr, client_addr;

    ASSERT(serverPath);

    unlink(serverPath);

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT(server_sock > 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, serverPath);

    ret = bind(server_sock,
            (struct sockaddr *)&server_addr, sizeof(server_addr));
    ASSERT(ret == 0 && "bind() failed");

    ret = listen(server_sock, BACKLOG);
    ASSERT(ret == 0);

    DBG("Server is wating(server_sock = %d)...", server_sock);

    for ( ; ; ) {
        bzero(&client_addr, sizeof(client_addr));
        client_sock =
            accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

        process_client(client_sock, (struct sockaddr *)&client_addr, client_len);
    }

    return 0;
}

/*****************************************************************************/

int main(int argc, char **argv)
{
    return server_loop(UNIX_SOCKET_PATH);
}
