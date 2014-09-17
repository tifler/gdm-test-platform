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
#include <signal.h>
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
#define VIDEO_FORMAT		GDMFB_YUV420P3


struct ody_videofile {
	int fd;		/* file handler */
	int width;
	int height;
	int format;
};

struct ody_videoframe {
	int shared_fd[3];
	unsigned char *address[3];	/* virtual address */
	int size[3];

	int rot_shared_fd;
	unsigned char *rot_address;
};

struct ody_framebuffer {
	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data buffer[2];
	int buffer_index;
};


struct ody_player {
	struct fb_var_screeninfo vi;
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
	int i = 0;
	int ret = 0;
	int fd;

	fd = ion_open();


	for(i = 0; i< 3; i++) {

		ret = ion_alloc_fd(fd, pframe->size[i], 0, ION_HEAP_CARVEOUT_MASK,
			0, &pframe->shared_fd[i]);
		if (ret) {
			printf("share failed %s\n", strerror(errno));
			goto cleanup;
		}

		pframe->address[i] = mmap(NULL, pframe->size[i], (PROT_READ | PROT_WRITE), MAP_SHARED, pframe->shared_fd[i], 0);
		if (pframe->address == MAP_FAILED) {
			goto cleanup;
		}
	}

	// rotator buffer ...

	ret = ion_alloc_fd(fd, (pframe->size[0] * 4), 0, ION_HEAP_CARVEOUT_MASK,
			0, &pframe->rot_shared_fd);

	pframe->rot_address = mmap(NULL, pframe->size[0] * 4,
		(PROT_READ | PROT_WRITE), MAP_SHARED, pframe->rot_shared_fd, 0);
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
//	req->src.endian = 0;
//	req->src.swap = 0;
	req->pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

	req->src_rect.x = req->src_rect.y = 0;
	req->src_rect.w = req->src.width;
	req->src_rect.h = req->src.height;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = gplayer->vi.yres;
	req->dst_rect.h = 272;//(req->src.height * req->dst_rect.w) / req->src.width;

	req->transp_mask = 0;
	req->flags = (GDM_DSS_FLAG_SCALING | GDM_DSS_FLAG_ROTATION | GDM_DSS_FLAG_ROTATION_HFLIP);//GDM_DSS_FLAG_ROTATION_90);
	req->id = GDMFB_NEW_REQUEST;

}

static void dss_get_fence_fd(int sockfd, int *release_fd, struct fb_var_screeninfo *vi)
{
	struct gdm_msghdr *msg = NULL;

	//printf("dss_get_fence_fd - start\n");
	msg = gdm_recvmsg(sockfd);

	//printf("received msg: %0x\n", (unsigned int)msg);
	if(msg != NULL){

		if(vi)
			memcpy(vi, msg->buf, sizeof(struct fb_var_screeninfo));
		//printf("msg->fds[0]: %d\n", msg->fds[0]);
		*release_fd = msg->fds[0];
		gdm_free_msghdr(msg);
	}

}


static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_ROT_SAMPLE_PLAYER;
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
	int fd_cnt = 0;

	fd_cnt = req_data->num_plane;
	if(req_data->dst_data.memory_id)
		fd_cnt++;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), fd_cnt);

	msg_data.app_id = APP_ID_ROT_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_PLAY;

	if(req_data)
		memcpy(msg_data.data, req_data, sizeof(struct gdm_dss_overlay_data));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	for(i = 0 ; i < req_data->num_plane ; i++)
		msg->fds[i] = req_data->data[i].memory_id;

	if(req_data->dst_data.memory_id)
		msg->fds[req_data->num_plane] = req_data->dst_data.memory_id;
	gdm_sendmsg(sockfd, msg);

	return 0;
}


static int dss_overlay_unset(int sockfd)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));
	msg_data.app_id = APP_ID_ROT_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_UNSET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
	gdm_sendmsg(sockfd, msg);

	return 0;

}


