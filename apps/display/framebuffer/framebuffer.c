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
#include <dirent.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
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

#include "gdm-msgio.h"
#include "debug.h"
#include "hwcomposer.h"
#include "sync.h"
#include "framebuffer.h"

#define DIGIT_NUM	6
#define LCD_WIDTH	800
#define LCD_HEIGHT	480

struct fb_context_t {
	pthread_t renderer_worker;

	int fb_fd;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;

	unsigned char *fb_ptr;
	int render_ndx;	// 0 or 1
	int bstop;
	int release_fd;
};

#if 0

static int readbmp(char *filename, unsigned char *buffer)
{
	int i,j;
	unsigned char r,g,b;//, a;
	unsigned int rgbvalue;
	HEADER header;
	INFOHEADER infoheader;
	FILE *fptr = NULL;
	int offset = 0;

	unsigned char *bmp_img;
	unsigned char *rgb_pos;

	int height_start, height_end;


	bmp_img = (unsigned char*)malloc(800*480*4);
	//printf("buffer address : %08x\n", buffer);

	/* Open file */
	if ((fptr = fopen(filename,"r")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\"\n",filename);
		exit(-1);
	}

	/* Read and check the header */
	fread(&header, 1, sizeof(HEADER), fptr);

	//printf("file size: %d\n", header.size);
	//printf("file offset: %d\n", header.offset);
	/* Seek to the start of the image data */

	/* Read and check the information header */
	if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
		fprintf(stderr,"Failed to read BMP info header\n");
		exit(-1);
	}

	fseek(fptr, 0x00, SEEK_SET);
	fread(bmp_img, 1, header.size, fptr);
	fclose(fptr);

	//printf("read success\n");
	rgb_pos = &bmp_img[header.offset];

	height_start = 0;
	height_end = (infoheader.height);

	/* Read the image */
	for (j=height_start;j<height_end;j++) {
		for (i=0;i<infoheader.width;i++) {

			switch (infoheader.bits) {
				case 1:
					break;
				case 4:
					break;
				case 8:
					break;
				case 32:
					b = *rgb_pos++;
					g = *rgb_pos++;
					r = *rgb_pos++;
					//a = *rgb_pos++;

					rgbvalue = ((127 << 24) | (r << 16) | (g << 8) | b);
					memcpy(buffer+offset, &rgbvalue, 4);
					offset += 4;

					//printf("rgbvalue: %08x, %d\n", rgbvalue, offset);
					break;
			}

		} /* i */
	} /* j */

	//printf("free image\n");
	free(bmp_img);

	return 0;
}

void load_image(struct fb_context_t *fb_ctx, int ndx)
{
	char filename[128] = {0};
	unsigned char *fb_ptr = NULL;
	sprintf(filename, "/mnt/images/bmp/image_%d.bmp", ndx);

	if(ndx == 0)
		fb_ptr = fb_ctx->fb_ptr;
	else
		fb_ptr = fb_ctx->fb_ptr + (fb_ctx->fi.line_length * fb_ctx->vi.yres);
	readbmp(filename, fb_ptr);
}
#else


static int readbmp(char *filename, unsigned char *buffer, int stride)
{
	int i,j;
	unsigned char r,g,b, a;
	unsigned int rgbvalue;
	HEADER header;
	INFOHEADER infoheader;
	FILE *fptr = NULL;
	int offset = 0;

	unsigned char *bmp_img;
	unsigned char *rgb_pos;
	unsigned char *save_img;

	int height_start, height_end;

	//bmp_img = (unsigned char*)malloc(800*480*4);
	//printf("buffer address : %08x\n", buffer);

	/* Open file */
	if ((fptr = fopen(filename,"r")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\"\n",filename);
		exit(-1);
	}

	printf("sizeof(HEADER): %d\n", sizeof(HEADER));
	/* Read and check the header */
	fread(&header, 1, sizeof(HEADER), fptr);


	/* Read and check the information header */
	if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
		fprintf(stderr,"Failed to read BMP info header\n");
		exit(-1);
	}

	bmp_img = malloc(header.size + 32*1024);

	fseek(fptr, 0x00, SEEK_SET);
	fread(bmp_img, 1, header.size, fptr);
	fclose(fptr);

	//printf("read success\n");
	rgb_pos = &bmp_img[header.offset];

	height_start = 0;
	height_end = (infoheader.height);

	/* Read the image */

	save_img = buffer + (stride * 4 * height_end);
	for (j=height_start;j<height_end;j++) {
		for (i=0;i<infoheader.width;i++) {

			switch (infoheader.bits) {
				case 1:
					break;
				case 4:
					break;
				case 8:
					break;
				case 32:
					b = *rgb_pos++;
					g = *rgb_pos++;
					r = *rgb_pos++;
					a = *rgb_pos++;

					rgbvalue = ((a << 24) | (r << 16) | (g << 8) | b);
					memcpy(save_img+offset, &rgbvalue, 4);
					offset += (infoheader.bits/8);

					//printf("rgbvalue: %08x, %d\n", rgbvalue, offset);
					break;
			}

		} /* i */
		offset = (stride * 4)*(height_end-j-1);
	} /* j */

	free(bmp_img);

	return 0;
}

static int my_power(int a, int b)
{
	int result = 1;
	int i;

	for(i=0; i<b; i++)
		result *= a;

	return result;
}

