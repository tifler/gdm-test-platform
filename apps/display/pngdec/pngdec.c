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

#include <png.h>
#include <ion_api.h>
#include "gdm_fb.h"
#include "pngdec.h"

#include "gdm-msgio.h"
#include "debug.h"
#include "hwcomposer.h"
#include "sync.h"

#define LCD_WIDTH	800
#define LCD_HEIGHT	480

#define MAX_FILE_NUM	50
#define MAX_STRING_LEN	256

struct gfx_context_t {
	struct image_desc desc[2];
	pthread_t renderer_worker;

	int render_ndx;	// 0 or 1
	int cur_ndx;	// current decoding file num
	int tot_num;	// numer of png file

	int bstop;
	int release_fd;
	char img_filename[MAX_FILE_NUM][MAX_STRING_LEN];	// png filename
};

static unsigned int dfb_color_to_argb( const DFBColor *color )
{
     return (color->a << 24) | (color->r << 16) | (color->g << 8) | color->b;
}

int read_png_file(char* file_name, struct image_desc *desc, DFBColor *transparent)
{
	FILE		  *fp;
	png_structp    png_ptr	= NULL;
	png_infop	   info_ptr = NULL;
	png_uint_32    width, height;
	unsigned char *data 	= NULL;
	int 		   type;
	char		   header[8];
	int 		   bytes, pitch;


	int src_format, dest_format;

	int ret;

	desc->fd = ion_open();
	if (desc->fd < 0)
		return desc->fd;

	dest_format = DSPF_UNKNOWN;
	/* open file and test for it being a png */
	fp = fopen(file_name, "rb");
	if (!fp) {
		printf("[read_png_file] File %s could not be opened for reading", file_name);
		goto cleanup;
	}

	bytes = fread(header, 1, sizeof(header), fp);
	if (png_sig_cmp((unsigned char*)header, 0, bytes)) {
		printf("[read_png_file] File %s is not recognized as a PNG file", file_name);
		goto cleanup;
	}

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr){
		printf("[read_png_file] png_create_read_struct failed");
		goto cleanup;
	}

	if (setjmp(png_jmpbuf(png_ptr))){
		printf("[read_png_file] Error during init_io");
		goto cleanup;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		printf("[read_png_file] png_create_info_struct failed");
		goto cleanup;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, bytes);

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR (png_ptr, info_ptr, &width, &height, &bytes, &type, NULL, NULL, NULL);

	if(bytes == 16)
		png_set_strip_16(png_ptr);

	png_set_bgr(png_ptr);

	src_format = (type & PNG_COLOR_MASK_ALPHA) ? DSPF_ARGB : DSPF_RGB32;
	switch (type) {
		case PNG_COLOR_TYPE_GRAY:
			if (dest_format == DSPF_A8) {
				src_format = DSPF_A8;
				break;
			}
			/* fallthru */
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			png_set_gray_to_rgb (png_ptr);
			//if (rgbformat)
			//	dest_format = rgbformat;
			break;

		case PNG_COLOR_TYPE_PALETTE:
			if (dest_format == DSPF_LUT8) {
				src_format = DSPF_LUT8;
				break;
			}
			png_set_palette_to_rgb (png_ptr);
			/* fallthru */
		case PNG_COLOR_TYPE_RGB:
			//if (rgbformat)
			//	dest_format = rgbformat;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			if (dest_format == DSPF_RGB24) {
				png_set_strip_alpha (png_ptr);
				src_format = DSPF_RGB24;
			}
			break;
	}

	switch (src_format) {
		case DSPF_RGB32:
			png_set_filler (png_ptr, 0xFF,
					PNG_FILLER_AFTER
				       );
			break;
		case DSPF_ARGB:
		case DSPF_A8:
			if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
				png_set_tRNS_to_alpha (png_ptr);
			break;
		default:
			break;
	}

	pitch = width * DFB_BYTES_PER_PIXEL (src_format);
	if (pitch & 3)
		pitch += 4 - (pitch & 3);

#if 0
	desc.size = height * pitch;
	data  = malloc (height * pitch);
	if (!data) {
		fprintf (stderr, "Failed to allocate %lu bytes.\n", (unsigned long)(height * pitch));
		goto cleanup;
	}


