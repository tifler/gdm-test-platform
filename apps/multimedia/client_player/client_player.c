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
#include <limits.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <linux/kd.h>
#include <linux/types.h>
#include <linux/stat.h>

#include "sync.h"
#include <ion_api.h>
#include "gdm_fb.h"
#include "gdm-msgio.h"
#include "debug.h"
#include "hwcomposer.h"

/*****************************************************************************/

#define UNIX_SOCKET_PATH       "/tmp/sock_msgio"
#define BUFFER_SIZE            (4096)
#define FD_COUNT               (16)

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
	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data buffer[2];
	int buffer_index;
};


struct ody_player {
	struct ody_framebuffer fb_info;
	struct ody_videofile video_info;
	struct ody_videoframe frame[2];

	int release_fd;
	int buf_index;
	int stop;
};


static pthread_t decoding_worker;

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

static int get_video_frame(int fd, unsigned char *pdata, int size)
{
	int ret;
	int nread = 0;


	nread = read(fd, pdata, size);

	if(nread != size) {
		close(fd);
		return -1;
	}

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
	req->dst_rect.w = 800;
	req->dst_rect.h = 480;

	req->transp_mask = 0;
	req->flags = GDM_DSS_FLAG_SCALING;
	req->id = GDMFB_NEW_REQUEST;

}

static void dss_get_fence_fd(int sockfd, int *release_fd)
{
	struct gdm_msghdr *msg = NULL;

	printf("dss_get_fence_fd - start\n");
	msg = gdm_recvmsg(sockfd);

	printf("received msg: %0x\n", msg);
	if(msg != NULL){
		printf("msg->fds[0]: %d\n", msg->fds[0]);
		*release_fd = msg->fds[0];
		gdm_free_msghdr(msg);
	}

}


static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_SET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);

	memcpy(msg_data.data, req, sizeof(struct gdm_dss_overlay));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	gdm_sendmsg(sockfd, msg);
}



static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 1);

	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_PLAY;
	memcpy(msg_data.data, req_data, sizeof(struct gdm_dss_overlay_data));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
	msg->fds[0] = req_data->data.memory_id;
	gdm_sendmsg(sockfd, msg);
}

static int dss_overlay_unset(int sockfd)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));
	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_UNSET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
	gdm_sendmsg(sockfd, msg);

}


/* Decoding thread */
void *decoding_thread(void *arg)
{
	int ret = 0;
	int frame_num = 0;
	int buf_ndx = 0;

	int sockfd;
	struct sockaddr_un server_addr;

	struct gdm_msghdr *msg;
	struct ody_player *gplayer = (struct ody_player *)arg;
	struct gdm_dss_overlay req;
	struct gdm_dss_overlay_data req_data;


	pthread_detach(pthread_self());

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT(sockfd > 0);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, UNIX_SOCKET_PATH);

	// step-01: connect & get release_fence from display

	ret = connect(sockfd,
		(struct sockaddr *)&server_addr, sizeof(server_addr));
	ASSERT(ret == 0 && "connect() failed");

	dss_get_fence_fd(sockfd, &gplayer->release_fd);


	// step-02: request overlay to display
	dss_overlay_default_config(&req, gplayer);
	dss_overlay_set(sockfd, &req);

	// step-03: send framebuffer to display
	buf_ndx = 0;
	while(!gplayer->stop) {
		ret = get_video_frame(gplayer->video_info.fd,
			(unsigned char*)gplayer->frame[buf_ndx].address,
			gplayer->frame[buf_ndx].size);
		if(ret == -1) {
			gplayer->stop = 1;
			break;
		}

		frame_num ++;

		memset(&req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
		req_data.id = 0;
		req_data.data.memory_id = gplayer->frame[buf_ndx].shared_fd;
		req_data.data.offset = 0;
		dss_overlay_queue(sockfd, &req_data);

		if(gplayer->release_fd != -1) {
			//printf("wait frame done signal\n");
			ret = sync_wait(gplayer->release_fd, 10000);
		}
		buf_ndx ^= 1;
	}

	// unset
	dss_overlay_unset(sockfd);

	if(gplayer->release_fd != -1)
		close(gplayer->release_fd);

	close(sockfd);
}


int main(int argc, char **argv)
{
	int ret = 0;

	struct ody_player gplayer;
	struct ody_framebuffer *pfb;
	struct ody_videofile *pvideo;
	char str[256];
	unsigned val;

	memset(&gplayer, 0x00, sizeof(struct ody_player));
	pfb = &gplayer.fb_info;
	pvideo = &gplayer.video_info;
	/* initialize */

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

	if(pthread_create(&decoding_worker, NULL, decoding_thread, &gplayer) != 0)
		goto exit;

	pthread_detach(decoding_worker);

	while(!gplayer.stop) {

		sleep(1);

	}

exit:

	pthread_cancel(decoding_worker);

	sleep(1);
	if(pvideo->fd)
		close(pvideo->fd);

	return 0;
}

