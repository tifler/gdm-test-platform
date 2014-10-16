#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "v4l2.h"
#include "debug.h"
#include "gdm-buffer.h"

/*****************************************************************************/

#define GISP_IN_BUF_TYPE            V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
#define GISP_OUT_BUF_TYPE           V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
#define GISP_FIELD_TYPE             V4L2_FIELD_NONE
#define GISP_MEM_TYPE               V4L2_MEMORY_DMABUF

#define GISP_BUF_TYPE(out)          (out ? GISP_OUT_BUF_TYPE : GISP_IN_BUF_TYPE)

#define ALIGN(x, a)                 (((x) + (a) - 1) & ~((a) - 1))

#define ARRAY_SIZE(a)               (sizeof(a) / sizeof(a[0]))
#define MAKE_COLOR_FORMAT(fmt)      .name = #fmt, .pixelformat = fmt

/*****************************************************************************/

struct GISPPixelFormat {
    const char *name;
    const char *description;
    unsigned int pixelformat;
    unsigned int planes;
    unsigned int components;
    unsigned int bitperpixel[3];
    int hw_format;  /* HW_FMT_* */
};

enum {
    HW_FMT_422_1P,
    HW_FMT_422_2P,
    HW_FMT_422_3P,
    HW_FMT_420_2P,
    HW_FMT_420_3P,
    HW_FMT_ARGB,
    HW_FMT_RGB565,
    HW_FMT_COUNT
};

/*****************************************************************************/

static const char *fourcc2String(unsigned int fourcc)
{
    static char __fourccBuf[5];

    __fourccBuf[3] = (fourcc >> 24) & 0xff;
    __fourccBuf[2] = (fourcc >> 16) & 0xff;
    __fourccBuf[1] = (fourcc >> 8) & 0xff;
    __fourccBuf[0] = fourcc & 0xff;

    return __fourccBuf;
}

static struct GISPPixelFormat pixelFormats[] = {
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

static struct GISPPixelFormat *lookupFormat(unsigned int pixelformat)
{
    int i;
    struct GISPPixelFormat *fmt;

    for (i = 0; i < ARRAY_SIZE(pixelFormats); i++) {
        fmt = &pixelFormats[i];
        if (fmt->pixelformat == pixelformat)
            return fmt;
    }

    return (struct GISPPixelFormat *)0;
}

/*****************************************************************************/

int v4l2_enum_fmt(int fd, unsigned int pixelFormat, int isOutput)
{
    int found = 0;
    struct v4l2_fmtdesc fmtdesc;

    fmtdesc.type = GISP_BUF_TYPE(isOutput);
    fmtdesc.index = 0;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        INFO("Request PixelFormat = 0x%x(%s)",
                pixelFormat, fourcc2String(pixelFormat));
        INFO("Enum PixelFormat = 0x%x(%s)",
                fmtdesc.pixelformat, fourcc2String(fmtdesc.pixelformat));
        if (fmtdesc.pixelformat == pixelFormat) {
            INFO(">> Found PixelFormat = 0x%x(%s) <<",
                    pixelFormat, fourcc2String(pixelFormat));
            found = 1;
            break;
        }
        fmtdesc.index++;
    }

    if (!found) {
        ERR("0x%x: Unsupported pixel format.", pixelFormat);
        return -1;
    }

    return 0;
}

int v4l2_s_fmt(int fd, int width, int height,
        unsigned int pixelFormat, struct v4l2_format *fmtbuf, int isOutput)
{
    struct v4l2_format v4l2_fmt;
    struct GISPPixelFormat *gispFormat;

    gispFormat = lookupFormat(pixelFormat);
    ASSERT(gispFormat);

    memset(&v4l2_fmt, 0, sizeof(struct v4l2_format));

    v4l2_fmt.type = GISP_BUF_TYPE(isOutput);
    v4l2_fmt.fmt.pix_mp.width = width;
    v4l2_fmt.fmt.pix_mp.height = height;
    v4l2_fmt.fmt.pix_mp.pixelformat = pixelFormat;
    v4l2_fmt.fmt.pix_mp.field = GISP_FIELD_TYPE;
    v4l2_fmt.fmt.pix_mp.num_planes = gispFormat->planes;

    if (ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
        ERR("VIDIOC_S_FMT: Failed");
        return -1;
    }

    if (fmtbuf)
        memcpy(fmtbuf, &v4l2_fmt, sizeof(*fmtbuf));

    return 0;
}

