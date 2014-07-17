/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>

#include "mscaler.h"
#include "debug.h"

/*****************************************************************************/

#define SCALER_FLAG_SRC_FORMAT              (0x0001)
#define SCALER_FLAG_SRC_DATA                (0x0002)
#define SCALER_FLAG_DST_FORMAT              (0x0010)
#define SCALER_FLAG_DST_DATA                (0x0020)
#define SCALER_FLAG_START_STREAMING         (0x8000)

#define ARRAY_SIZE(a)                       (sizeof(a) / sizeof(a[0]))
#define MAKE_COLOR_FORMAT(fmt)              .name = #fmt, .pixelformat = fmt

#define DBG_VAR(fmt, var)                   DBG("VAR: " #var "=" fmt, var)

/*****************************************************************************/

/* SRC vs DST
 *
 * SRC
 *   Data의 흐름은 memory to device 이다.
 *   따라서 V4L2 기준으로 OUTPUT에 해당한다.
 *
 * DST
 *   Data의 흐름은 device to memory 이다.
 *   따라서 V4L2 기준으로 CAPTURE에 해당한다.
 */

/*
 * Driver구현시 고려사항
 *  - open()콜시 context를 생성한다.
 *  - 각 context는 서로 독립이다.
 *  - 이 context를 이용해서 여러 process, thread에서 동시 사용가능하게 구현한다.
 *
 * Library 구현시 고려사항
 *  - MScaler는 기본적으로 thread safe 하지 않게 디자인 한다.
 *  - 단, 서로 다른 MScaler에 대해서는 독립적이기 때문에
 *    만약 여러 스레드에서 Scaler를 사용하고자 한다면, 여러개의 instance를
 *    생성하면 된다.(즉 scaler를 필요로 하는 thread가 3개라면, 각각
 *    MScalerOpen() 을 통해 MScaler instance를 생성하고 사용하면 된다.)
 *
 * 카메라에서의 동작 시퀀스: 동작중 Lock을 계속 유지
 *
 *   MScalerOpen()
 *   MScalerLock()
 *   MScalerSetImageFormat()
 *   for ( ; ; ) {
 *       MScalerSetImageData()
 *       MScalerRun()
 *       MScalerWaitDone()
 *   }
 *   MScalerUnlock()
 *   MScalerClose()
 *
 * 카메라 이외에서의 동작 시퀀스: 동작중 Lock을 획득/해제를 반복
 *
 *   MScalerOpen()
 *   for ( ; ; ) {
 *       if (MScalerTryLock()) {
 *           MScalerSetImageFormat()
 *           MScalerSetImageData()
 *           MScalerRunSync()
 *           MScalerUnlock()
 *       }
 *       else {
 *           // Another scaling method(GPU, S/W, ...)
 *       }
 *   }
 *   MScalerClose()
 */
/*****************************************************************************/

struct MScalerInfo {
    unsigned int width;
    unsigned int height;
    unsigned int pixelformat;
    enum v4l2_buf_type bufType;
    struct v4l2_format format;
    struct v4l2_buffer buffer;
    struct v4l2_plane planes[3];
    struct v4l2_crop crop;
};

struct MScaler {
    int fd;
    struct MScalerInfo src;
    struct MScalerInfo dst;
    int state;
    int flags;
};

/*****************************************************************************/

/*
 * State machine
 *
 * IDLE -> Lock() -> LOCKED -> Run() -> STARTED
 *   ^                |  ^                 |
 *   |                |  |                 |
 *   +--- Unlock() <--+  +--< waitDone() <-+
 */
enum MScalerState {
    MSCALER_STATE_IDLE,
    MSCALER_STATE_LOCKED,
    MSCALER_STATE_STARTED,
    MSCALER_STATE_COUNT
};

/*****************************************************************************/