#else
	desc->size = height * pitch;
	ret = ion_alloc_fd(desc->fd, desc->size, 0, ION_HEAP_CARVEOUT_MASK, 0, &desc->shared_fd);
	//ret = ion_share(desc->fd, desc->handle, &desc->shared_fd);
	if (ret) {
		printf("share failed %s\n", strerror(errno));
		goto cleanup;
	}

	data = mmap(NULL, desc->size, (PROT_READ | PROT_WRITE), MAP_SHARED, desc->shared_fd, 0);
	if (data == MAP_FAILED) {
		goto cleanup;
	}


	//ion_import(desc->fd, desc->shared_fd, &desc->handle);

	close(desc->fd);
#endif

	{
		unsigned int i;
		png_bytep bptrs[height];

		for (i = 0; i < height; i++)
			bptrs[i] = data + i * pitch;

		png_read_image (png_ptr, bptrs);
	}

	if (!dest_format)
		dest_format = src_format;

#if 0
	/*	replace color in completely transparent pixels	*/
	if (DFB_PIXELFORMAT_HAS_ALPHA(src_format))
	{
		unsigned char *row;
		int h;

		for (row = data, h = height; h; h--, row += pitch) {
			unsigned int *pixel;
			int  w;

			for (pixel = (unsigned int *) row, w = width; w; w--, pixel++) {
				if ((*pixel & 0xff000000) == 0x0)
					*pixel = dfb_color_to_argb (transparent);
			}
		}
	}
#endif

#if 0
	if (DFB_BYTES_PER_PIXEL(src_format) != DFB_BYTES_PER_PIXEL(dest_format)) {
		unsigned char *s, *d, *dest;
		int			d_pitch, h;

		assert (DFB_BYTES_PER_PIXEL (src_format) == 4);

		d_pitch = width * DFB_BYTES_PER_PIXEL (dest_format);
		if (d_pitch & 3)
			d_pitch += 4 - (d_pitch & 3);

		dest = malloc (height * d_pitch);
		if (!dest) {
			fprintf (stderr, "Failed to allocate %lu bytes.\n", (unsigned long)(height * d_pitch));
			goto cleanup;
		}
		h = height;
		switch (dest_format) {
			case DSPF_RGB16:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					if (dither565)
						dither_rgb16 ((u32 *) s, (u16 *) d, height - h, width);
					else
						dfb_argb_to_rgb16 ((u32 *) s, (u16 *) d, width);
				break;
			case DSPF_ARGB1555:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_argb1555 ((u32 *) s, (u16 *) d, width);
				break;
			case DSPF_RGBA5551:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_rgba5551 ((u32 *) s, (u16 *) d, width);
				break;
			case DSPF_ARGB2554:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_argb2554 ((u32 *) s, (u16 *) d, width);
				break;
			case DSPF_ARGB4444:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_argb4444 ((u32 *) s, (u16 *) d, width);
				break;
			case DSPF_ARGB8565:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_argb8565 ((u32 *) s, (u8 *) d, width);
				break;
			case DSPF_RGB332:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_rgb332 ((u32 *) s, (u8 *) d, width);
				break;
			case DSPF_A8:
				for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
					dfb_argb_to_a8 ((u32 *) s, (u8 *) d, width);
				break;
			default:
				fprintf (stderr,
						"Sorry, unsupported format conversion.\n");
				goto cleanup;
		}
		free (data);
		data = dest;
		pitch = d_pitch;
	}
#endif

	desc->flags = 1 ;//(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_PREALLOCATED);
	desc->width	   = width;
	desc->height	   = height;
	desc->pixelformat = dest_format;
	desc->pitch = pitch;
	desc->data = data;

	printf("data: %08x\n", (unsigned int)desc->data);

	data = NULL;

cleanup:
	if (fp)
		fclose (fp);

	if (png_ptr)
		png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

	if (data)
		free (data);

	return ((desc->flags) ? 0 : -1);

}



