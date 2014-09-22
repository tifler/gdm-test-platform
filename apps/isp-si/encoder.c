#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include "encoder.h"
#include "api/mme_c_api.h"
#include "gdm-buffer.h"
#include "debug.h"

/*****************************************************************************/

#define ARRAY_SIZE(a)               (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct GDMEncoder {
    int threadModel;
    pthread_t thid;
    void *encHandle;
    void *muxHandle;
    struct {
        unsigned char *stream;
        unsigned int streamSize;
        unsigned char *config;
        unsigned int configSize;
    } iBuffer;
    unsigned int frameCount;
    int stopRequest;

    struct GDMBuffer *frame;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

/*****************************************************************************/

static void makeFileName(
        char *buffer, int buflen, const char *prefix, unsigned int fmt)
{
    int len;
    const char *suffix = NULL;

    len = snprintf(buffer, buflen - 1, "%s", prefix);
    switch (fmt) {
        case MME_FOURCC_VIDEO_H264:
            suffix = ".yuv.h264.mp4";
            break;
        case MME_FOURCC_VIDEO_MPEG4:
            suffix = ".yuv.mpeg4.mp4";
            break;
        case MME_FOURCC_VIDEO_H263:
            suffix = ".yuv.h263.mov";
            break;
        default:
            ASSERT(! "Not supported format.");
            break;
    }

    ASSERT(suffix);

    buflen -= len;
    len = snprintf(&buffer[len], buflen - 1, "%s", suffix);
}

// must be called with lock or non-thread model
static void encodeFrame(struct GDMEncoder *e)
{
    int i;
    int ionFds[3];
    int memOffset[3];
    int streamSize;
    int configSize;
    unsigned int encFlag;

    ASSERT(e->frame);

    for (i = 0; i < 3; i++) {
        ionFds[i] = e->frame->plane[i].fd;
        memOffset[i] = 0;
    }

    mme_video_encoder_run(
            e->encHandle, ionFds, memOffset,
            e->iBuffer.stream, e->iBuffer.streamSize, &streamSize, &encFlag,
            e->iBuffer.config, e->iBuffer.configSize, &configSize);

    if (configSize > 0)
        mme_muxer_add_video_config(
                e->muxHandle, e->iBuffer.config, configSize);

    if (streamSize > 0)
        mme_muxer_add_video_data(
                e->muxHandle, e->iBuffer.stream, streamSize, encFlag);

    e->frameCount++;
}

// sync mode encoding thread
static void *encoderThreadSync(void *param)
{
    struct GDMEncoder *e = (struct GDMEncoder *)param;

    for ( ; ; ) {
        // wait frameQueud or stop requested
        pthread_mutex_lock(&e->lock);
        while (!e->frame && !e->stopRequest)
            pthread_cond_wait(&e->cond, &e->lock);

        if (e->stopRequest) {
            DBG("Encoder thread got STOP REQUEST.");
            pthread_mutex_unlock(&e->lock);
            break;
        }

        ASSERT(e->frame);

        encodeFrame(e);

        e->frame = NULL;

        // wakeup wait thread
        pthread_cond_broadcast(&e->cond);
        pthread_mutex_unlock(&e->lock);
    }

    return e;
}

static void startEncoderThread(struct GDMEncoder *e)
{
    int ret;
    ASSERT(e);
    ret = pthread_create(&e->thid, NULL, encoderThreadSync, e);
    ASSERT(ret == 0);
}

static void stopEncoderThread(struct GDMEncoder *e)
{
    int ret;

    ASSERT(e);

    pthread_mutex_lock(&e->lock);
    e->stopRequest = 1;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);

    ret = pthread_join(e->thid, NULL);
    ASSERT(ret == 0);
}

/*****************************************************************************/

struct GDMEncoder *GEncOpen(
        const struct GDMEncConfig *conf, const char *path, unsigned long flags)
{
    struct GDMEncoder *e;
    struct mme_muxer_config muxConf;
    struct mme_video_encoder_config *encConf;
    static unsigned int encFmt2MMEFmt[] = {
        MME_FOURCC_VIDEO_H264,
        MME_FOURCC_VIDEO_MPEG4,
        MME_FOURCC_VIDEO_H263,
    };

    ASSERT(conf);
    ASSERT(path);

    ASSERT(conf->width >= 16);
    ASSERT(conf->width <= 1920);
    ASSERT(conf->height >= 16);
    ASSERT(conf->height <= 1080);

    e = calloc(1, sizeof(*e));
    ASSERT(e);

    if (flags & GDM_ENC_FLAG_THREAD)
        e->threadModel = 1;

    pthread_cond_init(&e->cond, (pthread_condattr_t *)0);
    pthread_mutex_init(&e->lock, (pthread_mutexattr_t *)0);

    e->iBuffer.streamSize = conf->width * conf->height * 3 / 2;
    e->iBuffer.stream = malloc(e->iBuffer.streamSize);
    ASSERT(e->iBuffer.stream);
    e->iBuffer.configSize = 4096;
    e->iBuffer.config = malloc(e->iBuffer.configSize);
    ASSERT(e->iBuffer.config);

    memset(&muxConf, 0, sizeof(muxConf));
    encConf = &muxConf.video_encoder_config;

    encConf->width = (int)conf->width;
    encConf->height = (int)conf->height;
    encConf->format = encFmt2MMEFmt[conf->encFormat];
    encConf->fps = (int)conf->fps;
    encConf->bitrate = (int)conf->bitrate;
    encConf->idr_period = (int)conf->gopSize;
    encConf->sw_codec_use = conf->useSWEnc ? 1 : 0;

    makeFileName(
            muxConf.filename, sizeof(muxConf.filename), path, encConf->format);

    DBG("Format=0x%x", encConf->format);
    e->encHandle = mme_video_encoder_create_object(encConf);
    ASSERT(e->encHandle);

    e->muxHandle = mme_muxer_create_object(&muxConf);
    ASSERT(e->muxHandle);

    return e;
}

void GEncStart(struct GDMEncoder *e)
{
    if (e->threadModel)
        startEncoderThread(e);
}

void GEncStop(struct GDMEncoder *e)
{
    if (e->threadModel)
        stopEncoderThread(e);
}

void GEncClose(struct GDMEncoder *e)
{
    ASSERT(e);
    mme_video_encoder_destroy_object(e->encHandle);
    mme_muxer_destroy_object(e->muxHandle);
    pthread_cond_destroy(&e->cond);
    pthread_mutex_destroy(&e->lock);
    DBG("Encoding Frames = %u", e->frameCount);
    free(e);
}

int GEncEncodeFrame(struct GDMEncoder *e, struct GDMBuffer *frame)
{
    if (e->threadModel) {
        pthread_mutex_lock(&e->lock);

        ASSERT(!e->frame);
        e->frame = frame;
        pthread_cond_broadcast(&e->cond);

        while (e->frame)
            pthread_cond_wait(&e->cond, &e->lock);

        ASSERT(!e->frame);

        pthread_mutex_unlock(&e->lock);
    }
    else {
        e->frame = frame;
        encodeFrame(e);
    }

    return 0;
}