static struct MScalerPixelFormat pixelFormats[] = {
    // 422_1P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_UYVY),
        .description = "YUV 4:2:2 packed, CbYCrY",
        .planes = 1,
        .components  = 1,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_1P,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_VYUY),
        .description = "YUV 4:2:2 packed, CrYCbY",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_1P,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YUYV),
        .description = "YUV 4:2:2 packed, YCbYCr",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_1P,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YVYU),
        .description = "YUV 4:2:2 packed, YCrYCb",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_1P,
    },
    // 422_2P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV16),
        .description = "YUV 4:2:2 planar, Y/CbCr",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_2P,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV61),
        .description = "YUV 4:2:2 planar, Y/CrCb",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_2P,
    },
    // 422_3P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YUV422P),
        .description = "YUV 4:2:2 3-planar, Y/Cb/Cr",
        .planes = 1,
        .components = 3,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_422_3P,
    },
    // 420_2P
    {
        // NV12의 경우 plane을 두개로 하지만 실제로는 하나로 할당 받도록 처리
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV12),
        .description = "YUV 4:2:0 contiguous 2-planar, Y/CbCr",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 12 },
        .hw_format = HW_FMT_420_2P,
    },
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV21),
        .description = "YUV 4:2:0 contiguous 2-planar, Y/CrCb",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 12 },
        .hw_format = HW_FMT_420_2P,
    },
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV12M),
        .description = "YUV 4:2:0 non-contiguous 2-planar, Y/CbCr",
        .planes = 2,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .hw_format = HW_FMT_420_2P,
    }, {
#ifndef V4L2_PIX_FMT_NV21M
#define V4L2_PIX_FMT_NV21M        v4l2_fourcc('N', 'M', '2', '1') /* 21  Y/CrCb 4:2:0  */
#endif  /*V4L2_PIX_FMT_NV21M*/
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV21M),
        .description = "YUV 4:2:0 non-contiguous 2-planar, Y/CrCb",
        .planes = 2,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .hw_format = HW_FMT_420_2P,
    },
    // 420_3P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YUV420M),
        .description = "YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr",
        .planes = 3,
        .components = 3,
        .bitperpixel = { 8, 2, 2 },
        .hw_format = HW_FMT_420_3P,
    },
    // ARGB
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_RGB32),
        .description = "XRGB-8888, 32 bpp",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 32 },
        .hw_format = HW_FMT_ARGB,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_BGR32),
        .description = "XBGR-8888, 32 bpp",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 32 },
        .hw_format = HW_FMT_ARGB,
    },
    // RGB565
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_RGB565),
        .description = "RGB565",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .hw_format = HW_FMT_RGB565,
    },
};

/*****************************************************************************/

static int scalerLock(struct MScaler *scaler, int timeout_ms)
{
    int ret;

    switch (scaler->state) {
        case MSCALER_STATE_IDLE:
            break;

        case MSCALER_STATE_LOCKED:
            DBG("Scaler is *already* locked.");
            // 에러로 처리한다. 중복 Lock을 허용하면 추후 문제 소지가 있다.
            return -EBUSY;

        case MSCALER_STATE_STARTED:
            DBG("Scaler is running.");
            return -EBUSY;

        default:
            ASSERT(! "Never reached !!!");
            break;
    }

    ret = ioctl(scaler->fd, VIDIOC_MSCALER_LOCK, timeout_ms);

    if (!ret)
        scaler->state = MSCALER_STATE_LOCKED;

    return 0;
}

static int scalerUnlock(struct MScaler *scaler)
{
    int ret;

    switch (scaler->state) {
        case MSCALER_STATE_IDLE:
            DBG("Scaler is not locked.");
            return -EINVAL;

        case MSCALER_STATE_LOCKED:
            break;

        case MSCALER_STATE_STARTED:
            DBG("Scaler is running.");
            return -EBUSY;

        default:
            ASSERT(! "Never reached !!!");
            break;
    }

    ret = ioctl(scaler->fd, VIDIOC_MSCALER_UNLOCK, 0);
    // must be ret == 0
    ASSERT(ret == 0);
    scaler->state = MSCALER_STATE_IDLE;

    return 0;
}

