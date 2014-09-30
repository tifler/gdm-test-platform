#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gisp/gisp-isp.h"
#include "gisp/gisp-sif.h"
#include "gisp/gisp-dxo.h"
#include "v4l2.h"
#include "gdm-buffer.h"
#include "stream.h"
#include "display.h"
#include "encoder.h"
#include "yuv-writer.h"
#include "option.h"
#include "debug.h"

/*****************************************************************************/

#define CAPTURE_SIGNAL                      (SIGUSR1)

/*****************************************************************************/

struct PortContext {
    struct STREAM *stream;
    struct GDMDisplay *display;
    struct GDMEncoder *encoder;
    struct GDMYUVWriter *yuvWriter;
    struct GDMImageInfo yuvInfo;
    uint32_t yuvCount;
    int enableEffect;
    int currEffect;

    pthread_mutex_t captureLock;
    uint32_t captureCount;
};

/*****************************************************************************/

static int mainStop;
static struct PortContext portCtx[STREAM_PORT_COUNT] = {
    [STREAM_PORT_CAPTURE] = {
        .captureLock = PTHREAD_MUTEX_INITIALIZER,
    },
};

/*****************************************************************************/

static void helpExit(int argc, char **argv)
{
    fprintf(stderr, "%s <ini file>\n", argv[0]);
    exit(EXIT_FAILURE);
}

/*****************************************************************************/

static int displayCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;

    if (port->display) {
        //DBG("DISPLAY CALLBACK: Buffer Index = %d, Send Frame.", index);
        GDispSendFrame(port->display, buffer, 1000);
    }
    else {
        //DBG("DISPLAY CALLBACK: Buffer Index = %d", index);
    }

    return 0;
}

static int videoCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;

    if (port->encoder) {
        GEncEncodeFrame(port->encoder, buffer);
    }

    if (port->yuvWriter) {
        if (port->yuvCount > 0) {
            GYUVWriterWrite(port->yuvWriter, &port->yuvInfo, buffer);
            port->yuvCount--;
        }
        else {
            GYUVWriterClose(port->yuvWriter);
            port->yuvWriter = (struct GDMYUVWriter *)NULL;
        }
    }
    //DBG("VIDEO CALLBACK: Buffer Index = %d", index);
    return 0;
}

static int captureCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;

    pthread_mutex_lock(&port->captureLock);
    if (port->captureCount > 0) {
        DBG("CaptureCount = %d", port->captureCount);
        port->captureCount--;
        // TODO capture image and save it.
    }
    pthread_mutex_unlock(&port->captureLock);
    //DBG("CAPTURE CALLBACK: Buffer Index = %d", index);
    return 0;
}

static int faceDetectCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;
    //DBG("FD CALLBACK: Buffer Index = %d", index);
    return 0;
}

static struct STREAM *createStream(
        const struct Option *opt, struct DXO *dxo, int portId,
        int (*callback)(void *, struct GDMBuffer *, int), void *callbackParam)
{
    int i;
    int planes;
    int dxoPortId;
    unsigned int planeSizes[3];
    struct GDMBuffer **buffers;
    struct STREAM *stream;
    const struct OptionPort *port;
    struct DXOOutputFormat dxoFmt;

    static int portId2DXOPortId[] = {
        DXO_OUTPUT_CAPTURE,
        DXO_OUTPUT_VIDEO,
        DXO_OUTPUT_DISPLAY,
        DXO_OUTPUT_FD,
    };

    port = &opt->port[portId];
    ASSERT(!port->disable);

    stream = streamOpen(portId, opt->global.showFPS);
    ASSERT(stream);

    dxoPortId = portId2DXOPortId[portId];

    streamSetFormat(stream, port->width, port->height, port->pixelFormat);

    planes = streamGetBufferSize(stream, planeSizes);

    buffers = calloc(port->bufferCount, sizeof(*buffers));
    ASSERT(buffers);

    for (i = 0; i < port->bufferCount; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, GDM_BUF_FLAG_MAP);
        ASSERT(buffers[i]);
    }

    streamSetBuffers(stream, port->bufferCount, buffers);
    streamSetCallback(stream, callback, callbackParam);

    dxoFmt.width = port->width;
    dxoFmt.height = port->height;
    dxoFmt.pixelFormat = port->pixelFormat;
    dxoFmt.crop.left = port->cropLeft;
    dxoFmt.crop.top = port->cropTop;
    dxoFmt.crop.right = port->cropRight;
    dxoFmt.crop.bottom = port->cropBottom;
    DBG("dxoFmt = (%dx%d) (%d, %d, %d, %d)",
            dxoFmt.width, dxoFmt.height,
            dxoFmt.crop.left,
            dxoFmt.crop.top,
            dxoFmt.crop.right,
            dxoFmt.crop.bottom);
    DXOSetOutputFormat(dxo, dxoPortId, &dxoFmt);
    DXOSetOutputEnable(dxo, 1 << dxoPortId, 1 << dxoPortId);

    streamStart(stream);

    return stream;
}

