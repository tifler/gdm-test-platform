/* fbtest program */

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
#include <signal.h>

#include "android_wrapper.h"
#include "gdm_dss_wrapper.h"
#include "hwcomposer.h"
#include "gdm-msgio.h"
#include "debug.h"

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


static int open_framebuffer_device(struct hwc_context_t *ctx)
{
	int fb_fd = 0;

	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;

	float fps = 10.0f;	// TODO

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

	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].fd = fb_fd;
	//xres, yres may not be 32 aligned
	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].stride = fi.line_length /(vi.xres/8);
	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].xres = vi.xres;
	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].yres = vi.yres;
	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].vsync_period = 1000000000l / fps;

	//Unblank primary on first boot
	if(ioctl(fb_fd, FBIOBLANK,FB_BLANK_UNBLANK) < 0) {
		ALOGE("%s: Failed to unblank display", __FUNCTION__);
		return -errno;
	}

	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].isActive = true;
	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].vi = vi;
	ctx->dpyAttr[HWC_DISPLAY_PRIMARY].fi = fi;


	ctx->release_fence = -1;

	return 0;

}

static struct overlay_context_t *find_registered_overlay(struct hwc_context_t *hwc_ctx, int app_id)
{
	int i;
	struct overlay_context_t *cur_ov;

	for(i = 0 ; i< MAX_VID_OVERLAY ; i++) {
		cur_ov = &hwc_ctx->vid_cfg[i];
		if(cur_ov->application_id == app_id)
			return cur_ov;
	}

	for(i = 0 ; i< MAX_GFX_OVERLAY ; i++) {
		cur_ov = &hwc_ctx->gfx_cfg[i];
		if(cur_ov->application_id == app_id)
			return cur_ov;
	}

	cur_ov = &hwc_ctx->fb_cfg;
	if(cur_ov->application_id == app_id)
		return cur_ov;

	return NULL;
}

static struct overlay_context_t *find_empty_overlay(struct hwc_context_t *hwc_ctx, int pipe_type)
{
	int i;
	struct overlay_context_t *cur_ov;

	if(pipe_type == GDM_DSS_PIPE_TYPE_VIDEO) {
		for(i = 0 ; i< MAX_VID_OVERLAY ; i++) {
			cur_ov = &hwc_ctx->vid_cfg[i];
			if(cur_ov->application_id == 0x00)
				return cur_ov;
		}
	}
	else if(pipe_type == GDM_DSS_PIPE_TYPE_GFX) {
		for(i = 0 ; i< MAX_GFX_OVERLAY ; i++) {
			cur_ov = &hwc_ctx->gfx_cfg[i];
			if(cur_ov->application_id == 0x00)
				return cur_ov;
		}
	}
	else if(pipe_type == GDM_DSS_PIPE_TYPE_FB) {
		cur_ov = &hwc_ctx->fb_cfg;
		if(cur_ov->application_id == 0x00)
			return cur_ov;

	}

	return NULL;

}

static int register_overlay_cfg(struct hwc_context_t *hwc_ctx,
		int app_id, struct gdm_dss_overlay *req)
{

	struct overlay_context_t *cur_ov = NULL;;

	cur_ov = find_registered_overlay(hwc_ctx, app_id);

	if(cur_ov == NULL) {
		cur_ov = find_empty_overlay(hwc_ctx, req->pipe_type);

		if(cur_ov == NULL) {
			return -1;
		}
		else {
			cur_ov->application_id = app_id;
			memcpy(&cur_ov->ov_cfg, req, sizeof(*req));
			cur_ov->is_update = 1;
			cur_ov->ov_id = cur_ov->ov_cfg.id;
		}

	}
	else {
		memcpy(&cur_ov->ov_cfg, req, sizeof(*req));
		cur_ov->ov_cfg.id = cur_ov->ov_id;
		cur_ov->is_update = 1;
	}

	return 0;
}

static int register_overlay_data(struct hwc_context_t *hwc_ctx,
		int app_id, struct gdm_msghdr *msg)
{
	int i = 0;
	struct overlay_context_t *cur_ov = NULL;;
	struct gdm_hwc_msg *hwc_msg = NULL;
	struct gdm_dss_overlay_data *req_data = NULL;


	hwc_msg = (struct gdm_hwc_msg*)msg->buf;
	req_data = (struct gdm_dss_overlay_data *)hwc_msg->data;

	for(i = 0; i<msg->fdcount; i++) {
		req_data->data[i].memory_id = msg->fds[i];
	}

	cur_ov = find_registered_overlay(hwc_ctx, app_id);

	if(cur_ov == NULL) {
		return -1;
	}
	else {
		memcpy(&cur_ov->ov_data, req_data, sizeof(*req_data));
		cur_ov->ov_data.id = cur_ov->ov_id;
		cur_ov->is_new_data = 1;
	}

	return 0;
}