static inline struct MScalerPixelFormat *getPixelFormat(unsigned int pixelformat)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(pixelFormats); i++) {
        if (pixelFormats[i].pixelformat == pixelformat) {
            return &pixelFormats[i];
        }
    }

    ASSERT(! "Unknown Pixel Format.");
    return NULL;
}

// H/W 에 포맷을 적용한다.
static int scalerApplyFormat(struct MScaler *scaler, int isDst)
{
    int ret;
    struct MScalerInfo *info;
    struct v4l2_requestbuffers reqbuf;

    info = (isDst ? &scaler->dst : &scaler->src);

    ret = ioctl(scaler->fd, VIDIOC_S_FMT, &info->format);
    if (ret < 0) {
        ERR("VIDIOC_S_FMT failed.");
        goto done;
    }

    if (!isDst) {
        // src만 crop을 적용한다. dst는 지원하지 않는다.
        ret = ioctl(scaler->fd, VIDIOC_S_CROP, &info->crop);
        if (ret < 0) {
            ERR("VIDIOC_S_CROP failed.");
            goto done;
        }
    }

    reqbuf.count = 1;
    reqbuf.type = info->bufType;
    reqbuf.memory = V4L2_MEMORY_DMABUF;

    ret = ioctl(scaler->fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret < 0) {
        ERR("VIDIOC_REQBUFS failed.");
    }

done:
    return ret;
}

static int scalerSetFormat(
        struct MScaler *scaler, const struct MScalerImageFormat *fmt, int isDst)
{
    struct MScalerInfo *info;

    info = (isDst ? &scaler->dst : &scaler->src);
 
    info->width = fmt->width;
    info->height = fmt->height;
    info->pixelformat = fmt->pixelformat;
    info->bufType = (isDst ? 
            V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE :
            V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);

    info->format.type = info->bufType;
    info->format.fmt.pix_mp.width = fmt->width;
    info->format.fmt.pix_mp.height = fmt->height;
    info->format.fmt.pix_mp.pixelformat = fmt->pixelformat;
    info->format.fmt.pix_mp.field = V4L2_FIELD_ANY;
    info->format.fmt.pix_mp.num_planes =
        getPixelFormat(fmt->pixelformat)->planes;

    info->crop.type = info->bufType;
    info->crop.c.left = fmt->crop.left;
    info->crop.c.top = fmt->crop.top;
    info->crop.c.width = fmt->crop.width;
    info->crop.c.height = fmt->crop.height;

    return scalerApplyFormat(scaler, isDst);
}

static int scalerGetBufferInfo(struct MScaler *scaler,
        struct MScalerBufferInfo *bufInfo, int index, int isDst)
{
    int i;
    int ret;
    struct v4l2_buffer buf;
    struct MScalerInfo *info;
    struct v4l2_plane planes[MAX_PLANE_COUNT];

    memset(&buf, 0, sizeof(buf));
    memset(bufInfo, 0, sizeof(*bufInfo));

    info = (isDst ? &scaler->dst : &scaler->src);

    buf.index = index;
    buf.type = info->bufType;
    buf.memory = V4L2_MEMORY_DMABUF;
    buf.m.planes = planes;
    buf.length = info->format.fmt.pix_mp.num_planes;

    DBG_VAR("%d", buf.index);
    DBG_VAR("%d", buf.type);
    DBG_VAR("%d", buf.memory);
    DBG_VAR("%p", buf.m.planes);
    DBG_VAR("%d", buf.length);

    ret = ioctl(scaler->fd, VIDIOC_QUERYBUF, &buf);
    if (ret) {
        ERR("QUERYBUF failed.");
        return ret;
    }

    bufInfo->planes = buf.length;
    DBG_VAR("%d", bufInfo->planes);
    for (i = 0; i < bufInfo->planes; i++) {
        bufInfo->planeSizes[i] = planes[i].length;
        DBG("plane[%d].length = %u", i, planes[i].length);
        DBG("plane[%d].bytesused = %u", i, planes[i].bytesused);
    }

    return 0;
}

