
#ifndef GDM_DSS_WRAPPER_H
#define GDM_DSS_WRAPPER_H


#include <sys/ioctl.h>
#include <errno.h>
#include "gdm_fb.h"
#include "android_wrapper.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* FBIOGET_FSCREENINFO */
int gdss_io_get_fscreen_info(int fd, struct fb_fix_screeninfo *finfo);
/* FBIOGET_VSCREENINFO */
int gdss_io_get_vscreen_info(int fd, struct fb_var_screeninfo *vinfo);
/* FBIOPUT_VSCREENINFO */
int gdss_io_set_vscreen_info(int fd, struct fb_var_screeninfo *vinfo);
/* GDMFB_OVERLAY_SET */
int gdss_io_set_overlay(int fd, struct gdm_dss_overlay *ov);
/* GDMFB_OVERLAY_UNSET */
int gdss_io_unset_overlay(int fd, int ov_id);
/* GDMFB_OVERLAY_GET */
int gdss_io_get_overlay(int fd, struct gdm_dss_overlay *ov);
/* GDMFB_OVERLAY_PLAY */
int gdss_io_overlay_queue(int fd, struct gdm_dss_overlay_data *od);
/* GDMFB_DISPLAY_COMMIT */
int gdss_io_display_commit(int fd, struct gdm_display_commit *info);
/* GDMFB_BUFFER_SYNC */
int gdss_io_buffer_sync(int fd, struct gdm_dss_buf_sync *buf_sync);
/* GDMFB_WRITEBACK_INIT, GDMFB_WRITEBACK_START */
int gdss_io_writeback_start(int fd);
/* GDMFB_WRITEBACK_STOP, GDMFB_WRITEBACK_TERMINATE */
int gdss_io_writeback_terminate(int fd);
/* GDMFB_WRITEBACK_QUEUE_BUFFER */
int gdss_io_writeback_queue_buffer(int fd, struct gdmfb_data *fb_data);
/* GDMFB_WRITEBACK_DEQUEUE_BUFFER */
int gdss_io_writeback_dequeue_bufer(int fd, struct gdmfb_data *fb_data);
/* GDMFB_BUFFER_SYNC */
int gdss_io_overlay_buf_sync(int fd, struct gdm_dss_buf_sync *buf_sync);

#ifdef __cplusplus
}
#endif


#endif // GDM_DSS_WRAPPER_H
