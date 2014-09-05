#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gisp/gisp-isp.h"
#include "gisp/gisp-sif.h"
#include "gisp/gisp-dxo.h"
#include "v4l2.h"
#include "gdm-buffer.h"
#include "display.h"
#include "debug.h"

/*****************************************************************************/

// /dev/videoX
#define GCAM_PORT_NODE_BASE                 (1)
#define GCAM_PIXEL_FORMAT                   V4L2_PIX_FMT_NV16
#define DISPLAY_SOCK_PATH                   "/tmp/sock_msgio"

#define SENSOR_WIDTH                        (640)
#define SENSOR_HEIGHT                       (480)

#define DISPLAY_WIDTH                       (640)
#define DISPLAY_HEIGHT                      (480)

/*****************************************************************************/

enum {
    GCAM_PORT_CAP,
    GCAM_PORT_VID,
    GCAM_PORT_DIS,
    GCAM_PORT_FD,
    GCAM_PORT_COUNT,
};

struct GCamPort {
    int fd;
};

struct GCamera {
    struct GCamPort port[GCAM_PORT_COUNT];
    struct GDMDisplay *display;
};

struct Option {
    int buffers;
    int v4l2;
    int display;
};

static struct Option opt = {
    .buffers = 8,
    .v4l2 = 1,
    .display = 0,
};

/*****************************************************************************/

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

struct GCamera *GCamOpen(void)
{
    int i;
    char path[64];
    struct GCamera *gcam;

    gcam = calloc(1, sizeof(*gcam));
    ASSERT(gcam);

    for (i = 0; i < GCAM_PORT_COUNT; i++) {
        sprintf(path, "/dev/video%d", i + GCAM_PORT_NODE_BASE);
        gcam->port[i].fd = open(path, O_RDWR);
        ASSERT(gcam->port[i].fd > 0);
    }

    return gcam;
}

void GCamClose(struct GCamera *gcam)
{
    int i;

    ASSERT(gcam);

    for (i = 0; i < GCAM_PORT_COUNT; i++)
        close(gcam->port[i].fd);

    free(gcam);
}

static void mainLoop(
        struct GCamera *gcam, struct GDMBuffer **buffers, int count, int display)
{
    int ret;
    int idx;
    struct pollfd pollfd;

    if (display) {
        struct GDMDispFormat fmt;

        gcam->display = GDispOpen(DISPLAY_SOCK_PATH, 0);
        ASSERT(gcam->display);

        fmt.pixelformat = GCAM_PIXEL_FORMAT;
        fmt.width = DISPLAY_WIDTH;
        fmt.height = DISPLAY_HEIGHT;
        GDispSetFormat(gcam->display, &fmt);
    }

    pollfd.fd = gcam->port[GCAM_PORT_DIS].fd;
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

        if (gcam->display)
            GDispSendFrame(gcam->display, buffers[idx], 1000);

        ret = v4l2_qbuf(gcam->port[GCAM_PORT_DIS].fd,
                DISPLAY_WIDTH, DISPLAY_HEIGHT, buffers[idx], idx);

        if (ret) {
            DBG("=====> QBUF(%d) FAILED <=====", idx);
        }
        DBG("Buffer Index = %d", idx);
    }
}

/*****************************************************************************/

//int initdone;

int main(int argc, char **argv)
{
    int i;
    int ret;
    int planes;
    struct GCamera *gcam;
    struct v4l2_format fmt;
    struct v4l2_pix_format_mplane *pixmp;
    unsigned int planeSizes[3];
    struct GDMBuffer **buffers;
    struct DXOSystemConfig conf;
    struct DXOControl ctrl;
    struct DXOOutputFormat dxoFmt;
    struct SIFConfig sifConf;
    struct ISP *isp;
    struct SIF *sif;
    struct DXO *dxo;

    parseOption(argc, argv);

    buffers = calloc(opt.buffers, sizeof(*buffers));

    gcam = GCamOpen();
    ASSERT(gcam);

    // V4L2 init
    ret = v4l2_enum_fmt(gcam->port[GCAM_PORT_DIS].fd, GCAM_PIXEL_FORMAT);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(gcam->port[GCAM_PORT_DIS].fd,
            DISPLAY_WIDTH, DISPLAY_HEIGHT, GCAM_PIXEL_FORMAT, &fmt);
    ASSERT(ret == 0);

    ret = v4l2_reqbufs(gcam->port[GCAM_PORT_DIS].fd, opt.buffers);
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
        ret = v4l2_qbuf(gcam->port[GCAM_PORT_DIS].fd,
                DISPLAY_WIDTH, DISPLAY_HEIGHT, buffers[i], i);
        DBG("v4l2_qbuf = %d", ret);
    }

    memset(&conf, 0, sizeof(conf));
    conf.sysFreqMul = 32;
    conf.sysFreqDiv = 1;
    conf.frmTimeMul = 4;
    conf.frmTimeDiv = 1;

    isp = ISPInit();
    sif = SIFInit();
    dxo = DXOInit(&conf);

    sifConf.width = SENSOR_WIDTH;
    sifConf.height = SENSOR_HEIGHT;
    SIFSetConfig(sif, &sifConf);

    ctrl.input = DXO_INPUT_SOURCE_FRONT;
    ctrl.hMirror = 0;
    ctrl.vFlip = 0;
    ctrl.enableTNR = 0;
    ctrl.fpsMul = 8;
    ctrl.fpsDiv = 1;
    DXOSetControl(dxo, &ctrl);

    dxoFmt.width = DISPLAY_WIDTH;
    dxoFmt.height = DISPLAY_HEIGHT;
    dxoFmt.pixelFormat = V4L2_PIX_FMT_UYVY;
    dxoFmt.crop.left = 0;
    dxoFmt.crop.top = 0;
    dxoFmt.crop.right = DISPLAY_WIDTH - 1;
    dxoFmt.crop.bottom = DISPLAY_HEIGHT - 1;
    DXOSetOutputFormat(dxo, DXO_OUTPUT_DISPLAY, &dxoFmt);

    DXOSetOutputEnable(dxo, 1 << DXO_OUTPUT_DISPLAY, 1 << DXO_OUTPUT_DISPLAY);
    DXORunState(dxo, DXO_STATE_PREVIEW, 0);

//    initdone = 1;

    if (!opt.v4l2)
        pause();

    v4l2_streamon(gcam->port[GCAM_PORT_DIS].fd);
    DBG("======= STREAM ON =======");

    mainLoop(gcam, buffers, opt.buffers, opt.display);

    v4l2_streamoff(gcam->port[GCAM_PORT_DIS].fd);

    DXOExit(dxo);
    SIFExit(sif);
    ISPExit(isp);

    GCamClose(gcam);

    return 0;
}