static int unregister_overlay_cfg(struct hwc_context_t *hwc_ctx,
		int app_id)
{
	struct overlay_context_t *cur_ov = NULL;;

	cur_ov = find_registered_overlay(hwc_ctx, app_id);

	if(cur_ov == NULL) {
		return -1;
	}
	else {
		gdss_io_unset_overlay(hwc_ctx->dpyAttr[HWC_DISPLAY_PRIMARY].fd, cur_ov->ov_id);
		memset(cur_ov, 0x00, sizeof(struct overlay_context_t));
	}

	return 0;
}


// default thread fb0
void *commit_thread(void *argp)
{
	int i = 0, ret = 0, j = 0;
	struct hwc_context_t *hwc_ctx = (struct hwc_context_t *)argp;
	struct fb_var_screeninfo *vi = &hwc_ctx->dpyAttr[0].vi;
	int fb_fd = hwc_ctx->dpyAttr[0].fd;
	int buf_index = 0;
	struct gdm_dss_buf_sync buf_sync;

	pthread_detach(pthread_self());

	memset(&buf_sync, 0x00, sizeof(buf_sync));

	buf_sync.acq_fen_fd_cnt = 0;
	buf_sync.rel_fen_fd = &hwc_ctx->release_fence;
	gdss_io_buffer_sync(fb_fd, &buf_sync);

	while(1) {

		pthread_cond_wait(&hwc_ctx->commit_cond, &hwc_ctx->ov_lock);
		buf_index ^= 1;

		/* prepare */
		//pthread_mutex_lock(&hwc_ctx->ov_lock);

		for(i = 0; i < MAX_VID_OVERLAY; i++) {
			if(hwc_ctx->vid_cfg[i].application_id != 0) {
				if(hwc_ctx->vid_cfg[i].is_update) {
					gdss_io_set_overlay(fb_fd, &hwc_ctx->vid_cfg[i].ov_cfg);
					hwc_ctx->vid_cfg[i].ov_id = hwc_ctx->vid_cfg[i].ov_cfg.id;
					hwc_ctx->vid_cfg[i].is_update = 0;
				}

				if(hwc_ctx->vid_cfg[i].ov_data.data[0].memory_id != 0 && hwc_ctx->vid_cfg[i].is_new_data == 1) {
					hwc_ctx->vid_cfg[i].ov_data.id = hwc_ctx->vid_cfg[i].ov_id;
					gdss_io_overlay_queue(fb_fd, &hwc_ctx->vid_cfg[i].ov_data);
					hwc_ctx->vid_cfg[i].is_new_data = 0;

					for(j = 0 ; j< hwc_ctx->vid_cfg[i].ov_data.num_plane; j++)
						close(hwc_ctx->vid_cfg[i].ov_data.data[j].memory_id);
				}
			}
		}

		for(i = 0; i < MAX_GFX_OVERLAY; i++) {
			if(hwc_ctx->gfx_cfg[i].application_id != 0) {
				if(hwc_ctx->gfx_cfg[i].is_update) {
					gdss_io_set_overlay(fb_fd, &hwc_ctx->gfx_cfg[i].ov_cfg);
					hwc_ctx->gfx_cfg[i].ov_id = hwc_ctx->gfx_cfg[i].ov_cfg.id;
					hwc_ctx->gfx_cfg[i].is_update = 0;
				}

				if(hwc_ctx->gfx_cfg[i].ov_data.data[0].memory_id != 0 && hwc_ctx->gfx_cfg[i].is_new_data == 1) {
					hwc_ctx->gfx_cfg[i].ov_data.id = hwc_ctx->gfx_cfg[i].ov_id;
					gdss_io_overlay_queue(fb_fd, &hwc_ctx->gfx_cfg[i].ov_data);
					hwc_ctx->gfx_cfg[i].is_new_data = 0;
					close(hwc_ctx->gfx_cfg[i].ov_data.data[0].memory_id);
				}
			}
		}
		pthread_mutex_unlock(&hwc_ctx->ov_lock);

		/* commit */
		vi->activate = FB_ACTIVATE_VBL;
		vi->yoffset = vi->yres * buf_index;

		ret = ioctl(fb_fd, FBIOPUT_VSCREENINFO, vi);
		if(ret) {
			printf("%s::GDMFB_OVERLAY_COMMIT fail(%s)", __func__, strerror(errno));

		}
		//usleep(10*1000);
	}

}


