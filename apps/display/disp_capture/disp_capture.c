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
//#include <sys/socket.h>
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
#include <linux/fb.h>
#include <ion_api.h>
#include "gdm_dss_wrapper.h"

#define FRAMEBUFFER_NAME	"/dev/fb1"

struct ody_framebuffer {
	int fd;
	struct fb_var_screeninfo vi;
	struct fb_fix_screeninfo fi;

	int buffer_index;
};


static int save_capture_img(int id, unsigned char *ptr, int size)
{
	FILE *fp = NULL;
	char filename[32];

	sprintf(filename, "/mnt/capture_%04d.bin", id);
	fp = fopen(filename, "wb");
	if(fp!= NULL) {
		fwrite(ptr, 1, size, fp);
		fclose(fp);
		return 0;
	}

	return 1;

}

static int open_framebuffer(void)
{
	int fd = 0;

	fd = open(FRAMEBUFFER_NAME, O_RDWR);
	if (fd < 0) {
		printf("cannot open /dev/fb1\n");
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
	int blank = FB_BLANK_UNBLANK;
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



int main(int argc, char **argv)
{
	long type;

	struct ody_framebuffer framebuffer;
	struct gdmfb_data capture_buffer[2];
	struct gdmfb_data capture_output;

	/* ion */
	int ion_fd = -1, shared_fd = -1, size;
	struct ion_handle *handle;
	unsigned char *ptr[2] = {NULL};
	int ret;
	int i = 0;

	int val;

	framebuffer.fd = open_framebuffer();

	get_framebuffer(framebuffer.fd, &framebuffer.vi,
		&framebuffer.fi);

	memset(capture_buffer, 0x00, sizeof(struct gdmfb_data)*2);

	ion_fd = ion_open();
	if (ion_fd < 0)
		goto cleanup;

	size = framebuffer.vi.xres * framebuffer.vi.yres * sizeof(unsigned int);

	ret = ion_alloc_fd(ion_fd, size, 0, ION_HEAP_CARVEOUT_MASK, 0, &capture_buffer[0].memory_id);
	if (ret) {
		printf("share failed %s\n", strerror(errno));
		goto cleanup;
	}

	ptr[0] = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, capture_buffer[0].memory_id, 0);
	if (ptr[0] == MAP_FAILED) {
		goto cleanup;
	}


	ret = ion_alloc_fd(ion_fd, size, 0, ION_HEAP_CARVEOUT_MASK, 0, &capture_buffer[1].memory_id);
	if (ret) {
		printf("share failed %s\n", strerror(errno));
		goto cleanup;
	}

	ptr[1] = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, capture_buffer[1].memory_id, 0);
	if (ptr[1] == MAP_FAILED) {
		goto cleanup;
	}


	val = 1;	// MIXER
	ret = ioctl(framebuffer.fd, GDMFB_WRITEBACK_INIT, &val);
	ret = ioctl(framebuffer.fd, GDMFB_WRITEBACK_QUEUE_BUFFER, &capture_buffer[0]);
	ret = ioctl(framebuffer.fd, GDMFB_WRITEBACK_QUEUE_BUFFER, &capture_buffer[1]);
	ret = ioctl(framebuffer.fd, GDMFB_WRITEBACK_START);
	i = 0;

	while(i < 10) {
		int ndx = (i%2);
		ioctl(framebuffer.fd, GDMFB_OVERLAY_COMMIT);

		ioctl(framebuffer.fd, GDMFB_WRITEBACK_DEQUEUE_BUFFER, &capture_output);

		if(capture_output.memory_id == capture_buffer[0].memory_id)
			save_capture_img(i, ptr[0], size);
		else
			save_capture_img(i, ptr[1], size);

		ioctl(framebuffer.fd, GDMFB_WRITEBACK_QUEUE_BUFFER, &capture_output);
		i ++;

	}

	ioctl(framebuffer.fd, GDMFB_WRITEBACK_STOP);
	ioctl(framebuffer.fd, GDMFB_WRITEBACK_TERMINATE);

cleanup:

	if(!ptr[0] && ptr[0] != MAP_FAILED)
		munmap(ptr[0], size);

	if(!ptr[1] && ptr[1] != MAP_FAILED)
		munmap(ptr[1], size);

	if(capture_buffer[0].memory_id >= 0)
		close(capture_buffer[0].memory_id);


	if(capture_buffer[1].memory_id >= 0)
		close(capture_buffer[1].memory_id);

	close(framebuffer.fd);

	return 0;
}