int v4l2_querycap(int fd, int isOutput)
{
    struct v4l2_capability cap;

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        ERR("VIDIOC_QUERYCAP failed");
        return -1;
    }

    if (isOutput) {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE)) {
            ERR("Not a multi-planar capture device.");
            return -1;
        }
    }
    else {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
            ERR("Not a multi-planar capture device.");
            return -1;
        }
    }

    return 0;
}

int v4l2_enum_input(int fd, int index, char *buf, int buflen)
{
    struct v4l2_input input;

    input.index = index;

    if (ioctl(fd, VIDIOC_ENUMINPUT, &input) != 0) {
        ERR("VIDIOC_ENUMINPUT(%d) failed.", index);
        return -1;
    }

    strncpy(buf, (char *)input.name, buflen - 1);
    INFO("Input Channel[%d]: %s", input.index, input.name);

    return 0;
}

int v4l2_s_input(int fd, int index)
{
    struct v4l2_input input;

    input.index = index;

    if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0) {
        ERR("VIDIOC_S_INPUT(%d) failed", index);
        return -1;
    }

    return 0;
}

int v4l2_reqbufs(int fd, int bufferCount, int isOutput)
{
    struct v4l2_requestbuffers req;

    memset(&req, 0, sizeof(req));

    req.count = bufferCount;
    req.type = GISP_BUF_TYPE(isOutput);
    req.memory = GISP_MEM_TYPE;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        ERR("VIDIOC_REQBUFS(count=%d) failed.", bufferCount);
        return -1;
    }

    return req.count;
}

int v4l2_qbuf(int fd, int width, int height,
        const struct GDMBuffer *buffer, int index, int isOutput)
{
    int i;
    int ret;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    v4l2_buf.type = GISP_BUF_TYPE(isOutput);
    v4l2_buf.memory = GISP_MEM_TYPE;
    v4l2_buf.index = index;
    v4l2_buf.length = buffer->planeCount;
    v4l2_buf.m.planes = planes;

    for (i = 0; i < buffer->planeCount; i++) {
        planes[i].bytesused = 0;
        planes[i].m.fd = buffer->plane[i].fd;
        planes[i].length = buffer->plane[i].length;
        planes[i].data_offset = 0;
    }

    ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buf);
    if (ret < 0) {
        ERR("VIDIOC_QBUF(index=%d) failed.", index);
        return ret;
    }

    return 0;
}

int v4l2_dqbuf(int fd, int planeCount, int isOutput)
{
    int ret;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    v4l2_buf.m.planes = planes;
    v4l2_buf.length = planeCount;
    v4l2_buf.type = GISP_BUF_TYPE(isOutput);
    v4l2_buf.memory = GISP_MEM_TYPE;

    ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);
    if (ret < 0) {
        ERR("VIDIOC_DQBUF() failed.");
        return ret;
    }

    return v4l2_buf.index;
}

int v4l2_streamon(int fd, int isOutput)
{
    int ret;
    enum v4l2_buf_type bufferType = GISP_BUF_TYPE(isOutput);

    ret = ioctl(fd, VIDIOC_STREAMON, &bufferType);
    if (ret < 0) {
        ERR("VIDIOC_STREAMON(buf_type=%d) failed.(ret=%d)", bufferType, ret);
        return ret;
    }

    return ret;
}

int v4l2_streamoff(int fd, int isOutput)
{
    int ret;
    enum v4l2_buf_type bufferType = GISP_BUF_TYPE(isOutput);

    ret = ioctl(fd, VIDIOC_STREAMOFF, &bufferType);
    if (ret < 0) {
        ERR("VIDIOC_STREAMOFF(buf_type=%d) failed.", bufferType);
        return ret;
    }

    return ret;
}

int v4l2_g_ctrl(int fd, unsigned int id, int *value)
{
    struct v4l2_control ctrl;
    int ret;

    ctrl.id = id;

    ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
    if (ret < 0) {
        ERR("VIDIOC_G_CTRL(id=0x%x) failed.", id);
        return ret;
    }
    *value = ctrl.value;
    return 0;
}

int v4l2_s_ctrl(int fd, unsigned int id, int value)
{
    struct v4l2_control ctrl;
    int ret;

    ctrl.id = id;
    ctrl.value = value;

    ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);
    if (ret < 0) {
        ERR("VIDIOC_S_CTRL(id=0x%x, value=0x%x) failed. ret = %d, errno = %d", id, value, ret, errno);
        return ret;
    }

    return ctrl.value;
}