static void *client_thread(void *arg)
{
	struct gdm_msghdr *cmd_msg;
	struct gdm_msghdr *resp_msg;
	struct client *client = (struct client *)arg;
	struct hwc_context_t *hwc_ctx = (struct hwc_context_t*)client->usr_data;;
	struct gdm_hwc_msg *disp_message;

	printf("Client %d launched.\n", getpid());

	resp_msg = gdm_alloc_msghdr(10, 1);
	resp_msg->fds[0] = hwc_ctx->release_fence;
	gdm_sendmsg(client->sockfd, resp_msg);

	while(1) {
		cmd_msg = gdm_recvmsg(client->sockfd);
		ASSERT(cmd_msg);

		//DBG("cmd_msg->buf = %s", cmd_msg->buf);
		//DBG("cmd_msg->buflen = %d", (int)cmd_msg->buflen);
		//DBG("cmd_msg->fdcount = %d", (int)cmd_msg->fdcount);

		disp_message = (struct gdm_hwc_msg*)cmd_msg->buf;
		pthread_mutex_lock(&hwc_ctx->ov_lock);
		switch(disp_message->hwc_cmd) {
		case GDMFB_OVERLAY_GET:
			break;
		case GDMFB_OVERLAY_SET:
			register_overlay_cfg(hwc_ctx, disp_message->app_id,
					(struct gdm_dss_overlay*)disp_message->data);
			break;
		case GDMFB_OVERLAY_UNSET:
			unregister_overlay_cfg(hwc_ctx, disp_message->app_id);
			gdm_free_msghdr(cmd_msg);
			pthread_mutex_unlock(&hwc_ctx->ov_lock);
		//	hwc_ctx->bstop = 1;
			printf("client_exit\n");
			goto Exit;

		case GDMFB_OVERLAY_PLAY:
			register_overlay_data(hwc_ctx, disp_message->app_id,
					cmd_msg);
			break;
		}
		pthread_cond_signal(&hwc_ctx->commit_cond);
		pthread_mutex_unlock(&hwc_ctx->ov_lock);
		gdm_free_msghdr(cmd_msg);

	}
Exit:
	close(client->sockfd);
	free(client);

	return NULL;
}

static void process_client(int sockfd,
		const struct sockaddr *addr, socklen_t addrlen, void *argp)
{
	int ret;
	struct client *client;

	DBG("Client accepted. sock = %d", sockfd);

	client = calloc(1, sizeof(*client));
	ASSERT(client);

	client->sockfd = sockfd;
	memcpy(&client->addr, addr, addrlen);
	client->usr_data = argp;

	ret = pthread_create(
			&client->thid, (pthread_attr_t *)NULL, client_thread, client);
	ASSERT(ret == 0);

	pthread_detach(client->thid);
}

void *server_loop(void *argp)
{
	int ret;
	int server_sock;
	int client_sock;
	socklen_t client_len = 0;
	struct sockaddr_un server_addr, client_addr;
	struct hwc_context_t *hwc_ctx;

	ASSERT(UNIX_SOCKET_PATH);

	unlink(UNIX_SOCKET_PATH);

	hwc_ctx = (struct hwc_context_t *)argp;

	server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT(server_sock > 0);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, UNIX_SOCKET_PATH);

	ret = bind(server_sock,
			(struct sockaddr *)&server_addr, sizeof(server_addr));
	ASSERT(ret == 0 && "bind() failed");

	ret = listen(server_sock, BACKLOG);
	ASSERT(ret == 0);

	DBG("Server is wating(server_sock = %d)...", server_sock);

	for ( ; ; ) {
		bzero(&client_addr, sizeof(client_addr));
		client_sock =
			accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

		printf("client_sock: %d\n", client_sock);

		if (client_sock < 0) {
			perror("accept");
			continue;
		}

		process_client(client_sock, (struct sockaddr *)&client_addr, client_len, hwc_ctx);
	}

	close(server_sock);

	return 0;
}


int main(int argc, char **argv)
{
	struct hwc_context_t *hwc_ctx = NULL;
	pthread_t commitThread, serverThread;;


	signal(SIGPIPE, SIG_IGN);

	hwc_ctx = (struct hwc_context_t *)malloc(sizeof(struct hwc_context_t));
	memset(hwc_ctx, 0x00, sizeof(*hwc_ctx));

	open_framebuffer_device(hwc_ctx);

	if(pthread_cond_init(&hwc_ctx->commit_cond, NULL) != 0) {
		goto Exit;
	}

	pthread_mutex_init(&hwc_ctx->ov_lock, NULL);

	pthread_create(&commitThread, NULL, commit_thread, hwc_ctx);
	pthread_create(&serverThread, NULL, server_loop, hwc_ctx);

	pthread_detach(serverThread);
	pthread_detach(commitThread);

	while(!hwc_ctx->bstop)
		sleep(1);

Exit:
	free(hwc_ctx);
	return 0;
}