// H/W 에 데이터 버퍼를 적용한다.
static int scalerApplyData(struct MScaler *scaler, int isDst)
{
    int ret;
    struct MScalerInfo *info;

    info = (isDst ? &scaler->dst : &scaler->src);

    ret = ioctl(scaler->fd, VIDIOC_QBUF, &info->buffer);
    if (ret < 0) {
        ERR("QBUF failed.");
    }

    return ret;
}

// 내부 자료구조에 데이터 버퍼를 설정한다.
static int scalerSetData(
        struct MScaler *scaler, const struct MScalerImageData *data, int isDst)
{
    int i;
    struct MScalerInfo *info;
    struct MScalerPixelFormat *pf;

    info = (isDst ? &scaler->dst : &scaler->src);

    pf = getPixelFormat(info->pixelformat);
    ASSERT(pf);

    DBG("pf->planes = %d, data->planes = %d", pf->planes, data->planes);
    ASSERT(pf->planes == data->planes);

    info->buffer.index = 0;
    // TODO V4L2_BUF_FLAG_USE_SYNC가 정의되지 않음
    info->buffer.flags = 0x2000;//V4L2_BUF_FLAG_USE_SYNC;
    info->buffer.type = info->bufType;
    info->buffer.memory = V4L2_MEMORY_DMABUF;
    info->buffer.m.planes = info->planes;
    info->buffer.length = pf->planes;
//    info->buffer.reserved = data->acquireFenceFd;
    info->buffer.reserved = -1;

    for (i = 0; i < pf->planes; i++) {
        info->planes[i].m.fd = data->plane[i].fd;
        info->planes[i].length = data->plane[i].length;
        info->planes[i].bytesused = 0;
    }

    return scalerApplyData(scaler, isDst);
}

static int scalerStartStreaming(struct MScaler *scaler)
{
    enum v4l2_buf_type bufType;

    if (scaler->flags & SCALER_FLAG_START_STREAMING)
        return 0;

    // src stream on
    bufType = scaler->src.bufType;
    if (ioctl(scaler->fd, VIDIOC_STREAMON, &bufType) < 0) {
        ERR("STREAMON(src) failed.");
        return -1;
    }

    // dst stream on
    bufType = scaler->dst.bufType;
    if (ioctl(scaler->fd, VIDIOC_STREAMON, &bufType) < 0) {
        bufType = scaler->src.bufType;
        ioctl(scaler->fd, VIDIOC_STREAMOFF, &bufType);
        ERR("STREAMON(dst) failed.");
        return -1;
    }

    scaler->flags |= SCALER_FLAG_START_STREAMING;

    return 0;
}

static int scalerStopStreaming(struct MScaler *scaler)
{
    int ret;
    enum v4l2_buf_type bufType;
    struct v4l2_requestbuffers reqbuf;

    if (!(scaler->flags & SCALER_FLAG_START_STREAMING))
        return 0;

    switch (scaler->state) {
        case MSCALER_STATE_IDLE:
            DBG("Scaler is not started.");
            return 0;

        case MSCALER_STATE_LOCKED:
            break;

        case MSCALER_STATE_STARTED:
            DBG("Scaler is running.");
            return -EBUSY;

        default:
            ASSERT(! "Never reached !!!");
            break;
    }

    // src stream off
    bufType = scaler->src.bufType;
    if (ioctl(scaler->fd, VIDIOC_STREAMOFF, &bufType) < 0) {
        ERR("STREAMOFF(src) failed.");
        return -1;
    }

    reqbuf.count = 0;
    reqbuf.type = scaler->src.bufType;
    reqbuf.memory = V4L2_MEMORY_DMABUF;
    ret = ioctl(scaler->fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret < 0) {
        ERR("VIDIOC_REQBUFS failed.");
        return -1;
    }

    // dst stream off
    bufType = scaler->dst.bufType;
    if (ioctl(scaler->fd, VIDIOC_STREAMOFF, &bufType) < 0) {
        ERR("STREAMOFF(dst) failed.");
        return -1;
    }

    reqbuf.count = 0;
    reqbuf.type = scaler->dst.bufType;
    reqbuf.memory = V4L2_MEMORY_DMABUF;
    ret = ioctl(scaler->fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret < 0) {
        ERR("VIDIOC_REQBUFS failed.");
        return -1;
    }

    scaler->flags &= ~SCALER_FLAG_START_STREAMING;

    DBG("scalerStopStreaming done.");

    return 0;
}

