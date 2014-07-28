/*
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> // for POSIX threads
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>

#include <time.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <linux/stat.h>


#include <ion_api.h>
#include "gdm_fb.h"

#include <linux/fb.h>

#define FRAMEBUFFER_NAME	"/dev/fb0"
#define VIDEO_WIDTH		1280
#define VIDEO_HEIGHT		 720
#define VIDEO_FORMAT		GDM_DSS_PF_YUV420P3

struct ody_videofile {
	int fd;		/* file handler */
	int width;
	int height;
	int format;
};

struct ody_videoframe {
	int shared_fd;
	unsigned char *address;	/* virtual address */
	int size;
};


struct ody_framebuffer {
	int fd;
	struct fb_var_screeninfo vi;
	struct fb_fix_screeninfo fi;

	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data buffer[2];

	int buffer_index;
};


struct ody_player {
	struct ody_framebuffer fb_info;
	struct ody_videofile video_info;
	struct ody_videoframe frame[2];

	int buf_index;

	int renderer_stop;
	int stop;
};


static pthread_t decoding_worker, rendering_worker;
static pthread_cond_t video_renderer, video_decoder;
static pthread_mutex_t renderer_mutex, decoder_mutex;

static int open_framebuffer(void)
{
	int fd = 0;

	fd = open(FRAMEBUFFER_NAME, O_RDWR);
	if (fd < 0) {
		printf("cannot open /dev/fb0, retrying with /dev/fb0\n");
		return -1;
	}


	return fd;

}


static void dump_fb_info(struct fb_fix_screeninfo *fi, struct fb_var_screeninfo *vi)
{
	printf("dump_fb_info\n");
	fprintf(stderr,"vi.xres = %d\n", vi->xres);
	fprintf(stderr,"vi.yres = %d\n", vi->yres);
	fprintf(stderr,"vi.xresv = %d\n", vi->xres_virtual);
	fprintf(stderr,"vi.yresv = %d\n", vi->yres_virtual);
	fprintf(stderr,"vi.xoff = %d\n", vi->xoffset);
	fprintf(stderr,"vi.yoff = %d\n", vi->yoffset);
	fprintf(stderr, "vi.bits_per_pixel = %d\n", vi->bits_per_pixel);
	fprintf(stderr, "fi.line_length = %d\n", fi->line_length);
}


static int get_framebuffer(int fd,
		struct fb_var_screeninfo *vi,
		struct fb_fix_screeninfo *fi)
{
	int vsync_enable = 1;
	int ret = 0;
	if(ioctl(fd, FBIOGET_VSCREENINFO, vi) < 0) {
		printf("failed to get fb0 info");
		return -1;
	}

	if(ioctl(fd, FBIOGET_FSCREENINFO, fi) < 0) {
		perror("failed to get fb0 info");
		return -1;
	}

	dump_fb_info(fi, vi);

	ret = ioctl(fd, GDMFB_OVERLAY_VSYNC_CTRL, &vsync_enable);
	return ret;
}


static int open_video(char *file_name, struct ody_videofile *pvideo)
{
	int fd;

	if( 0 < (fd = open(file_name, O_RDONLY))) {
		pvideo->fd = fd;
		pvideo->width = VIDEO_WIDTH;
		pvideo->height = VIDEO_HEIGHT;
		pvideo->format = VIDEO_FORMAT;

		return fd;
	}

	return -1;

}

static int alloc_video_memory(struct ody_videoframe *pframe)
{
	int ret = 0;
	int fd;

	fd = ion_open();

	ret = ion_alloc_fd(fd, pframe->size, 0, ION_HEAP_CARVEOUT_MASK,
		0, &pframe->shared_fd);
	if (ret) {
		printf("share failed %s\n", strerror(errno));
		goto cleanup;
	}

	pframe->address = mmap(NULL, pframe->size, (PROT_READ | PROT_WRITE), MAP_SHARED, pframe->shared_fd, 0);
	if (pframe->address == MAP_FAILED) {
		goto cleanup;
	}

cleanup:
	ion_close(fd);

	printf("alloc_video_memory: fd: %d, ret: %d\n", fd, ret);
	return ret;

}

