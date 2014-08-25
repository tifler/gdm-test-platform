#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "v4l2.h"
#include "isp.h"
#include "dxo.h"
#include "isp-io.h"
#include "gdm-buffer.h"
#include "debug.h"

/*****************************************************************************/

// /dev/videoX
#define GISP_PORT_NODE_BASE                 (1)

//#define GISP_PIXEL_FORMAT           V4L2_PIX_FMT_NV12
//#define GISP_PIXEL_FORMAT           V4L2_PIX_FMT_YUV420M
//422p2
#define GISP_PIXEL_FORMAT           V4L2_PIX_FMT_NV16
#define GISP_DIS_BUF_COUNT          (8)

#define GISP_DEFAULT_WIDTH          (640)
#define GISP_DEFAULT_HEIGHT         (480)

/*****************************************************************************/

enum {
    GISP_PORT_CAP,
    GISP_PORT_VID,
    GISP_PORT_DIS,
    GISP_PORT_FD,
    GISP_PORT_COUNT,
};

struct GISPPort {
    int fd;
};

struct GISPContext {
    struct GISPPort port[GISP_PORT_COUNT];
};

static struct GISPContext *GISPOpen(void)
{
    int i;
    char path[64];
    struct GISPContext *gisp;

    gisp = calloc(1, sizeof(*gisp));
    ASSERT(gisp);

    for (i = 0; i < GISP_PORT_COUNT; i++) {
        sprintf(path, "/dev/video%d", i + GISP_PORT_NODE_BASE);
        gisp->port[i].fd = open(path, O_RDWR);
        ASSERT(gisp->port[i].fd > 0);
    }

    return gisp;
}

static void GISPClose(struct GISPContext *gisp)
{
    int i;
    ASSERT(gisp);

    for (i = 0; i < GISP_PORT_COUNT; i++)
        close(gisp->port[i].fd);

    free(gisp);
}

static void mainLoop(
        struct GISPContext *gisp, struct GDMBuffer **buffers, int count)
{
    int ret;
    int idx;
    struct pollfd pollfd;

    pollfd.fd = gisp->port[GISP_PORT_DIS].fd;
    pollfd.events = POLLIN;
    pollfd.revents = 0;

    for ( ; ; ) {
        ret = poll(&pollfd, 1, 1000);
        if (ret < 0) {
            perror("poll");
            exit(0);
        }
        else if (ret == 0) {
            DBG("Timeout.");
            continue;
        }

        idx = v4l2_dqbuf(pollfd.fd, 3);
        if (idx < 0) {
            perror("v4l2_dqbuf");
            exit(0);
        }

        DBG("=====> DQBUF INDEX = %d <=====", idx);

        ret = v4l2_qbuf(gisp->port[GISP_PORT_DIS].fd,
                GISP_DEFAULT_WIDTH, GISP_DEFAULT_HEIGHT, buffers[idx], idx);

        if (ret) {
            DBG("=====> QBUF(%d) FAILED <=====", idx);
        }
        else {
            DBG("=====> QBUF INDEX = %d <=====", idx);
        }
    }
}

/*****************************************************************************/

int main(int argc, char **argv)
{
    int i;
    int ret;
    int planes;
    struct GISPContext *gisp;
    struct v4l2_format fmt;
    struct v4l2_pix_format_mplane *pixmp;
    unsigned int planeSizes[3];
    struct GDMBuffer *buffers[GISP_DIS_BUF_COUNT];

    initISPIO();

    resetISP();

    gisp = GISPOpen();
    ASSERT(gisp);

    // V4L2 init
    ret = v4l2_enum_fmt(gisp->port[GISP_PORT_DIS].fd, GISP_PIXEL_FORMAT);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(gisp->port[GISP_PORT_DIS].fd,
            GISP_DEFAULT_WIDTH, GISP_DEFAULT_HEIGHT, GISP_PIXEL_FORMAT, &fmt);
    ASSERT(ret == 0);

    ret = v4l2_reqbufs(gisp->port[GISP_PORT_DIS].fd, GISP_DIS_BUF_COUNT);
    DBG("reqbufs result = %d", ret);

    pixmp = &fmt.fmt.pix_mp;
    planes = pixmp->num_planes;
    for (i = 0; i < planes; i++)
        planeSizes[i] = pixmp->plane_fmt[i].sizeimage;

    for (i = 0; i < GISP_DIS_BUF_COUNT; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, 0);
        ASSERT(buffers[i]);
    }

    for (i = 0; i < GISP_DIS_BUF_COUNT; i++) {
        ret = v4l2_qbuf(gisp->port[GISP_PORT_DIS].fd,
                GISP_DEFAULT_WIDTH, GISP_DEFAULT_HEIGHT, buffers[i], i);
        DBG("v4l2_qbuf = %d", ret);
    }

    // Start interrupt handling thread
    startIRQHandlerThread();

    shell_isp_init(NULL);

    v4l2_streamon(gisp->port[GISP_PORT_DIS].fd);

    mainLoop(gisp, buffers, GISP_DIS_BUF_COUNT);

    v4l2_streamoff(gisp->port[GISP_PORT_DIS].fd);

    GISPClose(gisp);

    return 0;
}
