/*
* Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#include <string.h>
#include "gdm_dss_wrapper.h"


int gdss_io_get_fscreen_info(int fd, struct fb_fix_screeninfo *finfo)
{
	if(ioctl(fd, FBIOGET_FSCREENINFO, finfo) < 0) {
		ALOGE("Failed to call ioctl FBIOGET_FSCREENINFO err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}


int gdss_io_get_vscreen_info(int fd, struct fb_var_screeninfo *vinfo)
{
	if(ioctl(fd, FBIOGET_VSCREENINFO, vinfo) < 0) {
		ALOGE("Failed to call ioctl FBIOGET_VSCREENINFO err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_set_vscreen_info(int fd, struct fb_var_screeninfo *vinfo)
{
	if(ioctl(fd, FBIOPUT_VSCREENINFO, vinfo) < 0) {
		ALOGE("Failed to call ioctl FBIOPUT_VSCREENINFO err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_set_overlay(int fd, struct gdm_dss_overlay *ov)
{
	if(ioctl(fd, GDMFB_OVERLAY_SET, ov) < 0) {
		ALOGE("Failed to call ioctl GDM_FB_OVERLAY_SET err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_unset_overlay(int fd, int ov_id)
{
	if(ioctl(fd, GDMFB_OVERLAY_UNSET, &ov_id) < 0) {
		ALOGE("Failed to call ioctl GDMFB_OVERLAY_UNSET err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_get_overlay(int fd, struct gdm_dss_overlay *ov)
{
	if(ioctl(fd, GDMFB_OVERLAY_GET, ov) < 0) {
		ALOGE("Failed to call ioctl GDMFB_OVERLAY_GET err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_overlay_queue(int fd, struct gdm_dss_overlay_data *od)
{
	if(ioctl(fd, GDMFB_OVERLAY_PLAY, od) < 0) {
		ALOGE("Failed to call ioctl GDMFB_OVERLAY_PLAY err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_display_commit(int fd, struct gdm_display_commit *info)
{
	if(ioctl(fd, GDMFB_DISPLAY_COMMIT, info) < 0) {
		ALOGE("Failed to call ioctl GDMFB_DISPLAY_COMMIT err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_buffer_sync(int fd, struct gdm_dss_buf_sync *buf_sync)
{
	if(ioctl(fd, GDMFB_BUFFER_SYNC, buf_sync) < 0) {
		ALOGE("Failed to call ioctl GDMFB_DISPLAY_COMMIT err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_vsync_ctl(int fd, int value)
{
	if(ioctl(fd, GDMFB_VSYNC_CTRL, &value) < 0) {
		ALOGE("Failed to call ioctl GDMFB_DISPLAY_COMMIT err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}
int gdss_io_writeback_start(int fd)
{
	if(ioctl(fd, GDMFB_WRITEBACK_INIT, NULL) < 0) {
		ALOGE("Failed to call ioctl GDMFB_WRITEBACK_INIT err=%s\n",
			strerror(errno));
		return -1;
	}

	if(ioctl(fd, GDMFB_WRITEBACK_START, NULL) < 0) {
		ALOGE("Failed to call ioctl GDMFB_WRITEBACK_START err=%s\n",
			strerror(errno));
		return -1;
	}


	return 0;
}

int gdss_io_writeback_terminate(int fd)
{
	if(ioctl(fd, GDMFB_WRITEBACK_STOP, NULL) < 0) {
		ALOGE("Failed to call ioctl GDMFB_WRITEBACK_STOP err=%s\n",
			strerror(errno));
		return -1;
	}

	if(ioctl(fd, GDMFB_WRITEBACK_TERMINATE, NULL) < 0) {
		ALOGE("Failed to call ioctl MSMFB_WRITEBACK_TERMINATE err=%s",
			strerror(errno));
		return -1;
	}

	return 0;
}

int gdss_io_writeback_queue_buffer(int fd, struct gdmfb_data *fb_data)
{
	if(ioctl(fd, GDMFB_WRITEBACK_QUEUE_BUFFER, fb_data) < 0) {
		ALOGE("Failed to call ioctl FBIOPUT_VSCREENINFO err=%s\n",
			strerror(errno));
		return -1;
	}

	return 0;
}

int gdss_io_writeback_dequeue_bufer(int fd, struct gdmfb_data *fb_data)
{
	if(ioctl(fd, GDMFB_WRITEBACK_DEQUEUE_BUFFER, fb_data) < 0) {
		ALOGE("Failed to call ioctl GDMFB_WRITEBACK_DEQUEUE_BUFFER err=%s\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

int gdss_io_overlay_buf_sync(int fd, struct gdm_dss_buf_sync *buf_sync)
{
	if(ioctl(fd, GDMFB_BUFFER_SYNC, buf_sync) < 0) {
		ALOGE("Failed to call ioctl GDMFB_BUFFER_SYNC err=%s\n",
			strerror(errno));
		return -1;
	}

	return 0;
}