static void destroyStream(struct STREAM *stream)
{
    streamStop(stream);
    streamClose(stream);
}

static void captureSignalHandler(int signo)
{
    struct PortContext *port = &portCtx[STREAM_PORT_CAPTURE];

    ASSERT(signo == CAPTURE_SIGNAL);
    DBG("[PID=%d] Capture Signal(%d) received.", getpid(), signo);

    pthread_mutex_lock(&port->captureLock);
    port->captureCount++;
    DBG("Increase capture count: %d", port->captureCount);
    pthread_mutex_unlock(&port->captureLock);
}

static void closeHandler(int signo)
{
    ASSERT(signo == SIGINT);
    mainStop = 1;
}

/*****************************************************************************/

static void blockSignals(void)
{
    int ret;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, CAPTURE_SIGNAL);
    ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
    ASSERT(ret == 0);
}

static void unblockSignals(void)
{
    int ret;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, CAPTURE_SIGNAL);
    ret = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    ASSERT(ret == 0);
}

static void installSigHandler(void)
{
    int ret;
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);

    act.sa_handler = captureSignalHandler;
    ret = sigaction(CAPTURE_SIGNAL, &act, NULL);
    ASSERT(ret == 0);

    act.sa_handler = closeHandler;
    ret = sigaction(SIGINT, &act, NULL);
    ASSERT(ret == 0);
}

static void opt2ImageInfo(
        const struct Option *opt, struct GDMImageInfo *info, int streamId)
{
    int i;

    ASSERT(opt);
    ASSERT(info);

    memset(info, 0, sizeof(*info));

    info->width = opt->port[streamId].width;
    info->height = opt->port[streamId].height;

    switch (opt->port[streamId].pixelFormat) {
        case V4L2_PIX_FMT_YUV420M:
            info->planeCount = 3;
            info->plane[0].stride = ((info->width + 7) >> 3) << 3;
            info->plane[0].bpl = opt->port[streamId].width;
            info->plane[0].lines = info->height;
            info->plane[1].stride = ((info->width + 7) >> 3) << 2;
            info->plane[1].bpl = opt->port[streamId].width >> 1;
            info->plane[1].lines = info->height >> 1;
            info->plane[2].stride = ((info->width + 7) >> 3) << 2;
            info->plane[2].bpl = opt->port[streamId].width >> 1;
            info->plane[2].lines = info->height >> 1;
            break;

        default:
            ASSERT(! "Not implemented.");
            break;
    }

    for (i = 0; i < info->planeCount; i++) {
        DBG("Plane[%d].stride = %u, bpl = %u",
                i, info->plane[i].stride, info->plane[i].bpl);
    }
}

static void changeEffect(void)
{
    int i;
    struct PortContext *port;

    for (i = 0; i < STREAM_PORT_COUNT; i++) {
        port = &portCtx[i];
        if (port->stream && port->enableEffect) {
            streamSetColorEffect(port->stream, port->currEffect);
            port->currEffect = (port->currEffect + 1) % 9;
        }
    }
}

/*****************************************************************************/

