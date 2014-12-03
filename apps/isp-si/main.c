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
#include "gisp/gisp-sensor.h"
#include "v4l2.h"
#include "gdm-buffer.h"
#include "stream.h"
#include "display.h"
#include "vid-enc.h"
#include "jpeg-enc.h"
#include "image-info.h"
#include "yuv-writer.h"
#include "file-io.h"
#include "option.h"
#include "media.h"
#include "debug.h"

/*****************************************************************************/

#define GISP_ALIGN                          (16)
#define CAPTURE_SIGNAL                      (SIGUSR1)

/*****************************************************************************/

struct PortContext {
    struct STREAM *stream;
    struct GDMDisplay *display;
    struct GDMEncoder *videoEncoder;
    struct GDMJPEGEncoder *jpegEncoder;
    struct GDMYUVWriter *yuvWriter;
    struct GDMImageInfo yuvInfo;
    uint32_t yuvCount;
    int enableEffect;
    int currEffect;

    int sendToVSensor;

    pthread_mutex_t captureLock;
    uint32_t captureCount;
    uint32_t captureIncrease;
    struct GDMJPEGEncConfig jpegConf;
    struct GDMBuffer *jpegDstBuffer;
    const char *jpegBasePath;
    int captureDisable;
};

/*****************************************************************************/

static int mainStop;
static struct PortContext portCtx[STREAM_PORT_COUNT] = {
    [STREAM_PORT_CAPTURE] = {
        .captureLock = PTHREAD_MUTEX_INITIALIZER,
    },
};

static struct DXO *DxO;

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

    if (port->sendToVSensor) {
        ASSERT(portCtx[STREAM_PORT_VSENSOR].stream);
        streamSendOutputBuffer(
                portCtx[STREAM_PORT_VSENSOR].stream, buffer, index);
    }

    return 0;
}

static int videoCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;

    if (port->videoEncoder) {
        GEncEncodeFrame(port->videoEncoder, buffer);
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
    
    if (port->sendToVSensor) {
        ASSERT(portCtx[STREAM_PORT_VSENSOR].stream);
        streamSendOutputBuffer(
                portCtx[STREAM_PORT_VSENSOR].stream, buffer, index);
    }

    return 0;
}

static void writeJpeg(const struct GDMBuffer *buffer,
        struct GDMJPEGEncConfig *conf, const char *basePath)
{
    int fd;
    char path[256];
    static int jpegID = 0;

    ASSERT(buffer);
    ASSERT(conf);
    ASSERT(basePath);

    snprintf(path, sizeof(path) - 1, "%s/CAP-%dx%d-%d.jpg",
            basePath, conf->image.width, conf->image.height, jpegID++);

    DBG("JPEG path = %s", path);
    fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0644);
    DBG("fd = %d", fd);
    ASSERT(fd >= 0);

    safeWrite(fd, buffer->plane[0].base, buffer->plane[0].used);

    close(fd);
}

static int captureCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;

    pthread_mutex_lock(&port->captureLock);
    if (port->captureCount > 0) {
        // When first callback is called, just change state.
        if (DXOGetState(DxO) != DXO_STATE_CAPTURE_A) {
            DXORunState(DxO, DXO_STATE_CAPTURE_A, port->captureCount);
        }
        else {
            port->jpegDstBuffer->plane[0].used = 0;
            DBG("JPEG Config: (%d, %d, 0x%x)",
                    port->jpegConf.image.width,
                    port->jpegConf.image.height,
                    port->jpegConf.image.pixelFormat);
            GJPEGEncEncodeFrame(port->jpegEncoder,
                    buffer, port->jpegDstBuffer, &port->jpegConf);
            writeJpeg(port->jpegDstBuffer, &port->jpegConf, port->jpegBasePath);

            port->captureCount--;
        }
    }

    if (port->sendToVSensor) {
        ASSERT(portCtx[STREAM_PORT_VSENSOR].stream);
        streamSendOutputBuffer(
                portCtx[STREAM_PORT_VSENSOR].stream, buffer, index);
    }

    if (port->captureCount == 0 && DXOGetState(DxO) == DXO_STATE_CAPTURE_A)
        DXORunState(DxO, DXO_STATE_PREVIEW, 0);

    pthread_mutex_unlock(&port->captureLock);

    DBG("CAPTURE CALLBACK: Buffer Index = %d", index);
    return 0;
}

