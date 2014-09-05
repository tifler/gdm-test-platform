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
#include <getopt.h>

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
	int interlace;

	int src_x, src_y;
	int src_w, src_h;
	int out_x, out_y;
	int out_w, out_h;

	int rotation;
};

struct ody_videoframe {
	int shared_fd[3];
	unsigned char *address[3];	/* virtual address */
	int size[3];
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


static int open_video(char *file_name,  struct ody_videofile *pvideo)
{
	int fd;

	if( 0 < (fd = open(file_name, O_RDONLY))) {
		pvideo->fd = fd;
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

cleanup:
	ion_close(fd);

	printf("alloc_video_memory: fd: %d, ret: %d\n", fd, ret);
	return ret;

}


static int dealloc_video_memory(struct ody_videoframe *pframe)
{
	int i = 0;

	for(i=0 ;i<3; i++) {
		if(pframe->shared_fd[i] > 0) {
			munmap(pframe->address[i], pframe->size[i]);
			close(pframe->shared_fd[i]);
		}
	}

	return 0;
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
	//req->src.endian = 0;
	//req->src.swap = 0;
	req->pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

	req->src_rect.x = req->src_rect.y = 0;
	req->src_rect.w = req->src.width;
	req->src_rect.h = req->src.height;


	//	req->dst_rect.w = gplayer->vi.xres;
	//	req->dst_rect.h = gplayer->vi.yres;

	if(gplayer->video_info.out_x != -1)
		req->dst_rect.x = gplayer->video_info.out_x;
	else
		req->dst_rect.x = 0;

	if(gplayer->video_info.out_y != -1)
		req->dst_rect.y = gplayer->video_info.out_y;
	else
		req->dst_rect.y = 0;

	if(gplayer->video_info.out_w != -1)
		req->dst_rect.w = gplayer->video_info.out_w;
	else
		req->dst_rect.w = gplayer->vi.xres;

	if(gplayer->video_info.out_h != -1)
		req->dst_rect.h = gplayer->video_info.out_h;
	else
		req->dst_rect.h = gplayer->vi.yres;


	req->transp_mask = 0;
	req->flags = GDM_DSS_FLAG_SCALING;

	if(gplayer->video_info.rotation) {
		switch(gplayer->video_info.rotation) {
		case GDM_DSS_ROTATOR_HOR_FLIP:
			req->flags |= GDM_DSS_FLAG_ROTATION_HFLIP;
			break;
		case GDM_DSS_ROTATOR_VER_FLIP:
			req->flags |= GDM_DSS_FLAG_ROTATION_VFLIP;
			break;
		case GDM_DSS_ROTATOR_180:
			req->flags |= GDM_DSS_FLAG_ROTATION_180;
			break;
		case GDM_DSS_ROTATOR_270_HOR_FLIP:
			req->flags |= (GDM_DSS_FLAG_ROTATION_HFLIP
				| GDM_DSS_FLAG_ROTATION_270);
			break;
		case GDM_DSS_ROTATOR_90:
			req->flags |= (GDM_DSS_FLAG_ROTATION_90);
			break;
		case GDM_DSS_ROTATOR_270:
			req->flags |= (GDM_DSS_FLAG_ROTATION_270);
			break;
		case GDM_DSS_ROTATOR_90_HOR_FLIP:
			req->flags |= (GDM_DSS_FLAG_ROTATION_90
				| GDM_DSS_FLAG_ROTATION_HFLIP);
		}

	}

	if(gplayer->video_info.interlace) {
		req->flags |= GDM_DSS_FLAG_IPC;
		//req->src.width = gplayer->video_info.width * 2;
		//req->src.height = gplayer->video_info.height / 2;
	}

	req->id |= GDMFB_NEW_REQUEST;

}

static void dss_get_fence_fd(int sockfd, int *release_fd, struct fb_var_screeninfo *vi)
{
	struct gdm_msghdr *msg = NULL;

//	printf("dss_get_fence_fd - start\n");
	msg = gdm_recvmsg(sockfd);

//	printf("received msg: %0x\n", (unsigned int)msg);
	if(msg != NULL){
		if(vi) {
			memcpy(vi, msg->buf, sizeof(struct fb_var_screeninfo));
		}
//		printf("msg->fds[0]: %d\n", msg->fds[0]);
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

	return 0;
}



static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;
	int i = 0;
	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), req_data->num_plane);

	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
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
	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
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
	int buf_ndx = 0;

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
	while(!gplayer->stop) {

		int i = 0;

		for(i = 0; i< 3; i++) {
			ret = get_video_frame(gplayer->video_info.fd,
				(unsigned char*)gplayer->frame[buf_ndx].address[i],
				gplayer->frame[buf_ndx].size[i]);
			if(ret == -1) {
				gplayer->stop = 1;
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

		dss_overlay_queue(sockfd, &req_data);
		dss_get_fence_fd(sockfd, &gplayer->release_fd, NULL);

		if(gplayer->release_fd != -1) {
		//	printf("sync_wait - in \n");
			ret = sync_wait(gplayer->release_fd, 1000);
			close(gplayer->release_fd);
		//	printf("sync_wait - out\n");
		}
		buf_ndx ^= 1;

//		usleep(50*1000);
	}

	// unset
	dss_overlay_unset(sockfd);

	if(gplayer->release_fd != -1)
		close(gplayer->release_fd);

	close(sockfd);

	return NULL;
}

static void help(char *progname)
{
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Usage: %s\n" \
            "  -iw width of contents [paramters]\n" \
            "  -ih height of contents [parameters]\n" \
            "  -if pixel format of contents [parameters]\n" \
            		"\t\t 8: GDM_DSS_PF_YUV422I \n" \
            		"\t\t 10: GDM_DSS_PF_YUV420P2 \n" \
			"\t\t 11: GDM_DSS_PF_YUV422P2 \n" \
			"\t\t 12: GDM_DSS_PF_YUV422P3 \n" \
			"\t\t 14: GDM_DSS_PF_YUV420P3 \n" \
            "  -ii interlace \n" \
            "  -iname [filename] \n" \
            "  -ox x position of output \n" \
            "  -oy y position of output \n" \
            "  -ow width of output \n" \
            "  -oh height of output \n" \
            "  -ox x position of output \n" \
            "  -oy y position of output \n" \
            "  -ow width of output \n" \
            "  -oh height of output \n" \
            "  -or rotation \n", progname);

    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example:\n" \
            "  %s -w 1280 -h 720 -i test.yuv\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
	int i;

	//int i_w, i_h, i_f, i_i;
	//int o_x, o_y, o_w, o_h, o_r;
	struct ody_player gplayer;
	struct ody_videofile *pvideo;
	char *filename = NULL;
	memset(&gplayer, 0x00, sizeof(struct ody_player));
	//pfb = &gplayer.fb_info;
	pvideo = &gplayer.video_info;
	/* initialize */


	pvideo->width = VIDEO_WIDTH;
	pvideo->height = VIDEO_HEIGHT;
	pvideo->format = VIDEO_FORMAT;
	pvideo->interlace = 0;

	pvideo->src_x = pvideo->src_y = 0;
	pvideo->src_w = pvideo->width;
	pvideo->src_h = pvideo->height;
	pvideo->out_x = pvideo->out_y = 0;

	// if out_w or out_h is -1, this value is set LCD Width or Height.
	pvideo->out_w = pvideo->out_h = -1;
	pvideo->rotation = 0;

	/* parameter parsing */
	while(1) {
		int option_index = 0, c = 0;
		static struct option long_options[] = {
				{"h",	no_argument, 0, 0},		// help
				{"help", no_argument, 0, 0},		// help
				{"iw", required_argument, 0, 0},	// input image width
				{"ih", required_argument, 0, 0},	// input image height
				{"if", required_argument, 0, 0},	// input image format
				{"ii", required_argument, 0, 0},	// input image type - interlace
				{"iname", required_argument, 0, 0},	// input file name
				{"sx", required_argument, 0, 0},	// source x position
				{"sy", required_argument, 0, 0},	// source y position
				{"sw", required_argument, 0, 0},	// source width
				{"sh", required_argument, 0, 0},	// source height
				{"dx", required_argument, 0, 0},	// output x position
				{"dy", required_argument, 0, 0},	// output y position
				{"dw", required_argument, 0, 0},	// output width
				{"dh", required_argument, 0, 0},	// output height
				{"dr", required_argument, 0, 0},	// output rotation
				{0, 0, 0, 0}
			};

        	c = getopt_long_only(argc, argv, "", long_options, &option_index);

		if(c == -1) break;

		if(c == '?') {
			help(argv[0]);
			return;
		}
		switch(option_index) {
		/* h, help */
		case 0:
		case 1:
			help(argv[0]);
			return 0;
			break;

		/* width */
		case 2:
			pvideo->width = atoi(optarg);
			break;
		/* height */
		case 3:
			pvideo->height = atoi(optarg);
			break;
		/* format */
		case 4:
			pvideo->format = atoi(optarg);
			break;
		/* interlace */
		case 5:
			pvideo->interlace = atoi(optarg);
			break;
		/* input filename */
		case 6:
			filename = strdup(optarg);
		    	break;
		/* source x-pos */
		case 7:
			pvideo->src_x = atoi(optarg);
			break;
		/* source y-pos */
		case 8:
			pvideo->src_y = atoi(optarg);
			break;
		/* source width */
		case 9:
			pvideo->src_w = atoi(optarg);
			break;
		/* source height */
		case 10:
			pvideo->src_h = atoi(optarg);
			break;
		/* output x-pos */
		case 11:
			pvideo->out_x = atoi(optarg);
			break;
		/* output y-pos */
		case 12:
			pvideo->out_y = atoi(optarg);
			break;
		/* output width */
		case 13:
			pvideo->out_w = atoi(optarg);
			break;
		/* output height */
		case 14:
			pvideo->out_h = atoi(optarg);
			break;
		/* rotation */
		case 15:
			pvideo->rotation = atoi(optarg);
			break;
		default:
			help(argv[0]);
			return 0;
		}

	}

	if(filename == NULL)
		return -1;


	if(open_video(filename, pvideo) == -1) {
		close(pvideo->fd);
		return -1;
	}

	gplayer.frame[0].size[0] = pvideo->width * pvideo->height;
	gplayer.frame[0].size[1] = gplayer.frame[0].size[2] = pvideo->width * pvideo->height / 4;

	gplayer.frame[1].size[0] = gplayer.frame[0].size[0];
	gplayer.frame[1].size[1] = gplayer.frame[0].size[1];
	gplayer.frame[1].size[2] = gplayer.frame[0].size[2];

	printf("frame size: %d %d\n", gplayer.frame[0].size[0], gplayer.frame[1].size[0]);

	for(i=0; i< 2; i++) {
		if(alloc_video_memory(&gplayer.frame[i]) != 0) {/* front buffer */
			goto exit;

		}
	}

	if(pthread_create(&decoding_worker, NULL, decoding_thread, &gplayer) != 0)
		goto exit;

	pthread_detach(decoding_worker);

	while(!gplayer.stop) {
		// 여기서 입력 받아서 설정을 변경?




		sleep(1);

	}

exit:
	for(i=0; i< 2; i++)
		dealloc_video_memory(&gplayer.frame[i]);

	pthread_cancel(decoding_worker);

	sleep(1);
	if(pvideo->fd)
		close(pvideo->fd);

	return 0;
}

