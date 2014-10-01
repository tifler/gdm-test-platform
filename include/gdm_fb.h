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

enum gdm_fb_format {
	GDMFB_RGB888 = 0,
	GDMFB_BGR888,
	GDMFB_RGB565,
	GDMFB_BGR565,
	GDMFB_ARGB1555,
	GDMFB_BGRA1555,
	GDMFB_RGBA1555,
	GDMFB_ABGR1555,
	GDMFB_ARGB8888,
	GDMFB_BGRA8888,
	GDMFB_RGBA8888,
	GDMFB_ABGR8888,
	GDMFB_YUV422I,
	GDMFB_YUV444I,
	GDMFB_YUV420P2,
	GDMFB_YUV422P2,
	GDMFB_YUV422P3,
	GDMFB_YUV422P3H_X,
	GDMFB_YUV420P3,
	GDMFB_MAX,
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

/* flag values */
#define GDM_DSS_ROT_NOP		0
#define GDM_DSS_FLIP_LR		0x1
#define GDM_DSS_FLIP_UD		0x2
#define GDM_DSS_ROT_90		0x4
#define GDM_DSS_ROT_180		(GDM_DSS_FLIP_UD|GDM_DSS_FLIP_LR)
#define GDM_DSS_ROT_270		(GDM_DSS_ROT_90|GDM_DSS_FLIP_UD|GDM_DSS_FLIP_LR)


#define GDM_TRANSP_NOP		0xffffffff
#define GDM_ALPHA_NOP		0xff

/* flag */
#define GDM_DSS_FLAG_SCALING		0x00000001
#define GDM_DSS_FLAG_ROTATION		0x00000002
#define GDM_DSS_FLAG_ROTATION_90	0x00000004
#define GDM_DSS_FLAG_ROTATION_180	0x00000008
#define GDM_DSS_FLAG_ROTATION_270	0x00000010
#define GDM_DSS_FLAG_ROTATION_HFLIP	0x00000020
#define GDM_DSS_FLAG_ROTATION_VFLIP	0x00000040
#define GDM_DSS_FLAG_IPC		0x00000080



#define GDM_OV_PLAY_NOWAIT		0x00200000
#define GDM_SOURCE_ROTATED_90		0x00100000
#define GDM_OVERLAY_PP_CFG_EN		0x00080000
#define GDM_BACKEND_COMPOSITION		0x00040000
#define GDM_BLEND_FG_PREMULT 		0x00020000
#define GDM_BORDERFILL_SUPPORTED	0x00010000
#define GDM_SECURE_OVERLAY_SESSION      0x00008000
#define GDM_MEMORY_ID_TYPE_FB		0x00001000

#define GDM_DSS_SECURE_DISPLAY_OVERLAY_SESSION	0x00008000
#define GDM_DSS_DISPLAY_OVERLAY_ID		0x10000000

enum gdm_fb_zorder {
	GDM_FB_ZORDER_0 = 0,
	GDM_FB_ZORDER_1,
	GDM_FB_ZORDER_2,
	GDM_FB_ZORDER_3,
	GDM_FB_ZORDER_4,
	GDM_FB_ZORDER_5,
};


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
	GDM_DSS_PIPE_TYPE_FB,
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
	//uint32_t swap;
	//uint32_t endian;
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
	uint32_t num_plane;
	struct gdmfb_data data[3];
	struct gdmfb_data dst_data;
};

struct gdm_display_commit {
	uint32_t flags;
	uint32_t wait_for_finish;
	struct fb_var_screeninfo var;
	struct gdm_dss_rect roi;
};



struct gdm_overlay_list {
	uint32_t num_overlays;
	struct gdm_dss_overlay **overlay_list;
	uint32_t flags;
	uint32_t processed_overlays;
};

union nr_peaking_control {
	struct {
		int nrp_on 		: 1;
		int nr_on		: 1;
		int nrp_lti_limit_on	: 1;
		int nrp_lti_on		: 1;
		int nrp_peaking_on	: 1;
		int nrp_lti_kernel_size	: 1;
		int nrp_pk_kernel_size	: 1;
		int resevered		: 25;
	} af;
	uint32_t a32;
};