static int scalerRun(struct MScaler *scaler)
{
    switch (scaler->state) {
        case MSCALER_STATE_IDLE:
            DBG("Scaler is not locked.");
            return -EINVAL;

        case MSCALER_STATE_LOCKED:
            break;

        case MSCALER_STATE_STARTED:
            DBG("Scaler is running.");
            return -EBUSY;

        default:
            ASSERT(! "Never reached !!!");
            return -EINVAL;
    }

    if (!(scaler->flags & SCALER_FLAG_SRC_FORMAT)) {
        ERR("SrcFormat is not specified.");
        return -EINVAL;
    }

    if (!(scaler->flags & SCALER_FLAG_DST_FORMAT)) {
        ERR("DstFormat is not specified.");
        return -EINVAL;
    }

    if (!(scaler->flags & SCALER_FLAG_SRC_DATA)) {
        ERR("SrcData is not specified.");
        return -EINVAL;
    }

    if (!(scaler->flags & SCALER_FLAG_DST_DATA)) {
        ERR("DstData is not specified.");
        return -EINVAL;
    }

    if (scalerStartStreaming(scaler) < 0) {
        ERR("scalerStartStreaming() failed.");
        return -EBUSY;
    }

    scaler->state = MSCALER_STATE_STARTED;

    return 0;
}

static int scalerWaitDone(struct MScaler *scaler, int timeout_ms)
{
    int ret;
    struct pollfd pollfd;

    switch (scaler->state) {
        case MSCALER_STATE_IDLE:
        case MSCALER_STATE_LOCKED:
            DBG("Scaler is not running.");
            return -EINVAL;

        case MSCALER_STATE_STARTED:
            break;

        default:
            ASSERT(! "Never reached !!!");
            break;
    }

    pollfd.fd = scaler->fd;
    pollfd.events = POLLIN;
    pollfd.revents = 0;

    ret = poll(&pollfd, 1, timeout_ms);
    DBG("poll ret = %d, revents = 0x%x\n", ret, pollfd.revents);
    if (ret > 0) {
        // dqbuf src & dst
        ret = ioctl(scaler->fd, VIDIOC_DQBUF, &scaler->dst.buffer);
        if (ret < 0) {
            ERR("DQBUF(dst) failed");
            return -1;
        }

        ret = ioctl(scaler->fd, VIDIOC_DQBUF, &scaler->src.buffer);
        if (ret < 0) {
            ERR("DQBUF(src) failed");
            return -1;
        }

#if 0
        ret = scalerStopStreaming(scaler);
        if (ret < 0) {
            ERR("stopStreaming failed");
            return -1;
        }
#endif  /*0*/
        scaler->state = MSCALER_STATE_LOCKED;
        ret = 0;
    }
    else if (ret == 0) {
        ret = -ETIMEDOUT;
    }

    return ret;
}

static int scalerCancel(struct MScaler *scaler)
{
    switch (scaler->state) {
        case MSCALER_STATE_IDLE:
        case MSCALER_STATE_LOCKED:
            DBG("Scaler is not running.");
            return -EINVAL;

        case MSCALER_STATE_STARTED:
            break;

        default:
            ASSERT(! "Never reached !!!");
            break;
    }

    /* TODO cancel scaling job */

    scaler->state = MSCALER_STATE_LOCKED;

    return 0;
}

#define IS_VALID_ALIGN(v, order)        (!((v) & ((1 << order) - 1)))