static int get_video_frame(int fd, unsigned char *pdata, int size, int *index)
{
	int ret;
	int nread = 0;


	nread = read(fd, pdata, size);

	if(nread != size) {
		close(fd);
		return -1;
	}

	*index ++;

	return 0;
}


static void dss_overlay_default_config(struct gdm_dss_overlay *req,
	struct ody_player *gplayer)
{

	memset(req, 0x00, sizeof(*req));

	req->alpha = 255;
	req->blend_op = GDM_FB_BLENDING_NONE;
	req->src.width = gplayer->video_info.width;
	req->src.height = gplayer->video_info.height;
	req->src.format = gplayer->video_info.format;
	req->src.endian = 0;
	req->src.swap = 0;
	req->pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

	req->src_rect.x = req->src_rect.y = 0;
	req->src_rect.w = req->src.width;
	req->src_rect.h = req->src.height;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = gplayer->fb_info.vi.xres;
	req->dst_rect.h = gplayer->fb_info.vi.yres;

	req->transp_mask = 0;
	req->flags = GDM_DSS_FLAG_SCALING;
	req->id = GDMFB_NEW_REQUEST;

}


static int dss_overlay_set(int fd, struct gdm_dss_overlay *req)
{
	return ioctl(fd, GDMFB_OVERLAY_SET, req);
}

static int dss_overlay_play(int fd, struct gdm_dss_overlay_data *req_data,
	struct ody_videoframe *pframe)
{
	req_data->data.memory_id = pframe->shared_fd;
	req_data->data.offset = 0;

	return ioctl(fd, GDMFB_OVERLAY_PLAY, req_data);
}

static int dss_overlay_commit(int fd)
{
	return ioctl(fd, GDMFB_OVERLAY_COMMIT, NULL);

}

static int dss_overlay_unset(int fd, int ov_id)
{
	return ioctl(fd, GDMFB_OVERLAY_UNSET, &ov_id);

}

void player_cleanup(void *arg)
{
	pthread_cond_destroy(&video_renderer);
	pthread_mutex_destroy(&renderer_mutex);

	pthread_cond_destroy(&video_decoder);
	pthread_mutex_destroy(&decoder_mutex);
}


#if 0
/* Decoding thread */
void *decoding_thread(void *arg)
{
	int frame_num = 0;
	struct ody_player *gplayer = (struct ody_player *)arg;
	int buf_ndx = gplayer->buf_index;

	pthread_cleanup_push(player_cleanup, NULL);

	sleep(1);	// waiting for rendering thread...

	get_video_frame(gplayer->video_info.fd,
		(unsigned char*)gplayer->frame[buf_ndx].address,
		gplayer->frame[buf_ndx].size,
		&frame_num);
	buf_ndx ^= 1;

	pthread_cond_signal(&video_renderer);


	while(!gplayer->stop || gplayer->renderer_stop == 0) {
		int ret = 0;
		ret = get_video_frame(gplayer->video_info.fd,
			(unsigned char*)gplayer->frame[buf_ndx].address,
			gplayer->frame[buf_ndx].size,
			&frame_num);

		if(ret == -1) {
			gplayer->stop = 1;
			pthread_cond_wait(&video_decoder, &decoder_mutex);
			break;
		}

		pthread_cond_wait(&video_decoder, &decoder_mutex);

		pthread_cond_signal(&video_renderer);

	}
	pthread_cleanup_pop(1);


}
#endif

#if 0
void *rendering_thread(void *arg)
{
	int ret = 0;
	struct ody_player *gplayer = (struct ody_player *)arg;
	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data req_data;
	int buf_ndx;

	dss_overlay_default_config(&request, gplayer);
	ret = dss_overlay_set(gplayer->fb_info.fd, &request);

	if(ret)
		return;
	else {
		req_data.id = request.id;
		printf("pipe id is %d\n", request.id);
	}
	while(!gplayer->stop)	{
		pthread_cond_wait(&video_renderer, &renderer_mutex);

		buf_ndx = (gplayer->buf_index ^ 1);
		dss_overlay_play(gplayer->fb_info.fd, &req_data,
			&gplayer->frame[buf_ndx]);

		dss_overlay_commit(gplayer->fb_info.fd);

		pthread_cond_signal(&video_decoder);
	}

	dss_overlay_unset(gplayer->fb_info.fd, request.id);

	gplayer->renderer_stop = 1;

}
#else