int main(int argc, char **argv)
{
    int i;
    struct DXOSystemConfig conf;
    struct DXOControl ctrl;
    struct SIFConfig sifConf;
    struct ISP *isp;
    struct SIF *sif;
    struct DXO *dxo;
    struct Option *opt;
    struct PortContext *port;

    if (argc != 2)
        helpExit(argc, argv);

    opt = createOption(argv[1]);
    ASSERT(opt);

    isp = ISPInit();
    sif = SIFInit();

    sifConf.id = opt->sensor.id;
    sifConf.width = opt->sensor.width;
    sifConf.height = opt->sensor.height;
    sifConf.fps = opt->sensor.fps;
    DBG("SIF = (%d x %d x %d FPS)", sifConf.width, sifConf.height, sifConf.fps);
    SIFSetConfig(sif, &sifConf);

    memset(&conf, 0, sizeof(conf));
    conf.sysFreqMul = opt->global.sysClkMul;
    conf.sysFreqDiv = opt->global.sysClkDiv;
    conf.frmTimeMul = 4;
    conf.frmTimeDiv = 1;
    conf.needPostEvent = opt->global.needPostEvent;
    conf.estimateIRQ = opt->global.estimateIRQ;
    conf.sensorId = opt->sensor.id;

    dxo = DXOInit(&conf);

    ctrl.input = DXO_INPUT_SOURCE_FRONT;
    ctrl.hMirror = 0;
    ctrl.vFlip = 0;
    ctrl.enableTNR = 0;
    ctrl.fpsMul = 8;
    ctrl.fpsDiv = 1;
    DXOSetControl(dxo, &ctrl);

    // Before create threads, we shoud block SIGUSR1 because threads inherit
    // parent(main thread)'s signal mask.
    blockSignals();

    // Display
    if (!opt->port[STREAM_PORT_DISPLAY].disable) {
        port = &portCtx[STREAM_PORT_DISPLAY];
        if (opt->global.display) {
            struct GDMDispFormat dispFmt;
            port->display = GDispOpen(opt->display.unixPath, 0);
            ASSERT(port->display);

            dispFmt.pixelformat = opt->port[STREAM_PORT_DISPLAY].pixelFormat;
            dispFmt.width = opt->port[STREAM_PORT_DISPLAY].width;
            dispFmt.height = opt->port[STREAM_PORT_DISPLAY].height;

            GDispSetFormat(port->display, &dispFmt);
        }
        port->stream = createStream(opt, dxo,
                STREAM_PORT_DISPLAY, displayCallback, port);
        ASSERT(port->stream);
        port->enableEffect = opt->port[STREAM_PORT_DISPLAY].enableEffect;
    }

    // Video
    if (!opt->port[STREAM_PORT_VIDEO].disable) {
        port = &portCtx[STREAM_PORT_VIDEO];
        if (opt->global.videoEncode) {
            struct GDMEncConfig encConf;
            encConf.width = opt->port[STREAM_PORT_VIDEO].width;
            encConf.height = opt->port[STREAM_PORT_VIDEO].height;
            encConf.gopSize = 10;
            encConf.bitrate = 4 * 1024 * 1024;
            encConf.fps = opt->sensor.fps;
            encConf.encFormat = opt->videoEncoder.format;
            encConf.useSWEnc = 0;

            port->encoder = GEncOpen(&encConf, opt->videoEncoder.basePath, 0);
            ASSERT(port->encoder);

            GEncStart(port->encoder);
        }

        if (opt->port[STREAM_PORT_VIDEO].writeYUV > 0) {
            port->yuvWriter = GYUVWriterOpen("video.yuv");
            ASSERT(port->yuvWriter);
            port->yuvCount = opt->port[STREAM_PORT_VIDEO].writeYUV;
            opt2ImageInfo(opt, &port->yuvInfo, STREAM_PORT_VIDEO);
        }

        port->stream = createStream(opt, dxo,
                STREAM_PORT_VIDEO, videoCallback, port);
        ASSERT(port->stream);
        port->enableEffect = opt->port[STREAM_PORT_VIDEO].enableEffect;
    }

    // Capture
    if (!opt->port[STREAM_PORT_CAPTURE].disable) {
        port = &portCtx[STREAM_PORT_CAPTURE];
        port->stream = 
            createStream(opt, dxo, STREAM_PORT_CAPTURE, captureCallback, port);
        ASSERT(port->stream);
        port->enableEffect = opt->port[STREAM_PORT_CAPTURE].enableEffect;
    }

    // FaceDetect
    if (!opt->port[STREAM_PORT_FACEDETECT].disable) {
        port = &portCtx[STREAM_PORT_VIDEO];
        port->stream = createStream(opt, dxo,
                STREAM_PORT_FACEDETECT, faceDetectCallback, port);
        ASSERT(port->stream);
    }

    changeEffect();

    DXORunState(dxo, opt->global.runState, 0);

    unblockSignals();

    installSigHandler();

    while (!mainStop) {
        changeEffect();
        sleep(1);
    }

    for (i = 0; i < STREAM_PORT_COUNT; i++) {
        port = &portCtx[i];
        if (port->stream)
            destroyStream(port->stream);

        if (port->encoder) {
            GEncStop(port->encoder);
            GEncClose(port->encoder);
        }
    }

    return 0;
}