union nr_peaking_config0 {
	struct {
		int nr_range_sel	: 4;
		int nr_domain		: 4;
		int nr_gain		: 8;
		int pk_config_pre	: 8;
		int reserved		: 8;
	} af;
	uint32_t a32;
};

union nr_peaking_config1 {
	struct {
		int pk_gain		: 8;
		int pk_limit		: 8;
		int pk_coring_post	: 8;
		int reserved		: 8;
	 } af;
	uint32_t a32;
};

union nr_peaking_config2 {
	struct {
		int pk_ctrl_y1		: 8;
		int pk_ctrl_tan		: 8;
		int pk_ctrl_x1		: 8;
		int reserved		: 8;
	 } af;
	uint32_t a32;
};

union nr_peaking_config3 {
	struct {
		int lti_ctrl_x1		: 8;
		int lti_gain		: 8;
		int pk_ctrl_y2		: 8;
		int reserved		: 8;
	 } af;
	uint32_t a32;
};

union nr_peaking_config4 {
	struct {
		int lti_ctrl_y2		: 8;
		int lti_ctrl_y1		: 8;
		int lti_ctrl_tan	: 8;
		int reserved		: 8;
	 } af;
	uint32_t a32;
};

struct gdm_dss_nr_peaking {
	union nr_peaking_control	ctrl;
	union nr_peaking_config0	config0;
	union nr_peaking_config1	config1;
	union nr_peaking_config2	config2;
	union nr_peaking_config3	config3;
	union nr_peaking_config4	config4;
};

struct gdmfb_nrp_data {
	uint32_t op_mode;
	struct gdm_dss_nr_peaking nrp_ctl;
};

enum gdm_dss_csc_type {
	GDM_DSS_CSC_JPEG = 0,
	GDM_DSS_CSC_SD_601,
	GDM_DSS_CSC_HD_709,
	GDM_DSS_CSC_MAX,
};

union pp1_control {
	struct {
		uint32_t reserved0	:	4;
		uint32_t r1_csc_on	:	1;
		uint32_t r1_range_mode :	1;
		uint32_t cont_on	:	1;
		uint32_t cont_bl_on	:	1;
		uint32_t cont_ana_sel 	:	1;
		uint32_t color_process_on : 	1;
		uint32_t reserved1	:	2;
		uint32_t con_bri_on	:	1;
		uint32_t round_on	:	1;
		uint32_t reserved3	:	7;
		uint32_t tone1_on	:	1;
		uint32_t tone2_on	:	1;
		uint32_t tone3_on	:	1;
		uint32_t rect_test	:	1;
	} af;
	uint32_t a32;
};


/* 0x0C24 */
union pp1_con_cfg0 {
	struct {
		uint32_t signal_range_low :	8;
		uint32_t reserved0	:	8;
		uint32_t signal_range_gain : 16;
	} af;
	uint32_t a32;
};

union pp1_con_cfg1 {
	struct {
		uint32_t sc_gain_limit_high : 8;
		uint32_t sc_gain_limit_low : 8;
		uint32_t sc_gain_high :	8;
		uint32_t sc_gain_low :	8;
	} af;
	uint32_t a32;
};

union pp1_con_cfg2 {
	struct {
		uint32_t mix_ratio :	8;
		uint32_t sc_limit_ratio :	8;
	} af;
	uint32_t a32;
};

union pp1_con_cfg3 {
	struct {
		uint32_t cgain_cth	:	8;
		uint32_t cgain_ctan :	8;
		uint32_t c2y_cgain	:	8;
		uint32_t c2y_cth	:	8;
	} af;
	uint32_t a32;
};

union pp1_con_cfg4 {
	struct {
		uint32_t m_bs_ref_level :	8;
		uint32_t m_bs_tanf	:	9;
		uint32_t reserved0	:	3;
		uint32_t m_center	:	10;
		uint32_t c2y_cth	:	8;
	} af;
	uint32_t a32;
};

union pp1_black0 {
	struct {
		uint32_t black_sref_th :	8;
		uint32_t black_sref_high :	8;
		uint32_t black_sref_low :	8;
		uint32_t black_sref_stan :	8;
	} af;
	uint32_t a32;
};

