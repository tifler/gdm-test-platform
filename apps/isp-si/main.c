#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <signal.h>
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
#include "option.h"
#include "debug.h"

/*****************************************************************************/

#define CAPTURE_SIGNAL                      (SIGUSR1)

/*****************************************************************************/

static int mainStop;

/*****************************************************************************/

static void helpExit(int argc, char **argv)
{
    fprintf(stderr, "%s <ini file>\n", argv[0]);
    exit(EXIT_FAILURE);
}

/*****************************************************************************/

static int displayCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct GDMDisplay *disp = (struct GDMDisplay *)param;

    if (disp) {
        //DBG("DISPLAY CALLBACK: Buffer Index = %d, Send Frame.", index);
        GDispSendFrame(disp, buffer, 1000);
    }
    else {
        //DBG("DISPLAY CALLBACK: Buffer Index = %d", index);
    }

    return 0;
}

static int videoCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct GDMEncoder *encoder = (struct GDMEncoder *)param;

    if (encoder) {
        GEncEncodeFrame(encoder, buffer);
    }
    //DBG("VIDEO CALLBACK: Buffer Index = %d", index);
    return 0;
}

static int captureCallback(void *param, struct GDMBuffer *buffer, int index)
{
    //DBG("CAPTURE CALLBACK: Buffer Index = %d", index);
    return 0;
}

static int faceDetectCallback(void *param, struct GDMBuffer *buffer, int index)
{
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

    stream = streamOpen(portId);
    ASSERT(stream);

    dxoPortId = portId2DXOPortId[portId];

    streamSetFormat(stream, port->width, port->height, port->pixelFormat);

    planes = streamGetBufferSize(stream, planeSizes);

    buffers = calloc(port->bufferCount, sizeof(*buffers));
    ASSERT(buffers);

    for (i = 0; i < port->bufferCount; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, 0);
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
    ASSERT(signo == CAPTURE_SIGNAL);
    DBG("[PID=%d] Capture Signal(%d) received.", getpid(), signo);
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
    struct STREAM *streams[STREAM_PORT_COUNT];
    struct Option *opt;
    struct GDMDisplay *display = NULL;
    struct GDMEncoder *encoder = NULL;

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

    memset(streams, 0, sizeof(streams));
    // Display
    if (!opt->port[STREAM_PORT_DISPLAY].disable) {
        if (opt->global.display) {
            struct GDMDispFormat dispFmt;
            display = GDispOpen(opt->display.unixPath, 0);
            ASSERT(display);

            dispFmt.pixelformat = opt->port[STREAM_PORT_DISPLAY].pixelFormat;
            dispFmt.width = opt->port[STREAM_PORT_DISPLAY].width;
            dispFmt.height = opt->port[STREAM_PORT_DISPLAY].height;

            GDispSetFormat(display, &dispFmt);
        }
        streams[STREAM_PORT_DISPLAY] =
            createStream(opt, dxo, STREAM_PORT_DISPLAY, displayCallback, display);
        ASSERT(streams[STREAM_PORT_DISPLAY]);
    }

    // Video
    if (!opt->port[STREAM_PORT_VIDEO].disable) {
        if (opt->global.videoEncode) {
            struct GDMEncConfig encConf;
            encConf.width = opt->port[STREAM_PORT_VIDEO].width;
            encConf.height = opt->port[STREAM_PORT_VIDEO].height;
            encConf.gopSize = 10;
            encConf.bitrate = 4 * 1024 * 1024;
            encConf.fps = opt->sensor.fps;
            encConf.encFormat = opt->videoEncoder.format;
            encConf.useSWEnc = 0;

            encoder = GEncOpen(&encConf, opt->videoEncoder.basePath, 0);
            ASSERT(encoder);

            GEncStart(encoder);
        }
        streams[STREAM_PORT_VIDEO] = 
            createStream(opt, dxo, STREAM_PORT_VIDEO, videoCallback, encoder);
        ASSERT(streams[STREAM_PORT_VIDEO]);
    }

    // Capture
    if (!opt->port[STREAM_PORT_CAPTURE].disable) {
        streams[STREAM_PORT_CAPTURE] = 
            createStream(opt, dxo, STREAM_PORT_CAPTURE, captureCallback, NULL);
        ASSERT(streams[STREAM_PORT_CAPTURE]);
    }

    // FaceDetect
    if (!opt->port[STREAM_PORT_FACEDETECT].disable) {
        streams[STREAM_PORT_FACEDETECT] = 
            createStream(opt, dxo, STREAM_PORT_FACEDETECT, faceDetectCallback, NULL);
        ASSERT(streams[STREAM_PORT_FACEDETECT]);
    }

    DXORunState(dxo, opt->global.runState, 0);

    unblockSignals();

    installSigHandler();

    while (!mainStop)
        pause();

    for (i = 0; i < STREAM_PORT_COUNT; i++) {
        if (streams[i])
            destroyStream(streams[i]);
    }

    if (encoder) {
        GEncStop(encoder);
        GEncClose(encoder);
    }

    return 0;
}
