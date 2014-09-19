

#ifndef __JPU_DRV_H__
#define __JPU_DRV_H__

#include <linux/fs.h>
#include <linux/types.h>

#if defined JPU_CONFIG_ION_RESERVED_MEMORY
// ION Feature
#include <linux/dma-buf.h>
#include <linux/memblock.h>
#include "../../../../../../../linux/drivers/staging/android/ion/ion.h"
#include "../../../../../../../linux/drivers/staging/android/sw_sync.h"
#include "../../../../../../../linux/drivers/staging/android/uapi/gdm_ion.h"
#endif

#define JDI_IOCTL_MAGIC  'J'


#define JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY			_IO(JDI_IOCTL_MAGIC, 0)
#define JDI_IOCTL_FREE_PHYSICALMEMORY			_IO(JDI_IOCTL_MAGIC, 1)
#define JDI_IOCTL_WAIT_INTERRUPT			_IO(JDI_IOCTL_MAGIC, 2)
#define JDI_IOCTL_SET_CLOCK_GATE			_IO(JDI_IOCTL_MAGIC, 3)
#define JDI_IOCTL_RESET						_IO(JDI_IOCTL_MAGIC, 4)
#define JDI_IOCTL_GET_INSTANCE_POOL			_IO(JDI_IOCTL_MAGIC, 5)
#define JDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO			_IO(JDI_IOCTL_MAGIC, 6)
#define JDI_IOCTL_OPEN_INSTANCE				_IO(JDI_IOCTL_MAGIC, 7)
#define JDI_IOCTL_CLOSE_INSTANCE			_IO(JDI_IOCTL_MAGIC, 8)
#define JDI_IOCTL_GET_INSTANCE_NUM			_IO(JDI_IOCTL_MAGIC, 9)

#if defined JPU_CONFIG_ION_RESERVED_MEMORY
// ION Feature
struct jb_dma_buf_data {
	struct ion_handle *ion_handle;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attachment;
	struct sg_table *sg_table;
	dma_addr_t dma_addr;
	struct sync_fence *fence;
};

struct gdm_codec_drv_data {
    // ION Feature
    struct ion_client *iclient; /* ION iclient */	 
    struct jb_dma_buf_data dma_buf_data[16];/* ION dma_buf_data */	
    struct platform_device *pdev;
};
#endif

typedef struct jpudrv_buffer_t {
	unsigned int size;
	unsigned long long phys_addr;
	unsigned long base;                     /* kernel logical address in use kernel */
	unsigned long virt_addr;                /* virtual user space address */
	unsigned int ion_shared_fd;             /* ion fd buffer index value */
} jpudrv_buffer_t;


#define jpu_loge(fmt, ...) printk(KERN_ERR     "[ERROR][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define jpu_logw(fmt, ...) printk(KERN_WARNING "[WARN ][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define jpu_logi(fmt, ...) printk(KERN_WARNING "[INFO ][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define jpu_logd(fmt, ...) printk(KERN_WARNING "[DEBUG][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif
