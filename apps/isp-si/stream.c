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
#include "debug.h"

/*****************************************************************************/

#define STREAM_TIMEOUT                      (1000)

#define STREAM_STAT_SET_FORMAT              (0x0001)
#define STREAM_STAT_SET_BUFFER              (0x0002)
#define STREAM_STAT_START                   (0x0004)

/*****************************************************************************/

struct STREAM {
    int fd;
    int port;
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
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
};

static const char *streamName[] = {
    "CAPTURE", "VIDEO", "DISPLAY", "FACEDETECT"
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

        idx = v4l2_dqbuf(pollfd.fd, 3);
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

        ret = v4l2_qbuf(stream->fd,
                stream->width, stream->height, stream->buffers[idx], idx);

        if (ret) {
            DBG("=====> QBUF(%d) FAILED <=====", idx);
        }

        stream->frames++;
        time(&sec);
        if (sec != stream->lastSec) {
            DBG("Stream-%s: %d FPS", streamName[stream->port], stream->frames);
            stream->lastSec = sec;
            stream->frames = 0;
        }
    }

    return stream->callbackParam;
}

/*****************************************************************************/

struct STREAM *streamOpen(int port)
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

    stream->width = width;
    stream->height = height;
    stream->pixelformat = pixelformat;

    ret = v4l2_enum_fmt(stream->fd, stream->pixelformat);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(stream->fd,
            stream->width, stream->height, stream->pixelformat, &stream->fmt);
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

int streamSetBuffers(
        struct STREAM *stream, uint32_t bufferCount, struct GDMBuffer **buffers)
{
    int i;
    int ret;

    stream->bufferCount = v4l2_reqbufs(stream->fd, bufferCount);
    ASSERT(stream->bufferCount >= 0);

    if (stream->buffers)
        free(stream->buffers);

    if (stream->bufferCount > 0) {
        stream->buffers = calloc(stream->bufferCount, sizeof(*buffers));
        ASSERT(stream->buffers);
    }

    for (i = 0; i < stream->bufferCount; i++) {
        stream->buffers[i] = buffers[i];
        ret = v4l2_qbuf(
                stream->fd, stream->width, stream->height, buffers[i], i);
        ASSERT(ret == 0);
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

    stream->status |= STREAM_STAT_START;

    ret = pthread_create(&stream->thread, NULL, streamThread, stream);
    ASSERT(ret == 0);

    ret = v4l2_streamon(stream->fd);
    ASSERT(ret == 0);

    return ret;
}

void streamStop(struct STREAM *stream)
{
    int ret;

    ret = v4l2_streamoff(stream->fd);
    ASSERT(ret == 0);

    stream->status &= ~STREAM_STAT_START;
    pthread_join(stream->thread, NULL);
}