static int isValidImageFormat(struct MScaler *scaler,
        const struct MScalerImageFormat *format, int isDst)
{
    if (getPixelFormat(format->pixelformat) == NULL) {
        ERR("Unknown pixelformat(0x%x)", format->pixelformat);
        return 0;
    }

    if (format->width > MSCALER_MAX_WIDTH ||
            format->height > MSCALER_MAX_HEIGHT) {
        ERR("Size is over the limit.");
        return 0;
    }

    if (!IS_VALID_ALIGN(format->width, MSCALER_HALIGN_ORDER) || 
            !IS_VALID_ALIGN(format->height, MSCALER_VALIGN_ORDER)) {
        ERR("Size is not aligned.(%d x %d)", format->width, format->height);
        return 0;
    }

    if (!isDst) {
        // Only for source
        if (
                !IS_VALID_ALIGN(format->crop.left, MSCALER_HALIGN_ORDER) || 
                !IS_VALID_ALIGN(format->crop.top, MSCALER_VALIGN_ORDER) || 
                !IS_VALID_ALIGN(format->crop.width, MSCALER_HALIGN_ORDER) || 
                !IS_VALID_ALIGN(format->crop.height, MSCALER_VALIGN_ORDER)) {
            ERR("Crop is not aligned.(%d, %d, %d, %d)",
                    format->crop.left, format->crop.top,
                    format->crop.width, format->crop.height);
            return 0;
        }
    }

    return 1;
}

static int isValidImageData(
        const struct MScaler *scaler, const struct MScalerImageData *data)
{
    return 1;
}

/*****************************************************************************/

MScalerHandle *MScalerOpen(const char *node)
{
    struct MScaler *scaler;

    scaler = calloc(1, sizeof(*scaler));
    ASSERT(scaler);

    scaler->fd = open(node, O_RDWR);
    ASSERT(scaler->fd > 0);

    scaler->state = MSCALER_STATE_IDLE;

    return (MScalerHandle)scaler;
}

/*
 * src/dst 이미지의 포맷을 결정한다.
 * 상태에 무관하게 설정 가능하다.
 * 실제 H/W 적용은 run 직전에 수행된다.
 *
 * 초기나 포맷이 변경되는 시점에만 호출하는것이 효과적이다.
 * 이 함수가 호출되면 run 직전에 포맷을 H/W에 적용한다.
 */
int MScalerSetImageFormat(
        MScalerHandle handle,
        const struct MScalerImageFormat *srcFormat,
        const struct MScalerImageFormat *dstFormat)
{
    struct MScaler *scaler;

    ASSERT(handle);
    ASSERT(srcFormat || dstFormat);

    scaler = (struct MScaler *)handle;
    ASSERT(scaler->fd > 0);

    if (srcFormat) {
        if (!isValidImageFormat(scaler, srcFormat, 0)) {
            ERR("Invalid image format.");
            return -EINVAL;
        }

        if (scalerSetFormat(scaler, srcFormat, 0) == 0)
            scaler->flags |= SCALER_FLAG_SRC_FORMAT;
    }

    if (dstFormat) {
        if (!isValidImageFormat(scaler, dstFormat, 1)) {
            ERR("Invalid image format.");
            return -EINVAL;
        }
        
        if (scalerSetFormat(scaler, dstFormat, 1) == 0)
            scaler->flags |= SCALER_FLAG_DST_FORMAT;
    }

    return 0;
}

int MScalerGetBufferInfo(
        MScalerHandle handle, struct MScalerBufferInfo *buffer, int isDst)
{
    struct MScaler *scaler;

    ASSERT(handle);

    scaler = (struct MScaler *)handle;
    ASSERT(scaler->fd > 0);

    if (isDst) {
        if (!(scaler->flags & SCALER_FLAG_DST_FORMAT)) {
            ERR("DstFormat is not specified.");
            return -EINVAL;
        }
    }
    else {
        if (!(scaler->flags & SCALER_FLAG_SRC_FORMAT)) {
            ERR("SrcFormat is not specified.");
            return -EINVAL;
        }
    }

    return scalerGetBufferInfo(scaler, buffer, 0, isDst);
}

/*
 * src/dst 이미지의 data 버퍼를 설정한다.
 * 상태에 무관하게 설정 가능하다.
 * 실제 H/W 적용은 run 직전에 수행된다.
 */
