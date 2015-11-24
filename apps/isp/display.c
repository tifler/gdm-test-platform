#include <poll.h>
#include <linux/videodev2.h>

#include "display.h"
#include "sync.h"
#include "ion_api.h"
#include "gdm_fb.h"
#include "gdm-msgio.h"
#include "gdm-buffer.h"
#include "hwcomposer.h"
#include "debug.h"

/*****************************************************************************/

#define ARRAY_SIZE(a)               (sizeof(a) / sizeof(a[0]))
#define MAKE_COLOR_FORMAT(fmt)      .name = #fmt, .pixelformat = fmt

/*****************************************************************************/

struct GDMDisplay {
    int sockfd;
    int releaseFd;
    struct fb_var_screeninfo screenInfo;
};

/*****************************************************************************/

struct GDispPixelFormat {
    const char *name;
    const char *description;
    unsigned int pixelformat;
    unsigned int planes;
    unsigned int components;
    unsigned int bitperpixel[3];
    int dss_format;  /* HW_FMT_* */
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

static struct GDispPixelFormat pixelFormats[] = {
    // 422_1P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_UYVY),
        .description = "YUV 4:2:2 packed, CbYCrY",
        .planes = 1,
        .components  = 1,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422I,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_VYUY),
        .description = "YUV 4:2:2 packed, CrYCbY",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422I,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YUYV),
        .description = "YUV 4:2:2 packed, YCbYCr",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422I,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YVYU),
        .description = "YUV 4:2:2 packed, YCrYCb",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422I,
    },
    // 422_2P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV16),
        .description = "YUV 4:2:2 planar, Y/CbCr",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422P2,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV61),
        .description = "YUV 4:2:2 planar, Y/CrCb",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422P2,
    },
    // 422_3P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YUV422P),
        .description = "YUV 4:2:2 3-planar, Y/Cb/Cr",
        .planes = 1,
        .components = 3,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_YUV422P3,
    },
    // 420_2P
    {
        // NV12의 경우 plane을 두개로 하지만 실제로는 하나로 할당 받도록 처리
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV12),
        .description = "YUV 4:2:0 contiguous 2-planar, Y/CbCr",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 12 },
        .dss_format = GDMFB_YUV420P2,
    },
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV21),
        .description = "YUV 4:2:0 contiguous 2-planar, Y/CrCb",
        .planes = 1,
        .components = 2,
        .bitperpixel = { 12 },
        .dss_format = GDMFB_YUV420P2,
    },
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV12M),
        .description = "YUV 4:2:0 non-contiguous 2-planar, Y/CbCr",
        .planes = 2,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .dss_format = GDMFB_YUV420P2,
    }, {

#ifndef V4L2_PIX_FMT_NV21M
#define V4L2_PIX_FMT_NV21M        v4l2_fourcc('N', 'M', '2', '1') /* 21  Y/CrCb 4:2:0  */
#endif  /*V4L2_PIX_FMT_NV21M*/

        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_NV21M),
        .description = "YUV 4:2:0 non-contiguous 2-planar, Y/CrCb",
        .planes = 2,
        .components = 2,
        .bitperpixel = { 8, 4 },
        .dss_format = GDMFB_YUV420P2,
    },
    // 420_3P
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_YUV420M),
        .description = "YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr",
        .planes = 3,
        .components = 3,
        .bitperpixel = { 8, 2, 2 },
        .dss_format = GDMFB_YUV420P3,
    },
    // ARGB
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_RGB32),
        .description = "XRGB-8888, 32 bpp",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 32 },
        .dss_format = GDMFB_ARGB8888,
    }, {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_BGR32),
        .description = "XBGR-8888, 32 bpp",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 32 },
        .dss_format = GDMFB_ABGR8888,
    },
    // RGB565
    {
        MAKE_COLOR_FORMAT(V4L2_PIX_FMT_RGB565),
        .description = "RGB565",
        .planes = 1,
        .components = 1,
        .bitperpixel = { 16 },
        .dss_format = GDMFB_RGB565,
    },
};

/*****************************************************************************/

static void dss_get_fence_fd(
        int sockfd, int *release_fd, struct fb_var_screeninfo *vi, int timeout)
{
    int ret;
    struct pollfd pollfd;
	struct gdm_msghdr *msg = NULL;

	//DBG("dss_get_fence_fd - start\n");

    // hwcomposer 에서 fenceFd를 보내지 못하는 경우가 있음.
    // 1초간 대기후 받지 못하면 즉시 리턴하도록 처리
    pollfd.fd = sockfd;
    pollfd.events = POLLIN | POLLERR | POLLHUP;
    pollfd.revents = 0;

    do {
        ret = poll(&pollfd, 1, timeout);
        if (ret < 0) {
            perror("poll");
        }
        else if (ret == 0) {
            DBG("Timeout.");
            *release_fd = -1;
            return;
        }

        if (pollfd.revents & (POLLERR | POLLHUP)) {
            ERR("Socket poll failed.(revents=0x%x)", pollfd.revents);
            exit(EXIT_FAILURE);
        }
    } while (ret <= 0);

    msg = gdm_recvmsg(sockfd);

	//DBG("received msg: %0x\n", (unsigned int)msg);
	if(msg != NULL){
        if (vi)
            memcpy(vi, msg->buf, sizeof(struct fb_var_screeninfo));
		//DBG("msg->fds[0]: %d\n", msg->fds[0]);
		*release_fd = msg->fds[0];
		gdm_free_msghdr(msg);
	}
}

