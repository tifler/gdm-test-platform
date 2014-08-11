/*
* Copyright (c) 2013 The Linux Foundation. All rights reserved.
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
*    * Neither the name of The Linux Foundation. nor the names of its
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

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "hwcomposer.h"
#include "writeback.h"
#include "gdm_dss_wrapper.h"

static bool start_session(struct writeback_ctx_t *wb)
{
	if(wb && wb->fd != 0) {
		gdss_io_writeback_start(wb->fd);
		return true;
	}
	else
		return false;

}


static bool stop_session(struct writeback_ctx_t *wb)
{
	if(wb && wb->fd != NULL) {
		gdss_io_writeback_terminate(wb->fd);
		return true;
	}
	else
		return false;
}

static bool set_output_format(struct writeback_ctx_t *wb, int format)
{
	wb->format = format;
}

static bool queue_buffer(struct writeback_ctx_t *wb, int memory_id, int offset)
{
	struct gdmfb_data *fb_data = &wb->fb_data;

	fb_data->memory_id = memory_id;
	fb_data->offset = offset;
	fb_data->id = 0;
	fb_data->flags = 0;

	if(!gdss_io_writeback_queue_buffer(wb->fd, &fb_data))
		return true;
	else {
		ALOGE("writeback queue_buffer failed");
		return false;
	}
}

static bool dequeue_buffer(struct writeback_ctx_t *wb)
{
	if(!gdss_io_writeback_dequeue_bufer(wb->fd, &wb->out_data))
		return true;
	else {
		ALOGE("writeback dequeue_buffer failed");
		return false;
	}


}

static bool write_sync(struct writeback_ctx_t *wb, int memory_id, int offset)
{
	struct gdm_display_commit info;
	memset(&info, 0, sizeof(struct gdm_display_commit));

	if(queue_buffer(wb, memory_id, offset) < 0) {
		return false;
	}

	if(!gdss_io_display_commit(wb->fd, &info) < 0) {
		return false;
	}

	if(!dequeue_buffer(wb) < 0) {
		return false;
	}

	return true;
}

bool init_writeback_context(void *hwc_ptr)
{

	struct writeback_ctx_t *wb_ctx = NULL;
	struct hwc_context_t *hwc_ctx = (struct hwc_context_t *)hwc_ptr;

	wb_ctx = (struct writeback_ctx_t*)malloc(sizeof(struct writeback_ctx_t));
	memset(wb_ctx, 0x00, sizeof(struct writeback_ctx_t));

	// TODO: FIXME
	wb_ctx->fd = open("/dev/fb1", O_RDWR);
	if(wb_ctx->fd < 0)
		return false;

	wb_ctx->xres = hwc_ctx->dpyAttr[1].xres;
	wb_ctx->yres = hwc_ctx->dpyAttr[1].yres;

	wb_ctx->start = start_session;
	wb_ctx->stop = stop_session;
	wb_ctx->queue = queue_buffer;
	wb_ctx->dequeue = dequeue_buffer;
	wb_ctx->sync = write_sync;

	hwc_ctx->wb_ctx = wb_ctx;


	return true;
}

void exit_writeback_context(void *hwc_ptr)
{
	struct hwc_context_t *hwc_ctx = (struct hwc_context_t *)hwc_ptr;
	struct writeback_ctx_t *wb_ctx = hwc_ctx->wb_ctx;
	if(wb_ctx) {
		if(wb_ctx->fd >=0) {
			close(wb_ctx->fd);
		}
		free(wb_ctx);

		hwc_ctx->wb_ctx = NULL;
	}
}


