#ifndef HWCOMPOSER_INCLUDE_H
#define HWCOMPOSER_INCLUDE_H


#include <linux/types.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <inttypes.h>

#include "android_wrapper.h"

#define MAX_DISPLAYS            	(2)
#define UNIX_SOCKET_PATH                "/tmp/sock_msgio"
#define BACKLOG                         (10)


#define APP_ID_MULTI_SAMPLE_PLAYER	0x00001000
#define APP_ID_GRAP_PNG_DECODER		0x00002000
#define APP_ID_CAMERA_PREVIEW		0x00004000
#define APP_ID_GPU_RENDERER		0x00008000


/*****************************************************************************/

struct client {
	int sockfd;
	struct sockaddr addr;
	pthread_t thid;
	void *usr_data;
};

struct gdm_hwc_msg {
	uint32_t app_id;
	uint32_t hwc_cmd;
	uint8_t data[256];
};


/* Display types and associated mask bits. */
enum {
	HWC_DISPLAY_PRIMARY     = 0,
	HWC_DISPLAY_VIRTUAL     = 1,

	HWC_NUM_PHYSICAL_DISPLAY_TYPES = 1,
	HWC_NUM_DISPLAY_TYPES          = 2,
};

#define MAX_SD_LEN 		10
#define MAX_OVERLAY_REQUEST	6

#define MAX_VID_OVERLAY		2
#define MAX_GFX_OVERLAY		3

struct overlay_context_t {
	bool is_update;
	bool is_new_data;
	int ov_id;
	struct gdm_dss_overlay ov_cfg;
	struct gdm_dss_overlay_data ov_data;
	int application_id;
	int acquire_fence;
	int release_fence;
};

struct display_attributes {
	uint32_t vsync_period; //nanos
	uint32_t xres;
	uint32_t yres;
	uint32_t stride;

	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;


	int fd;
	bool connected; //Applies only to secondary displays
	//Connected does not mean it ready to use.
	//It should be active also. (UNBLANKED)
	bool isActive;

	// In pause state, composition is bypassed
	// used for Writeback displays only
	bool isPause;
	//Secondary displays will have this set until they are able to acquire
	//pipes.
	bool isConfiguring;
};

/* context of each server thread */
struct comm_context_t {
	int sd;
	int sd_len;
	int id;
	pthread_t threadID;
};

struct hwc_context_t {

	bool bstop;

	pthread_cond_t	commit_cond;
	pthread_mutex_t ov_lock;

	int release_fence;

	struct comm_context_t comm_cfg;
	struct display_attributes dpyAttr[MAX_DISPLAYS];	// fb0: physical, fb1: virtual for writeback

	struct overlay_context_t vid_cfg[MAX_VID_OVERLAY];
	struct overlay_context_t gfx_cfg[MAX_GFX_OVERLAY];
	struct overlay_context_t fb_cfg;
};

typedef struct {
	struct hwc_context_t *pc;
	int fd;
} cfd;



#endif // HWCOMPOSER_INCLUDE_H