static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_CAMERA_PREVIEW;
	msg_data.hwc_cmd = GDMFB_OVERLAY_SET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);

	memcpy(msg_data.data, req, sizeof(struct gdm_dss_overlay));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	gdm_sendmsg(sockfd, msg);

	return 0;
}

static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;
	int i = 0;
	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), req_data->num_plane);

	msg_data.app_id = APP_ID_CAMERA_PREVIEW;
	msg_data.hwc_cmd = GDMFB_OVERLAY_PLAY;
	memcpy(msg_data.data, req_data, sizeof(struct gdm_dss_overlay_data));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	for(i = 0 ; i < req_data->num_plane ; i++)
		msg->fds[i] = req_data->data[i].memory_id;
	gdm_sendmsg(sockfd, msg);

	return 0;
}

static int dss_overlay_unset(int sockfd)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));
	msg_data.app_id = APP_ID_CAMERA_PREVIEW;
	msg_data.hwc_cmd = GDMFB_OVERLAY_UNSET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
	gdm_sendmsg(sockfd, msg);

	return 0;

}

static unsigned int V4LFmt2DSSFmt(unsigned int pixelformat)
{
    int i;
    struct GDispPixelFormat *fmt;

    for (i = 0; i < ARRAY_SIZE(pixelFormats); i++) {
        fmt = &pixelFormats[i];
        if (fmt->pixelformat == pixelformat)
            return fmt->dss_format;
    }

    return 0;
}

/*****************************************************************************/

struct GDMDisplay *GDispOpen(const char *path, unsigned long flags)
{
    int ret;
    struct GDMDisplay *d;
    struct sockaddr_un server_addr;

    ASSERT(path);

    d = calloc(1, sizeof(*d));
    ASSERT(d);

    d->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT(d->sockfd > 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);

    ret = connect(d->sockfd,
            (struct sockaddr *)&server_addr, sizeof(server_addr));
    ASSERT(ret == 0 && "connect() failed");

    DBG("Client connected to server...");

    dss_get_fence_fd(d->sockfd, &d->releaseFd, &d->screenInfo, 1000);

    return d;
}

void GDispClose(struct GDMDisplay *d)
{
    ASSERT(d);
    dss_overlay_unset(d->sockfd);
    close(d->sockfd);
    free(d);
}

int GDispSetFormat(struct GDMDisplay *d, const struct GDMDispFormat *fmt)
{
    struct gdm_dss_overlay overlay;

    memset(&overlay, 0, sizeof(overlay));

    overlay.alpha = 255;
    overlay.blend_op = GDM_FB_BLENDING_NONE;
    overlay.src.width = fmt->width;
    overlay.src.height = fmt->height;
    overlay.src.format = V4LFmt2DSSFmt(fmt->pixelformat);
//    overlay.src.endian = 0;
//    overlay.src.swap = 0;
    overlay.pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

    overlay.src_rect.x = overlay.src_rect.y = 0;
    overlay.src_rect.w = overlay.src.width;
    overlay.src_rect.h = overlay.src.height;

    overlay.dst_rect.x = overlay.dst_rect.y = 0;

#if 0
    overlay.dst_rect.w = d->screenInfo.xres;
    overlay.dst_rect.h = d->screenInfo.yres;
#else
    overlay.dst_rect.w = overlay.src_rect.w;
    overlay.dst_rect.h = overlay.src_rect.h;
#endif
    overlay.transp_mask = 0;
    overlay.flags = 0;//GDM_DSS_FLAG_SCALING;
    overlay.id = GDMFB_NEW_REQUEST;

    return dss_overlay_set(d->sockfd, &overlay);
}

int GDispSetAttr(struct GDMDisplay *d, const struct GDMDispAttr *attr)
{
    // TODO
    return 0;
}

int GDispSendFrame(struct GDMDisplay *d, struct GDMBuffer *frame, int timeout)
{
    int i;
    int ret;
    int releaseFd;
    struct gdm_dss_overlay_data ov;

    memset(&ov, 0, sizeof(ov));

    ov.id = 0;
    ov.num_plane = frame->planeCount;
    for (i = 0; i < ov.num_plane; i++) {
        ov.data[i].memory_id = frame->plane[i].fd;
        ov.data[i].offset = 0;
    }

    dss_overlay_queue(d->sockfd, &ov);
    dss_get_fence_fd(d->sockfd, &releaseFd, NULL, timeout);

    if (releaseFd != -1) {
        ret = sync_wait(releaseFd, 1000);
        if (ret != 0) {
            DBG("=======> SYNC WAIT FAILED : ret = %d <=======", ret);
        }
        close(releaseFd);
    }

    return 0;
}