union pp1_black1 {
	struct {
		uint32_t black_level_apl_tan : 8;
		uint32_t black_level_apl_th	: 8;
		uint32_t black_sref_tan	: 8;
	} af;
	uint32_t a32;
};

union pp1_sc_apl {
	struct {
		uint32_t sc_apl_max :	8;
		uint32_t sc_apl_min	:	8;
		uint32_t sc_apl_offset :	8;
		uint32_t sc_apl_tan	:	8;
	} af;
	uint32_t a32;
};

union pp1_iir0 {
	struct {
		uint32_t iitan_high :	8;
		uint32_t iitan_low	:	8;
		uint32_t iidth_high :	8;
		uint32_t iidth_low	:	8;
	} af;
	uint32_t a32;
};

union pp1_iir1 {
	struct {
		uint32_t iigain_min_high :	8;
		uint32_t iigain_min_low :	8;
	} af;
	uint32_t a32;
};

union pp1_cos_sin_val {
	struct {
		uint32_t sin_val	:	14;
		uint32_t reserved0	:	2;
		uint32_t cos_val	:	14;
	} af;
	uint32_t a32;
};

union pp1_gain_cfg0 {
	struct {
		uint32_t color_gain	:	8;
	} af;
	uint32_t a32;
};

union pp1_pg_mg {
	struct {
		uint32_t cr_mg	:	8;
		uint32_t cr_pg	:	8;
		uint32_t cb_mg	:	8;
		uint32_t cb_pg	:	8;
	} af;
	uint32_t a32;
};

union pp1_rect_cfg0 {
	struct {
		uint32_t rect_bottom :	8;
		uint32_t rect_right	:	8;
		uint32_t rect_top	:	8;
		uint32_t rect_left	:	8;
	} af;
	uint32_t a32;
};

union pp1_rect_cfg1 {
	struct {
		uint32_t rect_tcr	:	8;
		uint32_t rect_tcb	:	8;
		uint32_t rect_scr	:	8;
		uint32_t rect_scb	:	8;
	} af;
	uint32_t a32;
};

union pp1_rect_cfg2 {
	struct {
		uint32_t rect_btan	:	8;
		uint32_t rect_ttan	:	8;
		uint32_t rect_rtan	:	8;
		uint32_t rect_ltan	:	8;
	} af;
	uint32_t a32;
};

struct pp1_rect_cfg {
	union pp1_rect_cfg0 pp1_rect_cfg0;
	union pp1_rect_cfg1 pp1_rect_cfg1;
	union pp1_rect_cfg2 pp1_rect_cfg2;
};

union pp1_cs1_offset {
	struct {
		uint32_t cs1_offset3 :	9;
		uint32_t cs1_offset2 :	9;
		uint32_t cs1_offset1 : 	9;
	} af;
	uint32_t a32;
};

union pp1_cs1_coe_1_2 {
	struct {
		uint32_t cs1_coe2	:	13;
		uint32_t cs1_coe1	:	13;
	} af;
	uint32_t a32;
};

union pp1_cs1_coe_3_4 {
	struct {
		uint32_t cs1_coe4	:	13;
		uint32_t cs1_coe3	:	13;
	} af;
	uint32_t a32;
};

union pp1_cs1_coe_5_6 {
	struct {
		uint32_t cs1_coe6	:	13;
		uint32_t cs1_coe5	:	13;
	} af;
	uint32_t a32;
};

union pp1_cs1_coe_7_8 {
	struct {
		uint32_t cs1_coe8	:	13;
		uint32_t cs1_coe7	:	13;
	} af;
	uint32_t a32;
};

union pp1_cs1_coe_9 {
	struct {
		uint32_t cs1_coe9	:	13;
	} af;
	uint32_t a32;
};


union pp1_con_bri_gain {
	struct {
		uint32_t con_bri_gain3 :	9;
		uint32_t con_bri_gain2 :	9;
		uint32_t con_bri_gain1 :	9;
	} af;
	uint32_t a32;
};

union pp1_con_bri_offset {
	struct {
		uint32_t con_bri_offset3 :	9;
		uint32_t con_bri_offset2 :	9;
		uint32_t con_bri_offset1 :	9;
	} af;
	uint32_t a32;
};

