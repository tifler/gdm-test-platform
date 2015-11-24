#include <string.h>
#include <linux/videodev2.h>

#include "image-info.h"
#include "debug.h"

/*****************************************************************************/

#define ARRAY_SIZE(a)                   (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct GDMImageFormat {
    const char *name;
    unsigned int pixelformat;
    unsigned int planes;
    unsigned int components;
    unsigned int bitperpixel[3];
    unsigned int vdiv[3];
};

/*****************************************************************************/

static struct GDMImageFormat imageFormats[] = {
    // 422_1P
    {
        .name = "YUV 4:2:2 packed, CbYCrY",
        .pixelformat = V4L2_PIX_FMT_UYVY,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .vdiv = { 1 },
    }, {
        .name = "YUV 4:2:2 packed, CrYCbY",
        .pixelformat = V4L2_PIX_FMT_VYUY,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .vdiv = { 1 },
    }, {
        .name = "YUV 4:2:2 packed, YCbYCr",
        .pixelformat = V4L2_PIX_FMT_YUYV,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .vdiv = { 1 },
    }, {
        .name = "YUV 4:2:2 packed, YCrYCb",
        .pixelformat = V4L2_PIX_FMT_YVYU,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .vdiv = { 1 },
    },
    // 422_2P
    {
        .name = "YUV 4:2:2 planar, Y/CbCr",
        .pixelformat = V4L2_PIX_FMT_NV16,
        .planes = 1,
        .components = 2,
        .bitperpixel = { 8, 8 },
        .vdiv = { 1, 1 },
    }, {
        .name = "YUV 4:2:2 planar, Y/CrCb",
        .pixelformat = V4L2_PIX_FMT_NV61,
        .planes = 1,
        .components = 2,
        .bitperpixel = { 8, 8 },
        .vdiv = { 1, 1 },
    },
    // 422_3P
    {
        .name = "YUV 4:2:2 3-planar, Y/Cb/Cr",
        .pixelformat = V4L2_PIX_FMT_YUV422P,
        .planes = 1,
        .components = 3,
        .bitperpixel = { 8, 4, 4 },
        .vdiv = { 1, 1, 1 },
    },
    // 420_2P
    // 주의: NV12와 NV12M의 차이는 2Plane이 연속이냐 아니냐의 차이이다.
    {
        // NV12의 경우 plane을 두개로 하지만 실제로는 하나로 할당 받도록 처리
        .name = "YUV 4:2:0 contiguous 2-planar, Y/CbCr",
        .pixelformat = V4L2_PIX_FMT_NV12,
        .planes = 1,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .vdiv = { 1, 2 },
    },
    {
        .name = "YUV 4:2:0 non-contiguous 2-planar, Y/CbCr",
        .pixelformat = V4L2_PIX_FMT_NV12M,
        .planes = 2,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .vdiv = { 1, 2 },
    },
    {
        .name = "YUV 4:2:0 contiguous 2-planar, Y/CrCb",
        .pixelformat = V4L2_PIX_FMT_NV21,
        .planes = 1,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .vdiv = { 1, 2 },
    },
    {
        .name = "YUV 4:2:0 non-contiguous 2-planar, Y/CrCb",
        .pixelformat = V4L2_PIX_FMT_NV21M,
        .planes = 2,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .vdiv = { 1, 2 },
    },
    // 420_3P
    {
        .name = "YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr",
        .pixelformat = V4L2_PIX_FMT_YUV420M,
        .planes = 3,
        .components = 3,
        .bitperpixel = { 8, 2, 2 },
        .vdiv = { 1, 2, 2 },
    },
    // ARGB
    {
        .name = "XRGB-8888, 32 bpp",
        .pixelformat = V4L2_PIX_FMT_RGB32,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 32 },
        .vdiv = { 1 },
    }, {
        .name = "XBGR-8888, 32 bpp",
        .pixelformat = V4L2_PIX_FMT_BGR32,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 32 },
        .vdiv = { 1 },
    },
    // RGB565
    {
        .name = "RGB565",
        .pixelformat = V4L2_PIX_FMT_RGB565,
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .vdiv = { 1 },
    },
};

/*****************************************************************************/

static inline struct GDMImageFormat *lookupImageFormat(uint32_t pixelformat)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(imageFormats); i++) {
        if (pixelformat == imageFormats[i].pixelformat)
            return &imageFormats[i];
    }

    return (struct GDMImageFormat *)0;
}

/*****************************************************************************/

void GDMGetImageInfo(struct GDMImageInfo *info)
{
    unsigned int i;
    unsigned int offset;
    struct GDMImageFormat *fmt;

    ASSERT(info);
    ASSERT(info->width > 0);
    ASSERT(info->height > 0);
    ASSERT(info->pixelFormat);
    ASSERT(info->align > 0);

    fmt = lookupImageFormat(info->pixelFormat);
    ASSERT(fmt && "Unknown format");

    info->planeCountPhy = fmt->planes;
    info->planeCount = fmt->components;
    for (i = offset = 0; i < fmt->components; i++) {
        info->plane[i].offset = offset;
        // vdiv를 곱하는 이유는 420_2p, 420_3p를 위해서 이다.
        info->plane[i].bpl = 
            (((info->width * fmt->bitperpixel[i]) + 7) >> 3) * fmt->vdiv[i];
        info->plane[i].stride =
            (info->plane[i].bpl + info->align - 1) / info->align * info->align;
        ASSERT(fmt->vdiv[0] > 0);
        info->plane[i].lines = info->height / fmt->vdiv[i];
        if (fmt->components != fmt->planes)
            offset += info->plane[i].stride * info->plane[i].lines;
    }
}

