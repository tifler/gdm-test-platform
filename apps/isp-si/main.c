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
#include "stream.h"
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

#define VIDEO_WIDTH                         (320)
#define VIDEO_HEIGHT                        (240)
#define VIDEO_PIXEL_FORMAT                  V4L2_PIX_FMT_NV16

/*****************************************************************************/

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

static int displayCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct GDMDisplay *disp = (struct GDMDisplay *)param;

    if (disp)
        GDispSendFrame(disp, buffer, 1000);

    DBG("DISPLAY CALLBACK: Buffer Index = %d", index);

    return 0;
}

static int videoCallback(void *param, struct GDMBuffer *buffer, int index)
{
    DBG("VIDEO CALLBACK: Buffer Index = %d", index);
    return 0;
}

/*****************************************************************************/

int main(int argc, char **argv)
{
    int i;
    int planes;
    unsigned int planeSizes[3];
    struct GDMBuffer **buffers;
    struct DXOSystemConfig conf;
    struct DXOControl ctrl;
    struct DXOOutputFormat dxoFmt;
    struct SIFConfig sifConf;
    struct ISP *isp;
    struct SIF *sif;
    struct DXO *dxo;
    struct STREAM *displayStream = NULL;
    struct STREAM *videoStream = NULL;

    parseOption(argc, argv);

    // display stream
    displayStream = streamOpen(STREAM_PORT_DISPLAY);
    ASSERT(displayStream);

    streamSetFormat(displayStream,
            DISPLAY_WIDTH, DISPLAY_HEIGHT, GCAM_PIXEL_FORMAT);

    planes = streamGetBufferSize(displayStream, planeSizes);

    buffers = calloc(opt.buffers, sizeof(*buffers));
    ASSERT(buffers);

    for (i = 0; i < opt.buffers; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, 0);
        ASSERT(buffers[i]);
    }

    streamSetBuffers(displayStream, opt.buffers, buffers);

    if (opt.display) {
        struct GDMDisplay *display;
        struct GDMDispFormat dispFmt;
        display = GDispOpen(DISPLAY_SOCK_PATH, 0);
        ASSERT(display);

        dispFmt.pixelformat = GCAM_PIXEL_FORMAT;
        dispFmt.width = DISPLAY_WIDTH;
        dispFmt.height = DISPLAY_HEIGHT;

        GDispSetFormat(display, &dispFmt);

        streamSetCallback(displayStream, displayCallback, display);
    }
    else {
        streamSetCallback(displayStream, displayCallback, NULL);
    }
    // end of display stream

    // video stream
    videoStream = streamOpen(STREAM_PORT_VIDEO);
    ASSERT(videoStream);

    streamSetFormat(videoStream,
            VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_PIXEL_FORMAT);

    planes = streamGetBufferSize(videoStream, planeSizes);

    opt.buffers = 2;
    buffers = calloc(opt.buffers, sizeof(*buffers));
    ASSERT(buffers);

    for (i = 0; i < opt.buffers; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, 0);
        ASSERT(buffers[i]);
    }

    streamSetBuffers(videoStream, opt.buffers, buffers);

    streamSetCallback(videoStream, videoCallback, NULL);
    // end of video stream

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
    // XXX Crop and scalings are based on SENSOR size.
    dxoFmt.crop.right = SENSOR_WIDTH - 1;
    dxoFmt.crop.bottom = SENSOR_HEIGHT - 1;
    DXOSetOutputFormat(dxo, DXO_OUTPUT_DISPLAY, &dxoFmt);

    dxoFmt.width = VIDEO_WIDTH;
    dxoFmt.height = VIDEO_HEIGHT;
    //dxoFmt.width = DISPLAY_WIDTH;
    //dxoFmt.height = DISPLAY_HEIGHT;
    dxoFmt.pixelFormat = V4L2_PIX_FMT_UYVY;
    dxoFmt.crop.left = 0;
    dxoFmt.crop.top = 0;
    // XXX Crop and scalings are based on SENSOR size.
    dxoFmt.crop.right = SENSOR_WIDTH - 1;
    dxoFmt.crop.bottom = SENSOR_HEIGHT - 1;
    DXOSetOutputFormat(dxo, DXO_OUTPUT_VIDEO, &dxoFmt);

    DXOSetOutputEnable(dxo, 1 << DXO_OUTPUT_DISPLAY, 1 << DXO_OUTPUT_DISPLAY);
    DXOSetOutputEnable(dxo, 1 << DXO_OUTPUT_VIDEO, 1 << DXO_OUTPUT_VIDEO);
    DXORunState(dxo, DXO_STATE_PREVIEW, 0);

    if (!opt.v4l2)
        pause();

    streamStart(displayStream);
    streamStart(videoStream);

    while (1)
        sleep (1);

    return 0;
}