union pp1_cont_anal_apl {
	struct {
		uint32_t r_apcl 	:	8;
		uint32_t r_aplf	:	8;
		uint32_t r_aplfa	:	12;
	} af;
	uint32_t a32;
};

union pp1_cont_anal_center {
	struct {
		uint32_t r_bsreflevel :	8;
		uint32_t r_bstanf	:	9;
		uint32_t reserved0	:	3;
		uint32_t r_center	:	10;
	} af;
	uint32_t a32;
};


struct gdm_dss_pp1 {
	uint8_t	csc_type;
	union 	pp1_control 	pp1_control;
	union	pp1_con_cfg0	con_cfg0;
	union	pp1_con_cfg1	con_cfg1;
	union	pp1_con_cfg2	con_cfg2;
	union	pp1_con_cfg3	con_cfg3;
	union	pp1_con_cfg4	con_cfg4;
	union 	pp1_black0	black0;
	union 	pp1_black1	black1;
	union 	pp1_sc_apl	sc_apl;
	union 	pp1_iir0	iir0;
	union 	pp1_iir1	iir1;
	union 	pp1_cos_sin_val	cos_sin_val;
	union 	pp1_gain_cfg0	gain_cfg0;
	union 	pp1_pg_mg	pg_mg;
	struct 	pp1_rect_cfg 	rect_cfg[3];
	union 	pp1_cs1_offset	cs1_pre_offset;
	union 	pp1_cs1_offset	cs1_post_offset;
	union 	pp1_cs1_coe_1_2	cs1_coe_1_2;
	union 	pp1_cs1_coe_3_4	cs1_coe_3_4;
	union 	pp1_cs1_coe_5_6	cs1_coe_5_6;
	union 	pp1_cs1_coe_7_8	cs1_coe_7_8;
	union 	pp1_cs1_coe_9	cs1_coe_9;
	union 	pp1_con_bri_gain  con_bri_gain;
	union 	pp1_con_bri_offset	con_bri_offset;
	uint32_t			reserved1[15];
	union 	pp1_cont_anal_apl	cont_anal_apl;
	union 	pp1_cont_anal_center	cont_anal_center;
};

struct gdmfb_pp1_data {
	uint32_t op_mode;
	struct gdm_dss_pp1 pp1_ctl;
};

union pp2_control {
	struct {
		uint32_t reserved0	:	16;
		uint32_t gamma_lut_on :	1;
		uint32_t lut3d_on	:	1;
		uint32_t gamma_on	:	1;
		uint32_t degamma_on	:	1;
		uint32_t wb_conbri_on :	1;
		uint32_t dither_on	:	1;
		uint32_t pattern_on	:	1;
		uint32_t reserved1	:	4;
		uint32_t pattern_mode :	4;
	} af;
	volatile uint32_t a32;
};

union pp2_size {
	struct {
		uint32_t width	:	12;
		uint32_t reserved0	:	4;
		uint32_t height	:	12;
	} af;
	uint32_t a32;
};

union pp2_lut3d_color {
	struct {
		uint32_t seed_b	:	8;
		uint32_t seed_g 	:	8;
		uint32_t seed_r 	:	8;
	} af;
	uint32_t a32;
};

union pp2_wb_limit {
	struct {
		uint32_t wb_limit_hmax	:	2;
		uint32_t reserved0		:	2;
		uint32_t wb_limit_lmax	:	2;
		uint32_t reserved1		:	2;
		uint32_t wb_limit_lmin	:	2;
	} af;
	uint32_t a32;
};


union pp2_wb_gain {
	struct {
		uint32_t pp2_gain3	:	9;
		uint32_t pp2_gain2	:	9;
		uint32_t pp2_gain1	:	9;
	} af;
	uint32_t a32;
};

union pp2_wb_offset {
	struct {
		uint32_t wb_offset3	: 	9;
		uint32_t wb_offset2	: 	9;
		uint32_t wb_offset1	: 	9;
	} af;
	uint32_t a32;
};


struct dss_pp2_wb {
	union pp2_wb_limit	wb_limit;
	union pp2_wb_gain	wb_gain;
	union pp2_wb_offset	wb_offset;
};

