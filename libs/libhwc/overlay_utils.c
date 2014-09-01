/*
 * Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <math.h>
#include <linux/gdm_fb.h>
//#include "gralloc_priv.h"
#include "overlay_utils.h"
#include "gdm_dss_wrapper.h"

//----------From class Res ------------------------------
const char* const Res::fbPath = "/dev/graphics/fb%u";
//--------------------------------------------------------

int get_odysseus_rotate(eTransform rotation)
{
	switch(rotation)
	{
		case OVERLAY_TRANSFORM_0 : return 0;
		case OVERLAY_TRANSFORM_FLIP_V:	return GDM_DSS_ROTATOR_VER_FLIP;
		case OVERLAY_TRANSFORM_FLIP_H:	return GDM_DSS_ROTATOR_HOR_FLIP;
		case OVERLAY_TRANSFORM_ROT_90:	return GDM_DSS_ROTATOR_90;
		//getMdpOrient will switch the flips if the source is 90 rotated.
		//Clients in Android dont factor in 90 rotation while deciding flip.
		case OVERLAY_TRANSFORM_ROT_90_FLIP_V:
			return GDM_DSS_ROTATOR_270_HOR_FLIP;
		case OVERLAY_TRANSFORM_ROT_90_FLIP_H:
			return GDM_DSS_ROTATOR_90_HOR_FLIP;
		case OVERLAY_TRANSFORM_ROT_180:
			return GDM_DSS_ROTATOR_180;
		case OVERLAY_TRANSFORM_ROT_270:
			return GDM_DSS_ROTATOR_270;
		default:
		    ALOGE("%s: invalid rotation value (value = 0x%x",
			    __FUNCTION__, rotation);
	}
}

int get_hal_rotate(int rotate)
{
	switch (rotate) {
		//From graphics.h
		case GDM_DSS_ROTATOR_HOR_FLIP :
			return HAL_TRANSFORM_FLIP_H;
		case GDM_DSS_ROTATOR_VER_FLIP:
			return HAL_TRANSFORM_FLIP_V;
		case GDM_DSS_ROTATOR_90:
			return HAL_TRANSFORM_ROT_90;
		case GDM_DSS_ROTATOR_180:
			return HAL_TRANSFORM_ROT_180;
		case GDM_DSS_ROTATOR_270:
			return HAL_TRANSFORM_ROT_270;
		default:
			ALOGE("%s: Unsupported HAL format = 0x%x", __func__, format);
			return GDM_DSS_ROTATOR_0;
	}
	// not reached

}

//--------------------------------------------------------
//Refer to graphics.h, gralloc_priv.h, msm_mdp.h
int get_odysseus_Format(int format, int *rgb_swap, int *endian)
{

	*rgb_swap = GDM_DSS_PF_ORDER_RGB;
	*endian = GDM_DSS_PF_ENDIAN_LITTLE;

	switch (format) {
		//From graphics.h
		case HAL_PIXEL_FORMAT_RGBA_8888 :
			*endian = GDM_DSS_PF_ENDIAN_BIG;
			*rgb_swap = GDM_DSS_PF_ORDER_BGR;
			return GDM_DSS_PF_ARGB8888;
		case HAL_PIXEL_FORMAT_RGBX_8888:
			*endian = GDM_DSS_PF_ENDIAN_BIG;
			*rgb_swap = GDM_DSS_PF_ORDER_BGR;
			return GDM_DSS_PF_ARGB8888;
		case HAL_PIXEL_FORMAT_RGB_888:
			return GDM_DSS_PF_RGB888;
		case HAL_PIXEL_FORMAT_RGB_565:
			return GDM_DSS_PF_RGB565;
		case HAL_PIXEL_FORMAT_RGBA_4444:
			*endian = GDM_DSS_PF_ENDIAN_BIG;
			*rgb_swap = GDM_DSS_PF_ORDER_BGR;
			return GDM_DSS_PF_ARGB4444;
		case HAL_PIXEL_FORMAT_BGRA_8888:
			//*rgb_swap = GDM_DSS_PF_ORDER_BGR;
			*endian = GDM_DSS_PF_ENDIAN_BIG;
			return GDM_DSS_PF_ARGB8888;
		case HAL_PIXEL_FORMAT_YV12:
			return GDM_DSS_PF_YUV420P3;	// YV12
		case HAL_PIXEL_FORMAT_YCbCr_422_SP:	// NV16
			return GDM_DSS_PF_YUV422P2;
		case HAL_PIXEL_FORMAT_YCrCb_420_SP:	// NV12
			return GDM_DSS_PF_YUV420P2;
		case HAL_PIXEL_FORMAT_YCbCr_422_I:
			return GDM_DSS_PF_YUV422I;
		case HAL_PIXEL_FORMAT_Y8:
			return GDM_DSS_PF_GRAY_X;
		default:
			ALOGE("%s: Unsupported HAL format = 0x%x", __func__, format);
			return -1;
	}
	// not reached
	return -1;
}

//Takes mdp format as input and translates to equivalent HAL format
//Refer to graphics.h, gralloc_priv.h, msm_mdp.h for formats.
int get_hal_format(int gdm_format, int rgb_swap, int endian) {
	switch (gdm_format) {
		//From graphics.h
		case GDM_DSS_PF_ARGB8888:
			if(endian) {
				if(rgb_swap == GDM_DSS_PF_ORDER_BGR)
					return HAL_PIXEL_FORMAT_RGBA_8888;
				else
					return HAL_PIXEL_FORMAT_BGRA_8888;
			}
		case HAL_PIXEL_FORMAT_RGB_888:
			return HAL_PIXEL_FORMAT_RGB_888;
		case GDM_DSS_PF_RGB565:
			return HAL_PIXEL_FORMAT_RGB_565;
		case GDM_DSS_PF_YUV420P3:
			return HAL_PIXEL_FORMAT_YV12;
		case GDM_DSS_PF_YUV422P2:
			return HAL_PIXEL_FORMAT_YCbCr_422_SP;
		case GDM_DSS_PF_YUV420P2:
			return HAL_PIXEL_FORMAT_YCrCb_420_SP;
		case GDM_DSS_PF_ARGB4444:
			return HAL_PIXEL_FORMAT_RGBA_4444;
		case GDM_DSS_PF_YUV422I:
			return HAL_PIXEL_FORMAT_YCbCr_422_I;
		case GDM_DSS_PF_GRAY_X:
			return HAL_PIXEL_FORMAT_Y8;
		default:
			ALOGE("%s: Unsupported GDM-DSS format = 0x%x", __func__, gdm_format);
			return -1;
	}
	// not reached
	return -1;
}

int get_down_scale_factor(const int& src_w, const int& src_h,
		const int& dst_w, const int& dst_h) {
	int dscale_factor = utils::ROT_DS_NONE;
	// The tolerance is an empirical grey area that needs to be adjusted
	// manually so that we always err on the side of caution
	float fDscaleTolerance = 0.05;
	// We need this check to engage the rotator whenever possible to assist MDP
	// in performing video downscale.
	// This saves bandwidth and avoids causing the driver to make too many panel
	// -mode switches between BLT (writeback) and non-BLT (Direct) modes.
	// Use-case: Video playback [with downscaling and rotation].

	if (dst_w && dst_h)
	{
		float fDscale =  sqrtf((float)(src_w * src_h) / (float)(dst_w * dst_h)) +
			fDscaleTolerance;

		// On our MTP 1080p playback case downscale after sqrt is coming to 1.87
		// we were rounding to 1. So entirely MDP has to do the downscaling.
		// BW requirement and clock requirement is high across MDP4 targets.
		// It is unable to downscale 1080p video to panel resolution on 8960.
		// round(x) will round it to nearest integer and avoids above issue.
		uint32_t dscale = round(fDscale);

		if(dscale < 2) {
			// Down-scale to > 50% of orig.
			dscale_factor = utils::ROT_DS_NONE;
		} else if(dscale < 4) {
			// Down-scale to between > 25% to <= 50% of orig.
			dscale_factor = utils::ROT_DS_HALF;
		} else if(dscale < 8) {
			// Down-scale to between > 12.5% to <= 25% of orig.
			dscale_factor = utils::ROT_DS_FOURTH;
		} else {
			// Down-scale to <= 12.5% of orig.
			dscale_factor = utils::ROT_DS_EIGHTH;
		}
	}
	return dscale_factor;
}

static inline int compute(const uint32_t x, const uint32_t y,
		const uint32_t z) {
	return x - ( y + z );
}

//Expects transform to be adjusted for clients of Android.
//i.e flips switched if 90 component present.
//See getMdpOrient()
void pre_rotate_source(const eTransform& tr, Whf& whf, Dim& srcCrop) {
	if(tr & OVERLAY_TRANSFORM_FLIP_H) {
		srcCrop.x = compute(whf.w, srcCrop.x, srcCrop.w);
	}
	if(tr & OVERLAY_TRANSFORM_FLIP_V) {
		srcCrop.y = compute(whf.h, srcCrop.y, srcCrop.h);
	}
	if(tr & OVERLAY_TRANSFORM_ROT_90) {
		int tmp = srcCrop.x;
		srcCrop.x = compute(whf.h,
				srcCrop.y,
				srcCrop.h);
		srcCrop.y = tmp;
		swap(whf.w, whf.h);
		swap(srcCrop.w, srcCrop.h);
	}
}


