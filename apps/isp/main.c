#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "v4l2.h"
#include "isp.h"
#include "dxo.h"
#include "isp-io.h"
#include "gdm-buffer.h"
#include "display.h"
#include "debug.h"

/*****************************************************************************/

// /dev/videoX
#define GISP_PORT_NODE_BASE                 (1)

//#define GISP_PIXEL_FORMAT           V4L2_PIX_FMT_NV12
//#define GISP_PIXEL_FORMAT           V4L2_PIX_FMT_YUV420M
//422p2
#define GISP_PIXEL_FORMAT           V4L2_PIX_FMT_NV16

#define GISP_DEFAULT_WIDTH          (640)
#define GISP_DEFAULT_HEIGHT         (480)

#define DISPLAY_SOCK_PATH           "/tmp/sock_msgio"

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
    struct GDMDisplay *display;
    struct GISPPort port[GISP_PORT_COUNT];
};

struct Option {
    int buffers;
    int v4l2;
    int display;
};

/*****************************************************************************/

static struct Option opt = {
    .buffers = 8,
    .v4l2 = 1,
    .display = 0,
};

/*****************************************************************************/

static struct GISPContext *GISPOpen(int display)
{
    int i;
    char path[64];
    struct GISPContext *gisp;
    struct GDMDispFormat fmt;

    gisp = calloc(1, sizeof(*gisp));
    ASSERT(gisp);

    for (i = 0; i < GISP_PORT_COUNT; i++) {
        sprintf(path, "/dev/video%d", i + GISP_PORT_NODE_BASE);
        gisp->port[i].fd = open(path, O_RDWR);
        ASSERT(gisp->port[i].fd > 0);
    }

    if (display) {
        gisp->display = GDispOpen(DISPLAY_SOCK_PATH, 0);
        ASSERT(gisp->display);

        fmt.pixelformat = GISP_PIXEL_FORMAT;
        fmt.width = GISP_DEFAULT_WIDTH;
        fmt.height = GISP_DEFAULT_HEIGHT;

        GDispSetFormat(gisp->display, &fmt);
    }

    return gisp;
}

static void GISPClose(struct GISPContext *gisp)
{
    int i;
    ASSERT(gisp);

    if (gisp->display)
        GDispClose(gisp->display);

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

        if (gisp->display)
            GDispSendFrame(gisp->display, buffers[idx], 1000);

        ret = v4l2_qbuf(gisp->port[GISP_PORT_DIS].fd,
                GISP_DEFAULT_WIDTH, GISP_DEFAULT_HEIGHT, buffers[idx], idx);

        if (ret) {
            DBG("=====> QBUF(%d) FAILED <=====", idx);
        }
    }
}

static void help(int argc, char **argv)
{
    fprintf(stderr,
            "%s: [options]\n"
            "  --buffer | -b       v4l2 buffer count\n"
            "  --display | -d      send pictures to HWC to display\n"
            "  --v4l2 | v          use v4l2 buffer handling\n"
            "  --help | h          show this message\n",
            argv[0]);
}

static int parseOption(int argc, char **argv)
{
    int c, optidx = 0;
    static struct option options[] = {
        { "buffer", 1, 0, 'b' },
        { "v4l2", 0, 0, 'v' },
        { "display", 0, 0, 'd' },
        { "help", 0, 0, 'h' },
        { 0, 0, 0, 0 },
    };

    for( ; ; ) {
        c = getopt_long(argc, argv, "b:vdh", options, &optidx);
        if(c == -1)
            break;
        switch(c) {
        case 'b':
            opt.buffers = atoi(optarg);
            break;
        case 'v':
            opt.v4l2 = 1;
            break;
        case 'd':
            opt.display = 1;
            break;
        case 'h':
            help(argc, argv);
            exit(EXIT_SUCCESS);
            break;
        default:
            return -1;
        }
    }

    if (opt.buffers == 0 && opt.v4l2) {
        ERR("buffer count must be specified.");
        exit(EXIT_FAILURE);
    }

    if (opt.display && !opt.v4l2) {
        ERR("v4l2 option is needed to display");
        exit(EXIT_FAILURE);
    }

    return 0;
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
    struct GDMBuffer **buffers;

    parseOption(argc, argv);

    buffers = calloc(opt.buffers, sizeof(*buffers));

    initISPIO();

    resetISP();

    gisp = GISPOpen(opt.display);
    ASSERT(gisp);

    // V4L2 init
    ret = v4l2_enum_fmt(gisp->port[GISP_PORT_DIS].fd, GISP_PIXEL_FORMAT);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(gisp->port[GISP_PORT_DIS].fd,
            GISP_DEFAULT_WIDTH, GISP_DEFAULT_HEIGHT, GISP_PIXEL_FORMAT, &fmt);
    ASSERT(ret == 0);

    ret = v4l2_reqbufs(gisp->port[GISP_PORT_DIS].fd, opt.buffers);
    DBG("reqbufs result = %d", ret);

    pixmp = &fmt.fmt.pix_mp;
    planes = pixmp->num_planes;
    for (i = 0; i < planes; i++)
        planeSizes[i] = pixmp->plane_fmt[i].sizeimage;

    for (i = 0; i < opt.buffers; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, 0);
        ASSERT(buffers[i]);
    }

    for (i = 0; i < opt.buffers; i++) {
        ret = v4l2_qbuf(gisp->port[GISP_PORT_DIS].fd,
                GISP_DEFAULT_WIDTH, GISP_DEFAULT_HEIGHT, buffers[i], i);
        DBG("v4l2_qbuf = %d", ret);
    }

    // Start interrupt handling thread
    startIRQHandlerThread();

    shell_isp_init(NULL);

    if (!opt.v4l2)
        pause();

    v4l2_streamon(gisp->port[GISP_PORT_DIS].fd);

    mainLoop(gisp, buffers, opt.buffers);

    v4l2_streamoff(gisp->port[GISP_PORT_DIS].fd);

    GISPClose(gisp);

    return 0;
}
