#include <stdlib.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/videodev2.h>

#include "stream.h"
#include "gdm-buffer.h"
#include "v4l2.h"
#include "image-info.h"
#include "gdm-isp-ioctl.h"
#include "debug.h"

/*****************************************************************************/

#define STREAM_TIMEOUT                      (1000)

#define STREAM_STAT_SET_FORMAT              (0x0001)
#define STREAM_STAT_SET_BUFFER              (0x0002)
#define STREAM_STAT_START                   (0x0004)

#define ARRAY_SIZE(a)                       (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct STREAM {
    int fd;
    int port;
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    int planeCount;
    int bufferCount;
    unsigned long status;
    struct GDMBuffer **buffers;
    struct v4l2_format fmt;

    pthread_t thread;
    int (*callback)(void *callbackParam, struct GDMBuffer *buffer, int index);
    void *callbackParam;

    // for debugging
    uint32_t frames;
    time_t lastSec;

    int isOutput;
    int showFPS;
};

static const char *streamName[] = {
    "CAPTURE", "VIDEO", "DISPLAY", "FACEDETECT", "VSENSOR"
};

/*****************************************************************************/

static void *streamThread(void *arg)
{
    int ret;
    int idx;
    time_t sec;
    struct pollfd pollfd;
    struct STREAM *stream = (struct STREAM *)arg;

    pthread_detach(pthread_self());

    pollfd.fd = stream->fd;
    pollfd.events = POLLIN;
    pollfd.revents = 0;

    while (stream->status & STREAM_STAT_START) {
        ret = poll(&pollfd, 1, STREAM_TIMEOUT);
        if (ret < 0) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        else if (ret == 0) {
            DBG("Timeout stream-%s.", streamName[stream->port]);
            continue;
        }

        if (!(stream->status & STREAM_STAT_START))
            break;

        idx = v4l2_dqbuf(pollfd.fd, stream->planeCount, stream->isOutput);
        if (idx < 0) {
            perror("v4l2_dqbuf");
            exit(0);
        }

        //DBG("Buffer Index = %d", idx);

        ret = 0;
        if (stream->callback)
            ret = stream->callback(
                    stream->callbackParam, stream->buffers[idx], idx);

        if (ret)
            break;

        ret = v4l2_qbuf(stream->fd, stream->width, stream->height,
                stream->buffers[idx], idx, stream->isOutput);

        if (ret) {
            DBG("=====> QBUF(%d) FAILED <=====", idx);
        }

        stream->frames++;
        time(&sec);
        if (sec != stream->lastSec) {
            if (stream->showFPS)
                DBG("Stream-%s: %d FPS",
                        streamName[stream->port], stream->frames);
            stream->lastSec = sec;
            stream->frames = 0;
        }
    }

    return stream->callbackParam;
}

/*****************************************************************************/

struct STREAM *streamOpen(int port, int showFPS)
{
    char path[256];
    struct STREAM *stream;

    ASSERT(port >= 0);
    ASSERT(port < STREAM_PORT_COUNT);

    stream = (struct STREAM *)calloc(1, sizeof(*stream));
    ASSERT(stream);

    stream->port = port;
    snprintf(path, sizeof(path) - 1, "/dev/video%d", port + 1);
    stream->fd = open(path, O_RDWR);
    ASSERT(stream->fd > 0);

    stream->showFPS = showFPS;
    stream->isOutput = (port == STREAM_PORT_VSENSOR ? 1 : 0);

    return stream;
}

void streamClose(struct STREAM *stream)
{
    ASSERT(stream);
    ASSERT(stream->fd > 0);
    close(stream->fd);
    free(stream);
}

int streamSetFormat(struct STREAM *stream,
        uint32_t width, uint32_t height, uint32_t pixelformat)
{
    int ret;
    struct GDMImageInfo imageInfo;

    stream->width = width;
    stream->height = height;
    stream->pixelformat = pixelformat;

    imageInfo.width = width;
    imageInfo.height = height;
    imageInfo.pixelFormat = pixelformat;
    imageInfo.align = 16;
    GDMGetImageInfo(&imageInfo);
    stream->planeCount = imageInfo.planeCountPhy;

    ret = v4l2_enum_fmt(stream->fd, stream->pixelformat, stream->isOutput);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(stream->fd, stream->width, stream->height,
            stream->pixelformat, &stream->fmt, stream->isOutput);
    ASSERT(ret == 0);

    stream->status |= STREAM_STAT_SET_FORMAT;

    return 0;
}

int streamGetBufferSize(struct STREAM *stream, uint32_t planeSizes[3])
{
    int i;
    int planes;
    struct v4l2_pix_format_mplane *pixmp;

    ASSERT(stream->status & STREAM_STAT_SET_FORMAT);

    pixmp = &stream->fmt.fmt.pix_mp;
    planes = pixmp->num_planes;
    for (i = 0; i < planes; i++)
        planeSizes[i] = pixmp->plane_fmt[i].sizeimage;

    return planes;
}