#if 0
static int fimc_v4l2_s_input(int fp, int index)
{
    struct v4l2_input input;

    input.index = index;

    if (ioctl(fp, VIDIOC_S_INPUT, &input) < 0) {
        ERR("ERR(%s):VIDIOC_S_INPUT failed", __func__);
        return -1;
    }

    return 0;
}

static int fimc_v4l2_s_fmt_cap(int fp, int width, int height, unsigned int fmt)
{
    struct v4l2_format v4l2_fmt;
    struct v4l2_pix_format pixfmt;

    memset(&pixfmt, 0, sizeof(pixfmt));

    v4l2_fmt.type = V4L2_BUF_TYPE;

    pixfmt.width = width;
    pixfmt.height = height;
    pixfmt.pixelformat = fmt;
    if (fmt == V4L2_PIX_FMT_JPEG)
        pixfmt.colorspace = V4L2_COLORSPACE_JPEG;

    v4l2_fmt.fmt.pix = pixfmt;
    DBG("fimc_v4l2_s_fmt_cap : width(%d) height(%d)", width, height);

    /* Set up for capture */
    if (ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
        ERR("ERR(%s):VIDIOC_S_FMT failed", __func__);
        return -1;
    }

    return 0;
}

int fimc_v4l2_s_fmt_is(int fp, int width, int height, unsigned int fmt, enum v4l2_field field)
{
    struct v4l2_format v4l2_fmt;
    struct v4l2_pix_format pixfmt;

    memset(&pixfmt, 0, sizeof(pixfmt));

    v4l2_fmt.type = V4L2_BUF_TYPE_PRIVATE;

    pixfmt.width = width;
    pixfmt.height = height;
    pixfmt.pixelformat = fmt;
    pixfmt.field = field;

    v4l2_fmt.fmt.pix = pixfmt;
    DBG("fimc_v4l2_s_fmt_is : width(%d) height(%d)", width, height);

    /* Set up for capture */
    if (ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
        ERR("ERR(%s):VIDIOC_S_FMT failed", __func__);
        return -1;
    }

    return 0;
}



static int fimc_v4l2_s_ext_ctrl(int fp, unsigned int id, void *value)
{
    struct v4l2_ext_controls ctrls;
    struct v4l2_ext_control ctrl;
    int ret;

    ctrl.id = id;

    ctrls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ctrls.count = 1;
    ctrls.controls = &ctrl;

    ret = ioctl(fp, VIDIOC_S_EXT_CTRLS, &ctrls);
    if (ret < 0)
        ERR("ERR(%s):VIDIOC_S_EXT_CTRLS failed", __func__);

    return ret;
}