static int faceDetectCallback(void *param, struct GDMBuffer *buffer, int index)
{
    struct PortContext *port = (struct PortContext *)param;
    //DBG("FD CALLBACK: Buffer Index = %d", index);
   
    if (port->sendToVSensor) {
        ASSERT(portCtx[STREAM_PORT_VSENSOR].stream);
        streamSendOutputBuffer(
                portCtx[STREAM_PORT_VSENSOR].stream, buffer, index);
    }

    return 0;
}

/*****************************************************************************/

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
    port->captureCount += port->captureIncrease;
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

static struct STREAM *initVSensorStream(struct Option *opt, int portId)
{
    struct STREAM *stream;
    const struct OptionPort *port;

    DBG("VSensor From Port %d", portId);
    port = &opt->port[portId];
    ASSERT(!port->disable);

    portCtx[portId].sendToVSensor = 1;

    stream = streamOpen(STREAM_PORT_VSENSOR, opt->global.showFPS);
    ASSERT(stream);

    streamSetFormat(stream, port->width, port->height, port->pixelFormat);

    // buffer count should be the same as feeder stream's one.
    streamSetBuffers(stream,
            opt->port[portId].bufferCount, (struct GDMBuffer **)0);

    streamSetVSensorTo(stream, opt->global.vSensor);
    streamStart(stream);
    return stream;
}

static struct STREAM *initVSensor(struct Option *opt)
{
    struct STREAM *stream = (struct STREAM *)NULL;
    struct PortContext *port = &portCtx[STREAM_PORT_VSENSOR];

    switch (opt->global.vSensor) {
        case 3:
            // DXO -> WDMA & VS -> BT601
            // XXX bt601PortID가 지정되어야 한다.
            //     WDMA[portId] -> Memory -> RDMA(VSENSOR) -> BT601
            stream = initVSensorStream(opt, opt->global.bt601PortId % 4);
            ASSERT(stream);
            break;

        case 2:
            // VS -> DXO:
            // 이 경우는 별도 메모리를 할당하고 영상을 422P1으로 저장한 후
            // RDMA가 그것을 읽도록 한다.
            // 문제는 DxO에서 아마도 Bayer 영상을 받을텐데
            // Bayer를 넣어줘야 하는가? 확인 필요
            // TODO
            // XXX 이 기능은 지원하지 않기로 SoC와 합의함 
            ASSERT(! "Only support VSENSOR=3 mode");
            break;

        case 4 ... 7:
            // VS -> WDMA
            // 이 경우도 2번경우와 마찬가지로 외부 영상을 넣어준다.
            // TODO
           
            // XXX bt601PortID가 지정되어야 한다.
            // XXX 이 기능은 지원하지 않기로 SoC와 합의함 
            ASSERT(opt->global.bt601PortId >= 0);
            ASSERT(! "Only support VSENSOR=3 mode");
            break;

        default:
            break;
    }
    port->stream = stream;
    return stream;
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

    // ISP
    isp = ISPInit();

    // SIF
    DBG("bt601PortId = %d", opt->global.bt601PortId);
    sif = SIFInit();

    sifConf.id = opt->sensor.id;
    sifConf.width = opt->sensor.width;
    sifConf.height = opt->sensor.height;
    sifConf.fps = opt->sensor.fps;
    // VIGEN의 경우 PCLK을 SIF 내부적으로 Inversion 시킨다. 
    sifConf.invPCLK = (opt->sensor.id == SENSOR_ID_VIGEN ? 1 : 0);
    DBG("SIF = (%d x %d x %d FPS)", sifConf.width, sifConf.height, sifConf.fps);
    SIFSetConfig(sif, &sifConf);

    // DXO
    memset(&conf, 0, sizeof(conf));
    conf.sysFreqMul = opt->global.sysClkMul;
    conf.sysFreqDiv = opt->global.sysClkDiv;
    conf.frmTimeMul = 4;
    conf.frmTimeDiv = 1;
    conf.needPostEvent = opt->global.needPostEvent;
    conf.estimateIRQ = opt->global.estimateIRQ;
    conf.sensorId = opt->sensor.id;
    DBG("SensorId = %d", conf.sensorId);

    DxO = dxo = DXOInit(&conf);