static int scanning_directory(char *base_dir, struct gfx_context_t *gfx_ctx)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;

	if((dp = opendir(base_dir)) == NULL) {
		printf("cannot open directory: %s\n", base_dir);
		return -1;
	}

	chdir(base_dir);

	while((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);

		if(S_ISDIR(statbuf.st_mode)) {
			continue;
#if 0
			/* Found a directory, but ignore. and ... */
			if(strcmp(".", entry->d_name) == 0 ||
				strcmp("..", entry->d_name) == 0)
				continue;
			printf("%*s%s/\n", depth, "", entry->d_name);

			scanning_directory(entry->d_name, depth+4);
#endif
		}
		else {
			printf("[%02d]: %s/%s\n", gfx_ctx->tot_num, base_dir, entry->d_name);
			if(strstr(entry->d_name, ".png")) {
				memset(gfx_ctx->img_filename[gfx_ctx->tot_num], 0x00, MAX_STRING_LEN);

				strcat(gfx_ctx->img_filename[gfx_ctx->tot_num], base_dir);
				strcat(gfx_ctx->img_filename[gfx_ctx->tot_num], "/");
				strcat(gfx_ctx->img_filename[gfx_ctx->tot_num], entry->d_name);
			}
			gfx_ctx->tot_num ++;

		}

	}

	chdir("..");
	closedir(dp);

	return 0;

}

static void dss_overlay_default_gfx_config(struct gdm_dss_overlay *req,
	struct gfx_context_t *gfx_ctx)
{
	struct image_desc *desc = &gfx_ctx->desc[gfx_ctx->render_ndx];
	memset(req, 0x00, sizeof(*req));
	req->alpha = 255;
	req->blend_op = GDM_FB_BLENDING_COVERAGE;
	req->src.width = desc->width;
	req->src.height = desc->height;

	if(desc->pixelformat == DSPF_RGB24) {
		req->src.format = GDM_DSS_PF_RGB888;
		//req->src.endian = GDM_DSS_PF_ENDIAN_BIG;
		req->src.swap = GDM_DSS_PF_ORDER_BGR;
	}
	else {
		req->src.format = GDM_DSS_PF_ARGB8888;
		req->src.endian = GDM_DSS_PF_ENDIAN_BIG;
		req->src.swap = GDM_DSS_PF_ORDER_BGR;
	}
	req->pipe_type = GDM_DSS_PIPE_TYPE_GFX;

	req->src_rect.x = req->src_rect.y = 0;

	if(req->src.width > LCD_WIDTH)
		req->src_rect.w = LCD_WIDTH;
	else
		req->src_rect.w = req->src.width;

	if(req->src.height > LCD_HEIGHT)
		req->src_rect.h = LCD_HEIGHT;
	else
		req->src_rect.h = req->src.height;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = req->src_rect.w;
	req->dst_rect.h = req->src_rect.h;

	req->transp_mask = 0;
	req->flags = 0;
	req->id = GDMFB_NEW_REQUEST;

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



static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_GRAP_PNG_DECODER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_SET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);

	memcpy(msg_data.data, req, sizeof(struct gdm_dss_overlay));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	gdm_sendmsg(sockfd, msg);

	return 0;
}



static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data)
{
	int i;
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), req_data->num_plane);

	msg_data.app_id = APP_ID_GRAP_PNG_DECODER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_PLAY;
	memcpy(msg_data.data, req_data, sizeof(struct gdm_dss_overlay_data));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	for(i=0; i<req_data->num_plane; i++)
		msg->fds[i] = req_data->data[i].memory_id;
	gdm_sendmsg(sockfd, msg);
	return 0;
}

static int dss_overlay_unset(int sockfd)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));
	msg_data.app_id = APP_ID_GRAP_PNG_DECODER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_UNSET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);

	if(msg) {
		memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
		gdm_sendmsg(sockfd, msg);

		return 0;
	}

	return -1;

}

