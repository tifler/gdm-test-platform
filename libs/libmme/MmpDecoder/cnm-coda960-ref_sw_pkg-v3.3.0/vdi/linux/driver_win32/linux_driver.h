#ifndef WIN32_LINUX_DRIVER_H__
#define WIN32_LINUX_DRIVER_H__

#include "MmpDefine.h"

#define __user 
#define EFAULT 1
typedef unsigned int gfp_t;
typedef unsigned int dma_addr_t;

#define GFP_DMA (1<<0)
//#define GFP_KERNEL (1<<1)

#define KERN_ERR
#define printk printf
#define PAGE_ALIGN(x) x

struct device {
    int dummy;
};  

#ifdef __cplusplus
extern "C" {
#endif

int copy_to_user(void* dest, void* src, int size);
int copy_from_user(void* dest, void* src, int size);

void* dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag);

#ifdef __cplusplus
}
#endif

#endif