/* Decoding thread */
void *decoding_thread(void *arg)
{
	int ret = 0;
	int frame_num = 0;
	int buf_ndx = 0, i;

	int sockfd;
	struct sockaddr_un server_addr;

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

	dss_get_fence_fd(sockfd, &gplayer->release_fd, &gplayer->vi);


	// step-02: request overlay to display
	dss_overlay_default_config(&req, gplayer);
	dss_overlay_set(sockfd, &req);

	// step-03: send framebuffer to display
	buf_ndx = 0;
	while(!gplayer->stop && frame_num < 20) {

		int i = 0;

		for(i = 0; i< 3; i++) {
			ret = get_video_frame(gplayer->video_info.fd,
				(unsigned char*)gplayer->frame[buf_ndx].address[i],
				gplayer->frame[buf_ndx].size[i]);
			if(ret == -1) {
				gplayer->stop = 1;
				printf("rotation player is stopped !!!!\n");
				break;
			}
		}
		frame_num ++;

		memset(&req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
		req_data.id = 0;
		req_data.num_plane = 3;
		req_data.data[0].memory_id = gplayer->frame[buf_ndx].shared_fd[0];
		req_data.data[0].offset = 0;

		req_data.data[1].memory_id = gplayer->frame[buf_ndx].shared_fd[1];
		req_data.data[1].offset = 0;

		req_data.data[2].memory_id = gplayer->frame[buf_ndx].shared_fd[2];
		req_data.data[2].offset = 0;

		// Rotator Output
		req_data.dst_data.memory_id = gplayer->frame[buf_ndx].rot_shared_fd;
		req_data.dst_data.offset = 0;

		dss_overlay_queue(sockfd, &req_data);
		dss_get_fence_fd(sockfd, &gplayer->release_fd, NULL);
		if(gplayer->release_fd != -1) {
			//printf("wait frame done signal\n");
			ret = sync_wait(gplayer->release_fd, 1000);
			close(gplayer->release_fd);
			gplayer->release_fd = -1;
		}
		buf_ndx ^= 1;

		//usleep(50*1000);
	}

	gplayer->stop = 1;
	// unset

	printf("overlay unset\n");
	dss_overlay_unset(sockfd);
	if(gplayer->release_fd != -1)
		close(gplayer->release_fd);

	sleep(1);

	for(buf_ndx = 0; buf_ndx< 2; buf_ndx++) {
		for(i=0;i<3;i++) {
			if(gplayer->frame[buf_ndx].shared_fd[i] > 0) {
				munmap(gplayer->frame[buf_ndx].address[i], gplayer->frame[buf_ndx].size[i]);
				close(gplayer->frame[buf_ndx].shared_fd[i]);
			}
		}
		if(gplayer->frame[buf_ndx].rot_shared_fd > 0) {
			munmap(gplayer->frame[buf_ndx].rot_address, gplayer->frame[buf_ndx].size[0] * 4);
			close(gplayer->frame[buf_ndx].rot_shared_fd);
		}
	}

	close(sockfd);

	return NULL;
}


void sigHandler(int signum, siginfo_t *info, void *ptr)
{
	struct ody_player *player = (struct ody_player *)ptr;
	printf("sigHandler: %08x\n", signum);

	printf("ody_player: %d %d\n", player->frame[0].size[0], player->stop);
	player->stop = 1;
}

int main(int argc, char **argv)
{
	struct ody_player gplayer;
	struct ody_videofile *pvideo;
	struct sigaction act;


	memset(&act, 0, sizeof(act));

	memset(&gplayer, 0x00, sizeof(struct ody_player));
	//pfb = &gplayer.fb_info;
	pvideo = &gplayer.video_info;
	/* initialize */

	//act.sa_sigaction = sigHandler;
	//act.sa_flags = SA_SIGINFO;

	//sigaction(SIGINT, &act, &gplayer);
	//signal(SIGINT, sigHandler);

	if(open_video(argv[1], pvideo) == -1) {
		close(pvideo->fd);
		return -1;
	}

	gplayer.frame[0].size[0] = pvideo->width * pvideo->height;
	gplayer.frame[0].size[1] = gplayer.frame[0].size[2] = pvideo->width * pvideo->height / 4;

	gplayer.frame[1].size[0] = gplayer.frame[0].size[0];
	gplayer.frame[1].size[1] = gplayer.frame[0].size[1];
	gplayer.frame[1].size[2] = gplayer.frame[0].size[2];

	printf("frame size: %d %d\n", gplayer.frame[0].size[0], gplayer.frame[1].size[0]);

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