int MScalerSetImageData(
        MScalerHandle handle,
        const struct MScalerImageData *srcData,
        const struct MScalerImageData *dstData)
{
    struct MScaler *scaler;

    ASSERT(handle);
    ASSERT(srcData || dstData);

    scaler = (struct MScaler *)handle;
    ASSERT(scaler->fd > 0);

    if (srcData) {
        if (!(scaler->flags & SCALER_FLAG_SRC_FORMAT)) {
            ERR("SrcFormat is not specified.");
            return -EINVAL;
        }

        if (!isValidImageData(scaler, srcData)) {
            ERR("Invalid image data.");
            return -EINVAL;
        }
        
        if (scalerSetData(scaler, srcData, 0) == 0)
            scaler->flags |= SCALER_FLAG_SRC_DATA;
    }

    if (dstData) {
        if (!(scaler->flags & SCALER_FLAG_DST_FORMAT)) {
            ERR("DstFormat is not specified.");
            return -EINVAL;
        }

        if (!isValidImageData(scaler, dstData)) {
            ERR("Invalid image data.");
            return -EINVAL;
        }

        if (scalerSetData(scaler, dstData, 1) == 0)
            scaler->flags |= SCALER_FLAG_DST_DATA;
    }

    return 0;
}

int MScalerRun(MScalerHandle handle)
{
    struct MScaler *scaler = (struct MScaler *)handle;
    return scalerRun(scaler);
}

int MScalerWaitDone(MScalerHandle handle, int timeout_ms)
{
    struct MScaler *scaler = (struct MScaler *)handle;
    return scalerWaitDone(scaler, timeout_ms);
}

int MScalerCancel(MScalerHandle handle)
{
    struct MScaler *scaler = (struct MScaler *)handle;
    return scalerCancel(scaler);
}

/*
 * 사용중일 경우 EBUSY를 리턴할 수 있어야 한다.
 */
int MScalerRunSync(MScalerHandle handle, int timeout_ms)
{
    int ret;
    struct MScaler *scaler = (struct MScaler *)handle;

    ret = scalerRun(scaler);
    if (ret < 0)
        return ret;

    return scalerWaitDone(scaler, timeout_ms);
}

int MScalerStop(MScalerHandle handle)
{
    struct MScaler *scaler = (struct MScaler *)handle;
    return scalerStopStreaming(scaler);
}

void MScalerClose(MScalerHandle handle)
{
    struct MScaler *scaler;

    ASSERT(handle);

    scaler = (struct MScaler *)handle;

    ASSERT(scaler->fd > 0);

    scalerCancel(scaler);

    close(scaler->fd);
    free(scaler);
}

/*
 * 사용전에 lock을 try해서 사용 가능 여부를 체크한다.
 * 성공시 lock이 걸린다.
 * 명시적으로 MScalerUnlock()을 호출하거나, fd가 close되면 unlock 된다.
 */
int MScalerTryLock(MScalerHandle handle)
{
    struct MScaler *scaler;
    ASSERT(handle);
    scaler = (struct MScaler *)handle;
    return scalerLock(scaler, 0);
}

/*
 * lock을 획득할때까지 대기한다.
 */
int MScalerLock(MScalerHandle handle, int timeout_ms)
{
    struct MScaler *scaler;
    ASSERT(handle);
    scaler = (struct MScaler *)handle;
    return scalerLock(scaler, timeout_ms);
}

/*
 * lock을 해제한다.
 */
int MScalerUnlock(MScalerHandle handle)
{
    struct MScaler *scaler;
    ASSERT(handle);
    scaler = (struct MScaler *)handle;
    return scalerUnlock(scaler);
}

int MScalerGetPixelFormatCount(void)
{
    return ARRAY_SIZE(pixelFormats);
}

struct MScalerPixelFormat *MScalerGetPixelFormatByIndex(int index)
{
    ASSERT(index >= 0);
    ASSERT(index < ARRAY_SIZE(pixelFormats));
    return &pixelFormats[index];
}