int streamGetBufferStride(struct STREAM *stream, uint32_t strides[3])
{
    int i;
    int planes;
    struct v4l2_pix_format_mplane *pixmp;

    ASSERT(stream->status & STREAM_STAT_SET_FORMAT);

    pixmp = &stream->fmt.fmt.pix_mp;
    planes = pixmp->num_planes;
    for (i = 0; i < planes; i++)
        strides[i] = pixmp->plane_fmt[i].bytesperline;

    return planes;
}

int streamSetBuffers(
        struct STREAM *stream, uint32_t bufferCount, struct GDMBuffer **buffers)
{
    int i;
    int ret;

    stream->bufferCount =
        v4l2_reqbufs(stream->fd, bufferCount, stream->isOutput);
    ASSERT(stream->bufferCount >= 0);

    if (stream->buffers)
        free(stream->buffers);

    if (stream->bufferCount > 0) {
        stream->buffers = calloc(stream->bufferCount, sizeof(*buffers));
        ASSERT(stream->buffers);
    }

    if (buffers) {
        for (i = 0; i < stream->bufferCount; i++) {
            stream->buffers[i] = buffers[i];
            ret = v4l2_qbuf(stream->fd, stream->width, stream->height,
                    buffers[i], i, stream->isOutput);
            ASSERT(ret == 0);
        }
    }

    stream->status |= STREAM_STAT_SET_BUFFER;
    
    return stream->bufferCount;
}

int streamSetCallback(struct STREAM *stream,
        int (*callback)(void *param, struct GDMBuffer *buffer, int index),
        void *callbackParam)
{
    ASSERT(stream);
    stream->callback = callback;
    stream->callbackParam = callbackParam;
    return 0;
}

int streamStart(struct STREAM *stream)
{
    int ret;

    ASSERT(stream);
    ASSERT(stream->status & (STREAM_STAT_SET_FORMAT|STREAM_STAT_SET_BUFFER));

    if (!stream->isOutput) {
        stream->status |= STREAM_STAT_START;
        ret = pthread_create(&stream->thread, NULL, streamThread, stream);
        ASSERT(ret == 0);
    }

    ret = v4l2_streamon(stream->fd, stream->isOutput);
    ASSERT(ret == 0);

    return ret;
}

void streamStop(struct STREAM *stream)
{
    int ret;

    ret = v4l2_streamoff(stream->fd, stream->isOutput);
    ASSERT(ret == 0);

    if (!stream->isOutput) {
        stream->status &= ~STREAM_STAT_START;
        pthread_join(stream->thread, NULL);
    }
}

void streamSetColorEffect(struct STREAM *stream, int effect)
{
    int ret;
    static int effectConvTable[] = {
        V4L2_COLORFX_NONE,
        V4L2_COLORFX_NEGATIVE,
        V4L2_COLORFX_SEPIA,
        V4L2_COLORFX_MONOCHROME,
        V4L2_COLORFX_DESATURATION,
        V4L2_COLORFX_COLORFILTER,
        V4L2_COLORFX_OVEREXPOSER,
        V4L2_COLORFX_POSTERIZER,
        V4L2_COLORFX_VINTAGE,
    };
    static const char *effectStringTable[] = {
        "NONE",
        "NEGATIVE",
        "SEPIA",
        "MONOCHROME",
        "DESATURATION",
        "COLORFILTER",
        "OVEREXPOSER",
        "POSTERIZER",
        "VINTAGE",
    };

    ASSERT(effect >= 0);
    ASSERT(effect < ARRAY_SIZE(effectConvTable));

    if (stream->isOutput)
        return;

    ret = v4l2_s_ctrl(stream->fd, V4L2_CID_COLORFX, effectConvTable[effect]);
    ASSERT(ret >= 0);
    DBG("Port[%d] Current Effect is %s", stream->port, effectStringTable[effect]);
}

void streamSendOutputBuffer(
        struct STREAM *stream, struct GDMBuffer *buffer, int index)
{
    int ret;

    ASSERT(stream->isOutput);
    ret = v4l2_qbuf(stream->fd,
            stream->width, stream->height, buffer, index, stream->isOutput);
    ASSERT(ret == 0);

    ret = v4l2_dqbuf(stream->fd, stream->planeCount, stream->isOutput);
    ASSERT(ret == index);
}

void streamSetVSensorTo(struct STREAM *stream, int vsensorTo)
{
    int ret;
    ASSERT(stream);
    ASSERT(stream->isOutput);
    ret = v4l2_s_ctrl(stream->fd, GISP_CID_SELECT_VS_TO, vsensorTo);
    ASSERT(ret >= 0);
}