static int fimc_v4l2_s_ext_ctrl_face_detection(int fp, unsigned int id, void *value)
{
    struct v4l2_ext_control ext_ctrl_fd[111];
    struct v4l2_ext_controls ext_ctrls_fd;
    struct v4l2_ext_controls *ctrls;
    camera_frame_metadata_t *facedata = (camera_frame_metadata_t *)value;
    int i, ret;

    ext_ctrl_fd[0].id = V4L2_CID_IS_FD_GET_FACE_COUNT;
    for (i = 0; i < 5; i++) {
        ext_ctrl_fd[22*i+1].id = V4L2_CID_IS_FD_GET_FACE_FRAME_NUMBER;
        ext_ctrl_fd[22*i+2].id = V4L2_CID_IS_FD_GET_FACE_CONFIDENCE;
        ext_ctrl_fd[22*i+3].id = V4L2_CID_IS_FD_GET_FACE_SMILE_LEVEL;
        ext_ctrl_fd[22*i+4].id = V4L2_CID_IS_FD_GET_FACE_BLINK_LEVEL;
        ext_ctrl_fd[22*i+5].id = V4L2_CID_IS_FD_GET_FACE_TOPLEFT_X;
        ext_ctrl_fd[22*i+6].id = V4L2_CID_IS_FD_GET_FACE_TOPLEFT_Y;
        ext_ctrl_fd[22*i+7].id = V4L2_CID_IS_FD_GET_FACE_BOTTOMRIGHT_X;
        ext_ctrl_fd[22*i+8].id = V4L2_CID_IS_FD_GET_FACE_BOTTOMRIGHT_Y;
        ext_ctrl_fd[22*i+9].id = V4L2_CID_IS_FD_GET_LEFT_EYE_TOPLEFT_X;
        ext_ctrl_fd[22*i+10].id = V4L2_CID_IS_FD_GET_LEFT_EYE_TOPLEFT_Y;
        ext_ctrl_fd[22*i+11].id = V4L2_CID_IS_FD_GET_LEFT_EYE_BOTTOMRIGHT_X;
        ext_ctrl_fd[22*i+12].id = V4L2_CID_IS_FD_GET_LEFT_EYE_BOTTOMRIGHT_Y;
        ext_ctrl_fd[22*i+13].id = V4L2_CID_IS_FD_GET_RIGHT_EYE_TOPLEFT_X;
        ext_ctrl_fd[22*i+14].id = V4L2_CID_IS_FD_GET_RIGHT_EYE_TOPLEFT_Y;
        ext_ctrl_fd[22*i+15].id = V4L2_CID_IS_FD_GET_RIGHT_EYE_BOTTOMRIGHT_X;
        ext_ctrl_fd[22*i+16].id = V4L2_CID_IS_FD_GET_RIGHT_EYE_BOTTOMRIGHT_Y;
        ext_ctrl_fd[22*i+17].id = V4L2_CID_IS_FD_GET_MOUTH_TOPLEFT_X;
        ext_ctrl_fd[22*i+18].id = V4L2_CID_IS_FD_GET_MOUTH_TOPLEFT_Y;
        ext_ctrl_fd[22*i+19].id = V4L2_CID_IS_FD_GET_MOUTH_BOTTOMRIGHT_X;
        ext_ctrl_fd[22*i+20].id = V4L2_CID_IS_FD_GET_MOUTH_BOTTOMRIGHT_Y;
        ext_ctrl_fd[22*i+21].id = V4L2_CID_IS_FD_GET_ANGLE;
        ext_ctrl_fd[22*i+22].id = V4L2_CID_IS_FD_GET_NEXT;
    }

    ext_ctrls_fd.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ext_ctrls_fd.count = 111;
    ext_ctrls_fd.controls = ext_ctrl_fd;
    ctrls = &ext_ctrls_fd;

    ret = ioctl(fp, VIDIOC_G_EXT_CTRLS, &ext_ctrls_fd);

    facedata->number_of_faces = ext_ctrls_fd.controls[0].value;

    for(i = 0; i < facedata->number_of_faces; i++) {
        facedata->faces[i].rect[0]      = ext_ctrl_fd[22*i+5].value;
        facedata->faces[i].rect[1]      = ext_ctrl_fd[22*i+6].value;
        facedata->faces[i].rect[2]      = ext_ctrl_fd[22*i+7].value;
        facedata->faces[i].rect[3]      = ext_ctrl_fd[22*i+8].value;
        facedata->faces[i].score        = ext_ctrl_fd[22*i+2].value;
/* TODO : id is unique value for each face. We need to suppot this. */
        facedata->faces[i].id           = 0;
        facedata->faces[i].left_eye[0]  = (ext_ctrl_fd[22*i+9].value + ext_ctrl_fd[22*i+11].value) / 2;
        facedata->faces[i].left_eye[1]  = (ext_ctrl_fd[22*i+10].value + ext_ctrl_fd[22*i+12].value) / 2;
        facedata->faces[i].right_eye[0] = (ext_ctrl_fd[22*i+13].value + ext_ctrl_fd[22*i+15].value) / 2;
        facedata->faces[i].right_eye[1] = (ext_ctrl_fd[22*i+14].value + ext_ctrl_fd[22*i+16].value) / 2;
        facedata->faces[i].mouth[0]     = (ext_ctrl_fd[22*i+17].value + ext_ctrl_fd[22*i+19].value) / 2;
        facedata->faces[i].mouth[1]     = (ext_ctrl_fd[22*i+18].value + ext_ctrl_fd[22*i+20].value) / 2;
    }

    return ret;
}

static int fimc_v4l2_g_parm(int fp, struct v4l2_streamparm *streamparm)
{
    int ret;

    streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(fp, VIDIOC_G_PARM, streamparm);
    if (ret < 0) {
        ERR("ERR(%s):VIDIOC_G_PARM failed", __func__);
        return -1;
    }

    DBG("%s : timeperframe: numerator %d, denominator %d", __func__,
            streamparm->parm.capture.timeperframe.numerator,
            streamparm->parm.capture.timeperframe.denominator);

    return 0;
}

static int fimc_v4l2_s_parm(int fp, struct v4l2_streamparm *streamparm)
{
    int ret;

    streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(fp, VIDIOC_S_PARM, streamparm);
    if (ret < 0) {
        ERR("ERR(%s):VIDIOC_S_PARM failed", __func__);
        return ret;
    }

    return 0;
}
#endif  /*0*/