/* Decoding thread */
void *gfx_renderer(void *arg)
{
	int i;
	int cnt_loop = 0;
	int ret = 0;
	int sockfd;
	struct sockaddr_un server_addr;

	struct gfx_context_t *gfx_ctx = (struct gfx_context_t *)arg;
	struct gdm_dss_overlay req;
	struct gdm_dss_overlay_data req_data;
	DFBColor transparent = { 80, 40, 40, 40};

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

	dss_get_fence_fd(sockfd, &gfx_ctx->release_fd);
#if 0
	read_png_file(gfx_ctx->img_filename[gfx_ctx->cur_ndx], &gfx_ctx->desc[gfx_ctx->render_ndx], &transparent);
	// step-02: request overlay to display
	dss_overlay_default_gfx_config(&req, gfx_ctx);
	dss_overlay_set(sockfd, &req);
	req_data.id = 0;
	req_data.num_plane = 1;
	req_data.data[0].memory_id = gfx_ctx->desc[gfx_ctx->render_ndx].shared_fd;
	req_data.data[0].offset = 0;

	dss_overlay_queue(sockfd, &req_data);

	// step-03: send framebuffer to display
	gfx_ctx->render_ndx ^= 1;
	gfx_ctx->cur_ndx ++;

	if(gfx_ctx->cur_ndx >= gfx_ctx->tot_num)
		gfx_ctx->cur_ndx = 0;
#endif

	while(!gfx_ctx->bstop) {
		read_png_file(gfx_ctx->img_filename[gfx_ctx->cur_ndx], &gfx_ctx->desc[gfx_ctx->render_ndx], &transparent);
		// step-02: request overlay to display
		memset(&req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
		req_data.id = 0;
		req_data.num_plane = 1;
		req_data.data[0].memory_id = gfx_ctx->desc[gfx_ctx->render_ndx].shared_fd;
		req_data.data[0].offset = 0;


		printf("[%s]req_data.data[0].memory_id: %d\n", __FUNCTION__, req_data.data[0].memory_id);

		dss_overlay_default_gfx_config(&req, gfx_ctx);
		dss_overlay_set(sockfd, &req);
		dss_overlay_queue(sockfd, &req_data);

		if(gfx_ctx->release_fd != -1) {
			//printf("wait frame done signal\n");
			ret = sync_wait(gfx_ctx->release_fd, 10000);
		}

		gfx_ctx->render_ndx ^= 1;
		gfx_ctx->cur_ndx ++;

		if(gfx_ctx->cur_ndx >= gfx_ctx->tot_num)
			gfx_ctx->cur_ndx = 0;


		if(gfx_ctx->desc[gfx_ctx->render_ndx].shared_fd > 0) {
			munmap(gfx_ctx->desc[gfx_ctx->render_ndx].data, gfx_ctx->desc[gfx_ctx->render_ndx].size);
			close(gfx_ctx->desc[gfx_ctx->render_ndx].shared_fd);
			memset(&gfx_ctx->desc[gfx_ctx->render_ndx], 0x00, sizeof(struct image_desc));
		}

		cnt_loop ++;

		usleep(1000*1000);
		if(cnt_loop == 1000)
			gfx_ctx->bstop = 1;

	}

	// unset
	dss_overlay_unset(sockfd);

	if(gfx_ctx->release_fd != -1) {
		//printf("wait frame done signal\n");
		ret = sync_wait(gfx_ctx->release_fd, 10000);
	}

	if(gfx_ctx->release_fd != -1)
		close(gfx_ctx->release_fd);


	for(i = 0; i< 2; i++) {
		if(gfx_ctx->desc[i].shared_fd > 0) {
			munmap(gfx_ctx->desc[i].data, gfx_ctx->desc[i].size);
			close(gfx_ctx->desc[i].shared_fd);
		}
	}
	close(sockfd);

	return NULL;

}


int main(int argc, char **argv)
{
	struct gfx_context_t *gfx_context = NULL;

	gfx_context = (struct gfx_context_t*)malloc(sizeof(struct gfx_context_t));
	memset(gfx_context, 0x00, sizeof(struct gfx_context_t));


	scanning_directory(argv[1], gfx_context);

	if(pthread_create(&gfx_context->renderer_worker, NULL, gfx_renderer, gfx_context) != 0)
		goto exit;


	while(!gfx_context->bstop) {
		sleep(1);
	}

exit:
	free(gfx_context);

	return 0;
}