void *rendering_thread(void *arg)
{
	int ret = 0;
	struct ody_player *gplayer = (struct ody_player *)arg;
	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data req_data;
	int buf_ndx = 0;
	int frame_num = 0;

	dss_overlay_default_config(&request, gplayer);
	ret = dss_overlay_set(gplayer->fb_info.fd, &request);

	if(ret)
		return;
	else {
		req_data.id = request.id;
		printf("pipe id is %d\n", request.id);
	}

	pthread_cleanup_push(player_cleanup, NULL);

	while(!gplayer->stop)	{
		int ret = 0;
		ret = get_video_frame(gplayer->video_info.fd,
			gplayer->frame[buf_ndx].address,
			gplayer->frame[buf_ndx].size,
			&frame_num);

		//printf("frame_num[%04d]: 0x%08x\n", frame_num,
		//	gplayer->frame[buf_ndx].address);
		if(ret == -1) {
			gplayer->stop = 1;
			break;
		}

		dss_overlay_play(gplayer->fb_info.fd, &req_data,
			&gplayer->frame[buf_ndx]);

		dss_overlay_commit(gplayer->fb_info.fd);

		buf_ndx ^= 1;
	}

	dss_overlay_unset(gplayer->fb_info.fd, request.id);

	pthread_cleanup_pop(1);

	gplayer->renderer_stop = 1;

}

#endif

int main(int argc, char **argv)
{
	int ret = 0;

	struct ody_player gplayer;
	struct ody_framebuffer *pfb;
	struct ody_videofile *pvideo;

	//signal(SIGPIPE, SIG_IGN);

	memset(&gplayer, 0x00, sizeof(struct ody_player));
	pfb = &gplayer.fb_info;
	pvideo = &gplayer.video_info;
	/* initialize */

	pthread_mutex_init(&decoder_mutex, NULL);
	pthread_mutex_init(&renderer_mutex, NULL);

	if(pthread_cond_init(&video_renderer, NULL) != 0) {
		printf("could not initialize condition variable\n");
		goto exit;
	}
	if(pthread_cond_init(&video_decoder, NULL) != 0) {
		printf("could not initialize condition variable\n");
		goto exit;
	}


	pfb->fd = open_framebuffer();

	if(pfb->fd == -1)
		return -1;

	get_framebuffer(pfb->fd, &pfb->vi, &pfb->fi);

	if(open_video(argv[1], pvideo) == -1) {
		close(pvideo->fd);
		return -1;
	}

	gplayer.frame[0].size = pvideo->width * pvideo->height * 3 / 2;
	gplayer.frame[1].size = gplayer.frame[0].size;

	printf("frame size: %d %d\n", gplayer.frame[0].size, gplayer.frame[1].size);

	if(alloc_video_memory(&gplayer.frame[0]) != 0) {/* front buffer */
		goto exit;

	}
	if(alloc_video_memory(&gplayer.frame[1]) != 0) { /* back buffer */
		goto exit;

	}


	if(pthread_create(&rendering_worker, NULL, rendering_thread, &gplayer) != 0)
		goto exit;

	//if(pthread_create(&decoding_worker, NULL, decoding_thread, &gplayer) != 0)
	//	goto exit;

	pthread_detach(rendering_worker);
	//pthread_detach(decoding_worker);


	while(!gplayer.stop) {

		sleep(1);

	}

exit:

	pthread_cancel(rendering_worker);
	//pthread_cancel(decoding_worker);

	if(pfb->fd)
		close(pfb->fd);

	if(pvideo->fd)
		close(pvideo->fd);



	return 0;
}

