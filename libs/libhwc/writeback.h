#ifndef GDM_HWC_WRITEBACK_H
#define GDM_HWC_WRITEBACK_H

#include "gdm_fb.h"

struct writeback_ctx_t {

	int fd;
	int xres;
	int yres;
	int format;

	struct gdmfb_data fb_data;
	struct gdmfb_data out_data;

	bool (*start)(struct writeback_ctx_t *wb);
	bool (*stop)(struct writeback_ctx_t *wb);
	bool (*queue)(struct writeback_ctx_t *wb, int mem_id, int offset);
	bool (*dequeue)(struct writeback_ctx_t *wb);
	bool (*sync)(struct writeback_ctx_t *wb);


};

#ifdef __cplusplus
extern "C"
{
#endif

int init_writeback_ctx(void *hwc_ctx);
void exit_writeback_ctx(void *hwc_ctx);

#ifdef __cplusplus
}
#endif


#endif // GDM_HWC_WRITEBACK_H