    //ctrl.input = DXO_INPUT_SOURCE_FRONT;
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

            port->videoEncoder = GEncOpen(&encConf, opt->videoEncoder.basePath, 0);
            ASSERT(port->videoEncoder);

            GEncStart(port->videoEncoder);
        }

        if (opt->port[STREAM_PORT_VIDEO].writeYUV > 0) {
            port->yuvWriter = GYUVWriterOpen("video.yuv");
            ASSERT(port->yuvWriter);
            port->yuvCount = opt->port[STREAM_PORT_VIDEO].writeYUV;

            port->yuvInfo.width = opt->port[STREAM_PORT_VIDEO].width;
            port->yuvInfo.height = opt->port[STREAM_PORT_VIDEO].height;
            port->yuvInfo.pixelFormat = opt->port[STREAM_PORT_VIDEO].pixelFormat;
            port->yuvInfo.align = GISP_ALIGN;
            GDMGetImageInfo(&port->yuvInfo);
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

        if (opt->global.captureEncode) {
            unsigned int planeSizes[3];
            port->jpegConf.image.width = opt->port[STREAM_PORT_CAPTURE].width;
            port->jpegConf.image.height = opt->port[STREAM_PORT_CAPTURE].height;
            port->jpegConf.image.pixelFormat =
                opt->port[STREAM_PORT_CAPTURE].pixelFormat;
            port->jpegConf.image.align = GISP_ALIGN;
            GDMGetImageInfo(&port->jpegConf.image);
            port->jpegConf.quality = opt->jpegEncoder.ratio;

            if (opt->global.changeState) {
                // If ChangeState is enabled, capture thread doesn't have to
                // change state itself. To do this, set captureCount to zero.
                DBG("Change DxO State is enabled");
                port->captureIncrease = 0;
            }
            else  {
                port->captureIncrease = opt->jpegEncoder.captureCount;
                DBG("Change DxO State is not enabled. CaptureInc = %d",
                        port->captureIncrease);
            }

            memset(planeSizes, 0, sizeof(planeSizes));
            planeSizes[0] =
                port->jpegConf.image.width * port->jpegConf.image.height;
            port->jpegDstBuffer = allocContigMemory(1, planeSizes, GDM_BUF_FLAG_MAP);
            ASSERT(port->jpegDstBuffer);

            port->jpegEncoder = GJPEGEncOpen();
            ASSERT(port->jpegEncoder);

            port->jpegBasePath = opt->jpegEncoder.basePath;
            ASSERT(port->jpegBasePath);
    
        }
    }

    // FaceDetect
    if (!opt->port[STREAM_PORT_FACEDETECT].disable) {
        port = &portCtx[STREAM_PORT_FACEDETECT];
        port->stream = createStream(opt, dxo,
                STREAM_PORT_FACEDETECT, faceDetectCallback, port);
        ASSERT(port->stream);
    }

    // VSensor
    initVSensor(opt);

    DXORunState(dxo, opt->global.runState, 0);

    unblockSignals();

    installSigHandler();

    while (!mainStop) {
        int state;
        state = DXOGetState(dxo);
        DBG("===== CURRENT DXO STATE = %d =====", state);
        changeEffect();
        sleep(1);
        if (opt->global.changeState) {
            sleep(1);
            if (state == DXO_STATE_IDLE)
                DXORunState(dxo, DXO_STATE_PREVIEW, 0);
            else if (state == DXO_STATE_PREVIEW)
                DXORunState(dxo, DXO_STATE_VIDEOREC, 0);
            else if (state == DXO_STATE_VIDEOREC)
                DXORunState(dxo, DXO_STATE_CAPTURE_A, 1);
            else if (state == DXO_STATE_CAPTURE_A)
                DXORunState(dxo, DXO_STATE_CAPTURE_B, 1);
            else //if (state == DXO_STATE_CAPTURE_B)
                DXORunState(dxo, DXO_STATE_IDLE, 0);
        }
    }

    for (i = 0; i < STREAM_PORT_COUNT; i++) {
        port = &portCtx[i];
        if (port->stream)
            destroyStream(port->stream);

        if (port->videoEncoder) {
            GEncStop(port->videoEncoder);
            GEncClose(port->videoEncoder);
        }
    }

    return 0;
}
