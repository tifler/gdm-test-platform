/* include/linux/gdm-fb.h
 *
 * Copyright 2014 Anpass Inc.
 *
 * GDM7243V SoC Framebuffer driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software FoundatIon.
*/

#ifndef __LINUX_GDM_FB_H__
#define __LINUX_GDM_FB_H__

#include <linux/types.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <inttypes.h>

#define GDM_FB_MAX_OVERLAY	(6)	// except vid1
//#define GDM_FB_ROTATOR_LAYER	(5)	// GFX3


#define GDMFB_NEW_REQUEST 	(-1)
#define GDMFB_BASE_REQUEST 	(-2)

#ifndef __KERNEL__
enum gdm_dss_pixel_format
{
	GDM_DSS_PF_RGB888 = 0,
	GDM_DSS_PF_RGB565,
	GDM_DSS_PF_ARGB1555,
	GDM_DSS_PF_ARGB4444,
	GDM_DSS_PF_ARGB8888,
	GDM_DSS_PF_GRAY_X,
	GDM_DSS_PF_INDEX,
	GDM_DSS_PF_STREAM_X,
	GDM_DSS_PF_YUV422I,
	GDM_DSS_PF_YUV444I,
	GDM_DSS_PF_YUV420P2,
	GDM_DSS_PF_YUV422P2,
	GDM_DSS_PF_YUV422P3,
	GDM_DSS_PF_YUV422P3H_X,
	GDM_DSS_PF_YUV420P3,
	GDM_DSS_PF_YUV444_X,
	GDM_DSS_PF_MAX,
};

enum gdm_dss_pixel_rgb_order
{
	GDM_DSS_PF_ORDER_RGB = 0,
	GDM_DSS_PF_ORDER_RBG,
	GDM_DSS_PF_ORDER_GRB,
	GDM_DSS_PF_ORDER_GBR,
	GDM_DSS_PF_ORDER_BRG,
	GDM_DSS_PF_ORDER_BGR,
};

enum gdm_dss_pixel_endian
{
	GDM_DSS_PF_ENDIAN_LITTLE = 0,
	GDM_DSS_PF_ENDIAN_BIG,
};

enum gdm_dss_rotator_dir {
	GDM_DSS_ROTATOR_0 = 0,
	GDM_DSS_ROTATOR_HOR_FLIP,
	GDM_DSS_ROTATOR_VER_FLIP,
	GDM_DSS_ROTATOR_180,
	GDM_DSS_ROTATOR_270_HOR_FLIP,
	GDM_DSS_ROTATOR_90,
	GDM_DSS_ROTATOR_270,
	GDM_DSS_ROTATOR_90_HOR_FLIP,
	GDM_DSS_ROTATOR_DIR_MAX,
};
#endif


/* flag values */
#define GDM_DSS_ROT_NOP		0
#define GDM_DSS_FLIP_LR		0x1
#define GDM_DSS_FLIP_UD		0x2
#define GDM_DSS_ROT_90		0x4
#define GDM_DSS_ROT_180		(GDM_DSS_FLIP_UD|GDM_DSS_FLIP_LR)
#define GDM_DSS_ROT_270		(GDM_DSS_ROT_90|GDM_DSS_FLIP_UD|GDM_DSS_FLIP_LR)


#define GDM_TRANSP_NOP		0xffffffff
#define GDM_ALPHA_NOP		0xff

#define GDM_DSS_FLAG_SCALING		0x00000001
#define GDM_DSS_FLAG_ROTATION		0x00000002
#define GDM_DSS_FLAG_ROTATION_90	0x00000004
#define GDM_DSS_FLAG_ROTATION_180	0x00000008
#define GDM_DSS_FLAG_ROTATION_270	0x00000010
#define GDM_DSS_FLAG_ROTATION_HFLIP	0x00000020
#define GDM_DSS_FLAG_ROTATION_VFLIP	0x00000040