void load_image(struct fb_context_t *fb_ctx, int ndx, int frame_count)
{
	char filename[128] = {0};
	unsigned char *fb_ptr = NULL;
	int i = 0;

	fb_ptr = fb_ctx->fb_ptr + (fb_ctx->render_ndx * fb_ctx->vi.yres * fb_ctx->vi.xres * 4);

	for(i=DIGIT_NUM; i>= 0; i--) {
		int digit = frame_count / my_power(10,i);
		frame_count -= (digit * my_power(10, i));
		sprintf(filename, "/mnt/images/bmp/num_%d.bmp", digit);
		readbmp(filename, fb_ptr + (DIGIT_NUM-i)*100*4, fb_ctx->vi.xres);
	}
}


#endif

static inline int open_fb(int dpy) {
	int fd = -1;
#ifdef SUPPORT_ANDROID_SYSTEM
	const char *devtmpl = "/dev/graphics/fb%u";
#else
	const char *devtmpl = "/dev/fb%u";
#endif
	char name[64] = {0};
	snprintf(name, 64, devtmpl, dpy);
	fd = open(name, O_RDWR);
	return fd;
}



static int open_framebuffer_device(struct fb_context_t *fb_ctx)
{
	int fb_fd = 0;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;

	//float fps = 10.0f;	// TODO

	fb_fd = open_fb(HWC_DISPLAY_PRIMARY);
	if (fb_fd < 0) {
		printf("cannot open /dev/fb0, retrying with /dev/fb0\n");
		return -1;
	}

	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &vi) < 0) {
		printf("failed to get fb0 info");
		return -1;
	}

	if(ioctl(fb_fd, FBIOGET_FSCREENINFO, &fi) < 0) {
		perror("failed to get fb0 info");
		return -1;
	}

	if (fi.smem_len <= 0)
		return -errno;

	fb_ctx->fb_ptr = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if(fb_ctx->fb_ptr  == MAP_FAILED) {
		perror("failed to mmap framebuffer");
		return -1;
	}

	fb_ctx->fb_fd = fb_fd;
	fb_ctx->vi = vi;
	fb_ctx->fi = fi;

	return 0;

}


static void dss_get_fence_fd(int sockfd, int *release_fd)
{
	struct gdm_msghdr *msg = NULL;

	printf("dss_get_fence_fd - start\n");
	msg = gdm_recvmsg(sockfd);

	printf("received msg: %0x\n", (unsigned int)msg);
	if(msg != NULL){
		printf("msg->fds[0]: %d\n", msg->fds[0]);
		*release_fd = msg->fds[0];
		gdm_free_msghdr(msg);
	}
}

static void dss_overlay_default_fb_config(struct gdm_dss_overlay *req)
{
	memset(req, 0x00, sizeof(*req));
	req->alpha = 255;
	req->blend_op = GDM_FB_BLENDING_COVERAGE;
	req->src.width = LCD_WIDTH;
	req->src.height = LCD_HEIGHT;

	req->src.format = GDM_DSS_PF_ARGB8888;
	req->src.endian = GDM_DSS_PF_ENDIAN_BIG;
	req->src.swap = GDM_DSS_PF_ORDER_BGR;
	req->pipe_type = GDM_DSS_PIPE_TYPE_GFX;

	req->src_rect.x = req->src_rect.y = 0;

	req->src_rect.w = LCD_WIDTH;
	req->src_rect.h = LCD_HEIGHT;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = req->src_rect.w;
	req->dst_rect.h = req->src_rect.h;

	req->transp_mask = 0;
	req->flags = 0;
	req->id = GDMFB_BASE_REQUEST;

}


static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_GPU_RENDERER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_SET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);

	memcpy(msg_data.data, req, sizeof(struct gdm_dss_overlay));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	gdm_sendmsg(sockfd, msg);

	return 0;
}

/* Decoding thread */
void *framebuffer_renderer(void *arg)
{
	int ret = 0;
	int sockfd;
	struct sockaddr_un server_addr;
	int frame_count = 0;

	//struct gdm_dss_overlay req;
	//struct gdm_dss_overlay_data req_data;
	struct fb_context_t *fb_ctx = (struct fb_context_t*)arg;

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

	dss_get_fence_fd(sockfd, &fb_ctx->release_fd);
	//dss_overlay_default_fb_config(&req, gfx_ctx);


	while(!fb_ctx->bstop) {

#if 0
		load_image(fb_ctx, fb_ctx->render_ndx);
#else
		load_image(fb_ctx, fb_ctx->render_ndx, frame_count);
#endif
		fb_ctx->vi.activate = FB_ACTIVATE_VBL;
		fb_ctx->vi.yoffset = fb_ctx->vi.yres * fb_ctx->render_ndx;

		ret = ioctl(fb_ctx->fb_fd, FBIOPUT_VSCREENINFO, &fb_ctx->vi);
		if(ret) {
			printf("%s::GDMFB_OVERLAY_COMMIT fail(%s)", __func__, strerror(errno));

		}

		if(fb_ctx->release_fd != -1) {
			//printf("wait frame done signal\n");
			ret = sync_wait(fb_ctx->release_fd, 10000);
		}

		fb_ctx->render_ndx ^= 1;
		frame_count++;

	}

	close(sockfd);

	return NULL;

}


int main(int argc, char **argv)
{
	struct fb_context_t *fb_context = NULL;

	fb_context = (struct fb_context_t*)malloc(sizeof(struct fb_context_t));
	memset(fb_context, 0x00, sizeof(struct fb_context_t));

	open_framebuffer_device(fb_context);

	if(pthread_create(&fb_context->renderer_worker, NULL, framebuffer_renderer, fb_context) != 0)
		goto exit;


	while(!fb_context->bstop) {
		sleep(1);
	}

exit:
	free(fb_context);

	return 0;
}

