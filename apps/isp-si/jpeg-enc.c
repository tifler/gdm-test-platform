#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include "jpeg-enc.h"
#include "api/mme_c_api.h"
#include "gdm-buffer.h"
#include "v4l2_jpeg_api.h"
#include "debug.h"

/*****************************************************************************/

#define ARRAY_SIZE(a)               (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct GDMJPEGEncoder {
    int fd;
    struct GDMBuffer *frame;
    unsigned int count;
};

/*****************************************************************************/

#if 0
static int encodeJPEG(struct GDMJPEGEncoder *e,
        struct v4l2_ion_frame *src, struct v4l2_ion_buffer *dst, int quality)
{
    int ret;

    if (e->count == 0) {
        ret = v4l2_jpeg_enc_set_config(e->fd, src, quality, dst);
        ASSERT(ret == 0);

        ret = v4l2_jpeg_enc_reqbuf_src(e->fd, src);
        ASSERT(ret == 0);

        ret = v4l2_jpeg_enc_reqbuf_dst(e->fd, dst);
        ASSERT(ret == 0);
    }

    ret = v4l2_jpeg_enc_exe(e->fd, src, dst);
    ASSERT(ret == 0);

    ret = v4l2_jpeg_get_ctrl(e->fd, V4L2_JEPG_CID_ENC_GET_COMPRESSION_SIZE);
    ASSERT(ret > 0);

    e->count++;

    return ret;
}
#endif  /*0*/

/*****************************************************************************/

struct GDMJPEGEncoder *GJPEGEncOpen(void)
{
    struct GDMJPEGEncoder *e;

    e = calloc(1, sizeof(*e));
    ASSERT(e);
#if 0
    e->fd = v4l2_jpeg_enc_open();
    ASSERT(e->fd > 0);
#endif  /*0*/
    return e;
}

void GJPEGEncClose(struct GDMJPEGEncoder *e)
{
    ASSERT(e);
#if 0
    ASSERT(e->fd > 0);
    v4l2_jpeg_enc_close(e->fd);
#endif  /*0*/
    free(e);
}

void GJPEGEncEncodeFrame(struct GDMJPEGEncoder *e,
        const struct GDMBuffer *src, struct GDMBuffer *dst,
        struct GDMJPEGEncConfig *conf)
{
    int i;
    struct v4l2_ion_frame srcFrame;
    struct v4l2_ion_buffer dstBuffer;

    srcFrame.width = conf->image.width;
    srcFrame.height = conf->image.height;
    srcFrame.pix_fourcc = conf->image.pixelFormat;
    srcFrame.plane_count = conf->image.planeCount;
    for (i = 0; i < conf->image.planeCount; i++) {
        if (conf->image.planeCount > src->planeCount) {
            srcFrame.plane[i].shared_fd = src->plane[0].fd;
#if 0
            srcFrame.plane[i].buf_size =
                conf->image.plane[i].stride * conf->image.plane[i].lines;
#else
            srcFrame.plane[i].buf_size = src->plane[0].length;
#endif  /*0*/
        }
        else {
            srcFrame.plane[i].shared_fd = src->plane[i].fd;
            srcFrame.plane[i].buf_size = src->plane[i].length;
        }
        srcFrame.plane[i].vir_addr = 0;
        srcFrame.plane[i].mem_offset = conf->image.plane[i].offset;
        srcFrame.plane[i].stride = conf->image.plane[i].stride;

        DBG("PLANE-%d", i);
        DBG("shared_fd = %d", srcFrame.plane[i].shared_fd);
        DBG("buf_size = %d", srcFrame.plane[i].buf_size);
        DBG("stride = %d", srcFrame.plane[i].stride);
        DBG("offset = %d", srcFrame.plane[i].mem_offset);
    }

    dstBuffer.shared_fd = dst->plane[0].fd;
    dstBuffer.buf_size = dst->plane[0].length;
    dstBuffer.vir_addr = 0;
    dstBuffer.mem_offset = 0;
    dstBuffer.stride = 0;

#if 0
    dst->plane[0].used = encodeJPEG(e, &srcFrame, &dstBuffer, conf->quality);
#else
    i = v4l2_jpeg_encode_ion(
            &srcFrame, conf->quality, &dstBuffer, &dst->plane[0].used);
    ASSERT(i == 0);
#endif  /*0*/
}