#define GDM_OV_PLAY_NOWAIT		0x00200000
#define GDM_SOURCE_ROTATED_90		0x00100000
#define GDM_OVERLAY_PP_CFG_EN		0x00080000
#define GDM_BACKEND_COMPOSITION		0x00040000
#define GDM_BLEND_FG_PREMULT 		0x00020000
#define GDM_BORDERFILL_SUPPORTED	0x00010000
#define GDM_SECURE_OVERLAY_SESSION      0x00008000
#define GDM_MEMORY_ID_TYPE_FB		0x00001000


enum gdm_fb_blending {
	GDM_FB_BLENDING_NONE = 0,
	GDM_FB_BLENDING_PREMULT = 1,
	GDM_FB_BLENDING_COVERAGE = 2,
	GDM_FB_BLENDING_MAX = 3,
};

enum gdm_fb_rotation_type {
	GDM_FB_ROT_TYPE_MIXER = 0,
	GDM_FB_ROT_TYPE_LAYER,
	GDM_FB_ROT_TYPE_MAX,
};

enum gdm_fb_rotation_value {
	GDM_FB_ROT_NONE = 0,
	GDM_FB_ROT_HOR_FLIP,
	GDM_FB_ROT_VER_FLIP,
	GDM_FB_ROT_180,
	GDM_FB_ROT_270_HOR_FLIP,
	GDM_FB_ROT_90,
	GDM_FB_ROT_270,
	GDM_FB_ROT_90_HOR_FLIP,
	GDM_FB_ROT_MAX,
};

enum gdm_fb_scaler_input_pf {
	GDM_FB_SCALER_PF_YUV422I = 8,
	GDM_FB_SCALER_PF_YUV444I = 9,
	GDM_FB_SCALER_PF_YUV420P2 = 10,
	GDM_FB_SCALER_PF_YUV422P2 = 11,
	GDM_FB_SCALER_PF_YUV422P3 = 12,
	GDM_FB_SCALER_PF_YUV420P3 = 14,
};

//enum gdm_fb_scaler_output_pf {
//
//};


enum gdm_fb_capture_target {
	GDM_FB_WDMA_SRC_SCALER = 0,
	GDM_FB_WDMA_SRC_MIXER,
	GDM_FB_WDMA_SRC_RGBIF,
	GDM_FB_WDMA_SRC_MAX,
};

/* scaler */
enum gdm_fb_scaler_output {
	GDM_FB_SCALER_MIXER = 0,
	GDM_FB_SCALER_WDMA0,
	GDM_FB_SCALER_WDMA1,
	GDM_FB_SCALER_ROT,
};

enum gdm_dss_pipe_type {
	GDM_DSS_PIPE_TYPE_UNUSED,
	GDM_DSS_PIPE_TYPE_VIDEO,
	GDM_DSS_PIPE_TYPE_GFX,
};


struct gdm_fb_user_plane_alpha {
	int		channel;
	enum gdm_fb_blending blending_type;
	unsigned char	plane_alpha;
};

struct gdm_fb_user_chroma {
	int		enabled;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};


struct gdm_fb_ion_client {
	int	fd;
	int	offset;
};

struct gdm_fb_user_ion_client {
	int	fd;
	int	offset;
};

struct disp_info_type_suspend {
	int op_enable;
	int panel_power_on;
};



#define GDM_DSS_MAX_FENCE_FD		10
#define GDM_DSS_BUF_SYNC_FLAG_WAIT	1
#define GDM_BUF_SYNC_FLAG_RETIRE_FENCE	0x10

struct gdm_dss_buf_sync {
	uint32_t flags;
	uint32_t acq_fen_fd_cnt;
	uint32_t session_id;
	int *acq_fen_fd;
	int *rel_fen_fd;
	int *retire_fen_fd;
};

struct gdm_dss_img {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t swap;
	uint32_t endian;
};

struct gdm_dss_rect {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};