union pp2_dither_bit {
	struct {
		uint32_t pp2_dith_bit3 :	2;
		uint32_t reserved0	:	2;
		uint32_t pp2_dith_bit2 :	2;
		uint32_t reserved1	:	2;
		uint32_t pp2_dith_bit1 : 	2;
	} af;
	uint32_t a32;
};

union pp2_gamma_lut_ce {
	struct {
		uint32_t pp2_dith_bit3 :	2;
		uint32_t reserved0	:	2;
		uint32_t pp2_dith_bit2 :	2;
		uint32_t reserved1	:	2;
		uint32_t pp2_dith_bit1 :	2;
	} af;
	uint32_t a32;
};

union pp2_gamma_lut_addr {
	struct {
		uint32_t gamma_lut_addr0 :	8;
		uint32_t gamma_lut_addr1 :	8;
	} af;
	uint32_t a32;
};

union pp2_gamma_wdata {
	struct {
		uint32_t gamma_lut_wdata0 :	16;
		uint32_t gamma_lut_wdata1 :	16;
	} af;
	uint32_t a32;
};

union pp2_gamma_rdata {
	struct {
		uint32_t gamma_lut_rdata0 :	16;
		uint32_t gamma_lut_rdata1 :	16;
	} af;
	uint32_t a32;
};



struct gdm_dss_pp2 {
	union 	pp2_control 	pp2_ctl;
	union	pp2_size  	pp2_size;
	uint32_t			reserved0[34];
	union 	pp2_lut3d_color	lut3d_black;
	union	pp2_lut3d_color	lut3d_blue;
	union	pp2_lut3d_color	lut3d_green;
	union	pp2_lut3d_color	lut3d_cyan;
	union	pp2_lut3d_color	lut3d_red;
	union	pp2_lut3d_color	lut3d_magenta;
	union	pp2_lut3d_color	lut3d_yellow;
	union	pp2_lut3d_color	lut3d_white;
	uint32_t			reserved1[2];
	union 	pp2_wb_limit	wb_limit;
	union 	pp2_wb_gain	wb_gain;
	union 	pp2_wb_offset	wb_offset;
	union	pp2_dither_bit	 dither_bit;
	union	pp2_gamma_lut_ce gamma_lut_ce;
};

struct gdmfb_pp2_data {
	uint32_t	op_mode;
	struct gdm_dss_pp2 pp2_ctl;
};

//
/* IOCTL commands */
#define GDMFB_IOCTL_MAGIC	'A'
#define GDMFB_SET_LUT			_IOW(GDMFB_IOCTL_MAGIC, 101, struct fb_cmap)
///#define GDMFB_NOTIFY_UPDATE		_IOW(GDMFB_IOCTL_MAGIC, 215, unsigned int)
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

#define GDMFB_WRITEBACK_INIT		_IOW(GDMFB_IOCTL_MAGIC, 300, unsigned int)
#define GDMFB_WRITEBACK_START		_IO(GDMFB_IOCTL_MAGIC, 301)
#define GDMFB_WRITEBACK_STOP		_IO(GDMFB_IOCTL_MAGIC, 302)
#define GDMFB_WRITEBACK_QUEUE_BUFFER	_IOWR(GDMFB_IOCTL_MAGIC, 303, \
						struct gdmfb_data)
#define GDMFB_WRITEBACK_DEQUEUE_BUFFER	_IOWR(GDMFB_IOCTL_MAGIC, 304, \
						struct gdmfb_data)
#define GDMFB_WRITEBACK_TERMINATE	_IO(GDMFB_IOCTL_MAGIC, 305)

#define GDMFB_NR_PEAKING		_IOWR(GDMFB_IOCTL_MAGIC, 400, struct gdmfb_nrp_data)
#define GDMFB_PP1			_IOWR(GDMFB_IOCTL_MAGIC, 401, struct gdmfb_pp1_data)
#define	GDMFB_PP2			_IOWR(GDMFB_IOCTL_MAGIC, 400, struct gdmfb_pp2_data)

#define GDMFB_MIXER_ROTATE		_IOWR(GDMFB_IOCTL_MAGIC, 500, struct gdm_dss_overlay)

#endif // __LINUX_GDM_FB_H__