struct MScalerPixelFormat *MScalerGetPixelFormatByName(const char *name)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(pixelFormats); i++) {
        if (strcmp(pixelFormats[i].name, name) == 0) {
            return &pixelFormats[i];
        }
    }

    return (struct MScalerPixelFormat *)NULL;
}

struct MScalerPixelFormat *MScalerGetPixelFormat(unsigned int pixelformat)
{
    return getPixelFormat(pixelformat);
}

void MScalerGetPlaneInfo(const struct MScalerImageFormat *img,
        int plane, struct MScalerPlaneInfo *pi)
{
    struct MScalerPixelFormat *fmt;

    fmt = getPixelFormat(img->pixelformat);
    ASSERT(fmt);

    memset(pi, 0, sizeof(*pi));
    switch (fmt->hw_format) {
        case HW_FMT_422_1P:
            ASSERT(plane == 0);
            pi->comps = 1;
            pi->bpl[0] = (img->width << 1);
            pi->stride[0] = (((pi->bpl[0] + 3) >> 2) << 2);
            pi->height[0] = img->height;
            break;

        case HW_FMT_422_2P:
            ASSERT(plane == 0);

            pi->comps = 2;
            pi->bpl[0] = img->width;
            pi->stride[0] = (((pi->bpl[0] + 7) >> 3) << 3);
            pi->height[0] = img->height;

            pi->bpl[1] = img->width;
            pi->stride[1] = (((pi->bpl[1] + 7) >> 3) << 3);
            pi->height[1] = img->height;
            break;

        case HW_FMT_422_3P:
            ASSERT(plane == 0);

            pi->comps = 3;

            pi->bpl[0] = img->width;
            pi->stride[0] = (((pi->bpl[0] + 7) >> 3) << 3);
            pi->height[0] = img->height;

            pi->bpl[1] = (img->width >> 1);
            pi->stride[1] = (((pi->bpl[1] + 7) >> 3) << 3);
            pi->height[1] = img->height;

            pi->bpl[2] = (img->width >> 1);
            pi->stride[2] = (((pi->bpl[2] + 7) >> 3) << 3);
            pi->height[2] = img->height;
            break;

        case HW_FMT_420_2P:
            if (fmt->planes == 1) {
                // NV12, NV21
                pi->comps = 2;
                pi->bpl[0] = pi->bpl[1] = img->width;
                pi->stride[0] = pi->stride[1] = (((pi->bpl[0] + 7) >> 3) << 3);
                pi->height[0] = img->height;
                pi->height[1] = (img->height >> 1);
            }
            else {
                // NV12M, NV21M
                ASSERT(fmt->planes == 2);
                ASSERT(plane == 0 || plane == 1);
                pi->comps = 1;
                pi->bpl[0] = img->width;
                pi->stride[0] = (((pi->bpl[0] + 7) >> 3) << 3);
                if (plane == 0)
                    pi->height[0] = img->height;
                else
                    pi->height[0] = (img->height >> 1);
            }
            break;

        case HW_FMT_420_3P:
            pi->comps = 1;
            if (plane == 0) {
                pi->bpl[0] = img->width;
                pi->stride[0] = (((pi->bpl[0] + 7) >> 3) << 3);
                pi->height[0] = img->height;
            }
            else {
                ASSERT(plane == 1 || plane == 2);
                pi->bpl[0] = img->width >> 1;
                pi->stride[0] = (((pi->bpl[0] + 7) >> 3) << 3);
                pi->height[0] = img->height >> 1;
            }
            break;

        case HW_FMT_ARGB:
            pi->comps = 1;
            pi->bpl[0] = img->width << 2;
            pi->stride[0] = (((pi->bpl[0] + 3) >> 2) << 2);
            pi->height[0] = img->height;
            break;

        case HW_FMT_RGB565:
            pi->comps = 1;
            pi->bpl[0] = img->width << 1;
            pi->stride[0] = (((pi->bpl[0] + 3) >> 2) << 2);
            pi->height[0] = img->height;
            break;

        default:
            DIE("Not supported format for source image.");
            break;
    }
}