struct gdm_dss_overlay {
	struct gdm_dss_img src;
	struct gdm_dss_rect src_rect; /* Overlay */
	struct gdm_dss_rect dst_rect; /* Mixer */
	uint32_t rot_source;	/* RDMA0, RDMA1, Mixer */
	uint32_t z_order;	/* stage number */
	uint32_t is_fg;		/* control alpha & transp */
	uint32_t alpha;
	uint32_t transp_mask;
	uint32_t blend_op;
	uint32_t flags; /* GDM_DSS_FLAG_SCALE */
	uint32_t pipe_type; /* VIDEO */
	uint32_t id; /* -1 */

	//struct mdp_scale_data scale;
	uint32_t user_data[8];
};


struct gdmfb_data {	/*msmfb_data */
	uint32_t offset;
	int memory_id;
	int id;
	uint32_t flags;
	int fence_fd;
	uint32_t priv;
};

struct gdm_dss_overlay_data {
	uint32_t id;
	struct gdmfb_data data;
	struct gdmfb_data dst_data;
};

struct gdm_display_commit {
	uint32_t flags;
	uint32_t wait_for_finish;
	struct fb_var_screeninfo var;
	struct gdm_dss_rect roi;
};



#define GDMFB_IOCTL_MAGIC	'A'
#define GDMFB_SET_LUT			_IOW(GDMFB_IOCTL_MAGIC, 101, struct fb_cmap)
#define GDMFB_OVERLAY_VSYNC_CTRL 	_IOW(GDMFB_IOCTL_MAGIC, 102, unsigned int)
#define GDMFB_VSYNC_CTRL  		_IOW(GDMFB_IOCTL_MAGIC, 103, unsigned int)
#define GDMFB_BUFFER_SYNC		_IOW(GDMFB_IOCTL_MAGIC, 104, \
						struct gdm_dss_buf_sync)
#define GDMFB_NOTIFY_UPDATE		_IOWR(GDMFB_IOCTL_MAGIC, 105, unsigned int)

#define GDMFB_OVERLAY_GET		_IOWR(GDMFB_IOCTL_MAGIC, 201, \
						struct gdm_dss_overlay)
#define GDMFB_OVERLAY_SET		_IOWR(GDMFB_IOCTL_MAGIC, 202, \
						struct gdm_dss_overlay)
#define GDMFB_OVERLAY_UNSET		_IOW(GDMFB_IOCTL_MAGIC, 203, __u32)
#define GDMFB_OVERLAY_COMMIT		_IO(GDMFB_IOCTL_MAGIC, 204)
#define GDMFB_OVERLAY_PLAY		_IOW(GDMFB_IOCTL_MAGIC, 205, \
						struct gdm_dss_overlay_data)	// queue
#define GDMFB_OVERLAY_PLAY_ENABLE 	_IOW(GDMFB_IOCTL_MAGIC, 206, __u32)
#define GDMFB_OVERLAY_PLAY_WAIT		_IOW(GDMFB_IOCTL_MAGIC, 207, \
						struct gdm_dss_overlay_data)
#define GDMFB_DISPLAY_COMMIT		_IOW(GDMFB_IOCTL_MAGIC, 208, \
						struct gdm_display_commit)
#define GDMFB_OVERLAY_PREPARE		_IOWR(GDMFB_IOCTL_MAGIC, 209, \
						struct gdm_overlay_list)

#define GDMFB_WRITEBACK_INIT		_IOW(GDMFB_IOCTL_MAGIC, 300, __u32)
#define GDMFB_WRITEBACK_START		_IO(GDMFB_IOCTL_MAGIC, 301)
#define GDMFB_WRITEBACK_STOP		_IO(GDMFB_IOCTL_MAGIC, 302)
#define GDMFB_WRITEBACK_QUEUE_BUFFER	_IOWR(GDMFB_IOCTL_MAGIC, 303, \
						struct gdmfb_data)
#define GDMFB_WRITEBACK_DEQUEUE_BUFFER	_IOWR(GDMFB_IOCTL_MAGIC, 304, \
						struct gdmfb_data)
#define GDMFB_WRITEBACK_TERMINATE	_IO(GDMFB_IOCTL_MAGIC, 305)

#endif // __LINUX_GDM_FB_H__

