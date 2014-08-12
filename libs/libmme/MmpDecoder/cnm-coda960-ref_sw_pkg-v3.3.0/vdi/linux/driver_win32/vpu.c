#include "MmpDefine.h"
#include "win32_port.h"
#include "linux_driver.h"
#include "vpu.h"
#include "wchar.h"

#define VPU_SUPPORT_RESERVED_VIDEO_MEMORY
#define MAX_NUM_VPU_CORE 1

#	define SUPPORT_INTERRUPT
#	define VPU_BIT_REG_SIZE	(0x4000*MAX_NUM_VPU_CORE)
#		define VDI_SRAM_BASE_ADDR	ANA_CODEC_SRAM_BASE	// if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#		define VDI_SRAM_SIZE		0x20000		// FHD MAX size, 0x17D00  4K MAX size 0x34600
#	define VDI_SYSTEM_ENDIAN VDI_LITTLE_ENDIAN


#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
#define VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (48*1024*1024)
//#define VPU_DRAM_PHYSICAL_BASE 0x86C00000
#define VPU_DRAM_PHYSICAL_BASE 0xdC000000 // 
//#include "vmm.h"
//static video_mm_t s_vmem;
//static vpudrv_buffer_t s_video_memory = {0};
#else
#endif /*VPU_SUPPORT_RESERVED_VIDEO_MEMORY*/

//static vpudrv_buffer_t s_instance_pool = {0};

/* Common Buffer

   --------------
    CODE_BUF  

   -------------- 
    TEMP_BUF  

   -------------- 
    WORK_BUF

   -------------- 
    PARA_BUF

#   define CODE_BUF_SIZE      (248*1024)
#   define TEMP_BUF_SIZE      (204*1024)
#   define WORK_BUF_SIZE      (80*1024) // to be updated 320*1024

*/

struct vpu_driver {
    int clock_enable;

    vpudrv_buffer_t instance_pool;
    vpudrv_buffer_t common_memory;
    
    //unsigned long mem_size;
    //unsigned long mem_phyaddr;
    //unsigned long mem_base;
    vpudrv_buffer_t video_memory;
    unsigned long video_memory_alloc_offset;
};


static int vpu_alloc_dma_buffer(struct vpu_driver *p_vpu_driver, vpudrv_buffer_t *vb)
{
	if (!vb)
		return -1;

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY

#if (MMP_OS == MMP_OS_WIN32)

    vb->virt_addr = 0;
    vb->phys_addr = p_vpu_driver->video_memory_alloc_offset;//p_vpu_driver->video_memory.phys_addr + p_vpu_driver->video_memory_alloc_offset;
    vb->base = (unsigned long)(p_vpu_driver->video_memory.base + vb->phys_addr);//(vb->phys_addr - p_vpu_driver->video_memory.phys_addr));
    
    p_vpu_driver->video_memory_alloc_offset += MMP_BYTE_ALIGN(vb->size, 16);

#else
	vb->phys_addr = (unsigned long)vmem_alloc(&s_vmem, vb->size, 0);
	if ((unsigned long)vb->phys_addr  == (unsigned long)-1) {
		vpu_loge("reserved Physical memory allocation error size=%d, base_addr=0x%x, mem_size=%d\n", vb->size, (int)s_vmem.base_addr, (int)s_vmem.mem_size);
		return -1;
	}
    vb->base = (unsigned long)(s_video_memory.base + (vb->phys_addr - s_video_memory.phys_addr));
#endif

	
#else
	vb->base = (unsigned long)dma_alloc_coherent(NULL, PAGE_ALIGN(vb->size), (dma_addr_t *) (&vb->phys_addr), GFP_DMA | GFP_KERNEL);
	if ((void *)(vb->base) == NULL)	{
		vpu_loge("dynamic Physical memory allocation error size=%d\n", vb->size);
		return -1;
	}
#endif


	return 0;
}

static void vpu_free_dma_buffer(vpudrv_buffer_t *vb)
{
	if (!vb)
		return;

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY

#if (MMP_OS == MMP_OS_WIN32)
    //if (vb->base) {
    //    free(vb->phys_addr);
    //}
#else
	if (vb->base)
		vmem_free(&s_vmem, vb->phys_addr, 0);
#endif

#else
	if (vb->base)
		dma_free_coherent(0, PAGE_ALIGN(vb->size), (void *)vb->base, vb->phys_addr);
#endif

}



extern void  vdi_ffmpeg_init();
extern void  vdi_ffmpeg_deinit();

int vpu_open(char* drvname, int flag) {

    void* ptr_fd;
    struct vpu_driver *p_vpu_driver;
    int fd = -1;

    ptr_fd = malloc(sizeof(struct vpu_driver));
    if(ptr_fd != NULL) {
        fd = ptr_fd;
        memset(ptr_fd, 0x00, sizeof(struct vpu_driver));

        p_vpu_driver = (struct vpu_driver *)fd;

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY

        p_vpu_driver->video_memory_alloc_offset = VPU_BIT_REG_SIZE;
        
	    p_vpu_driver->video_memory.size = VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE;
	    p_vpu_driver->video_memory.phys_addr = VPU_DRAM_PHYSICAL_BASE;
	    p_vpu_driver->video_memory.base = (unsigned long)malloc(PAGE_ALIGN(p_vpu_driver->video_memory.size)); //ioremap(s_video_memory.phys_addr, PAGE_ALIGN(s_video_memory.size));
        memset(p_vpu_driver->video_memory.base, 0x00, p_vpu_driver->video_memory.size);
        /*
	    if (!s_video_memory.base) {
		    printk(KERN_ERR "[VPUDRV] :  fail to remap video memory physical phys_addr=0x%x, base=0x%x, size=%d\n", (int)s_video_memory.phys_addr, (int)s_video_memory.base, (int)s_video_memory.size);
		    goto ERROR_PROVE_DEVICE;
	    }

	    if (vmem_init(&s_vmem, s_video_memory.phys_addr, s_video_memory.size) < 0) {
		    printk(KERN_ERR "[VPUDRV] :  fail to init vmem system\n");
		    goto ERROR_PROVE_DEVICE;
	    }
	    vpu_logi("[VPUDRV] success to probe vpu device with reserved video memory phys_addr=0x%x, base = 0x%x\n", (int) s_video_memory.phys_addr, (int)s_video_memory.base);
        */
#else
	vpu_logi("[VPUDRV] success to probe vpu device with non reserved video memory\n");
#endif

        vdi_ffmpeg_init();

    }

    return fd;
}

int vpu_close(int fd) {

    struct vpu_driver *p_vpu_driver;

    p_vpu_driver = (struct vpu_driver *)fd;
    free(p_vpu_driver);

    vdi_ffmpeg_deinit();

    return 0;
}

int vpu_ioctl(int fd, int cmd, void* arg) {

    int ret = 0;
    struct vpu_driver *p_vpu_driver;
    vpudrv_buffer_t *p_vdb;

    p_vpu_driver = (struct vpu_driver *)fd;
    switch(cmd) {
    
        case VDI_IOCTL_GET_INSTANCE_POOL:
		    {
			    //down(&s_vpu_sem);
			    if(p_vpu_driver->instance_pool.base != 0) {
				    ret = copy_to_user((void __user *)arg, &p_vpu_driver->instance_pool, sizeof(vpudrv_buffer_t));
                    if (ret != 0) {
					    ret = -EFAULT;
                    }
			    } else {
				    ret = copy_from_user(&p_vpu_driver->instance_pool, (vpudrv_buffer_t *)arg, sizeof(vpudrv_buffer_t));
				    if (ret == 0) {
                        if (vpu_alloc_dma_buffer(p_vpu_driver, &p_vpu_driver->instance_pool) != -1) {
						    memset((void *)p_vpu_driver->instance_pool.base, 0x0, p_vpu_driver->instance_pool.size); /*clearing memory*/
						    ret = copy_to_user((void __user *)arg, &p_vpu_driver->instance_pool, sizeof(vpudrv_buffer_t));
					        if (ret == 0) {
							    /* success to get memory for instance pool */
							    //up(&s_vpu_sem);
							    break;
						    }
					    }

				    }
				    //ret = -EFAULT;
                    ret = -1;
			    }

			    //up(&s_vpu_sem);
		    }
		    break;

        case VDI_IOCTL_GET_COMMON_MEMORY:
            {
			    if(p_vpu_driver->common_memory.base != 0) {
				    ret = copy_to_user((void __user *)arg, &p_vpu_driver->common_memory, sizeof(vpudrv_buffer_t));
				    if (ret != 0)
					    ret = -EFAULT;
			    } 
                else {
				    ret = copy_from_user(&p_vpu_driver->common_memory, (vpudrv_buffer_t *)arg, sizeof(vpudrv_buffer_t));
				    if (ret == 0) {
					    if (vpu_alloc_dma_buffer(p_vpu_driver, &p_vpu_driver->common_memory) != -1) {
						    ret = copy_to_user((void __user *)arg, &p_vpu_driver->common_memory, sizeof(vpudrv_buffer_t));
					        if (ret == 0) {
							    /* success to get memory for common memory */
							    break;
						    }
					    }
				    }

				    ret = -EFAULT;
			    }
		    }
		    break;
        

        case VDI_IOCTL_SET_CLOCK_GATE:
            p_vpu_driver->clock_enable = *((int*)arg);
            break;

        case VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY:

            p_vdb = (vpudrv_buffer_t*)arg;
            vpu_alloc_dma_buffer(p_vpu_driver, p_vdb);

            break;
    }
    
    return ret;
}

void *vpu_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset) {

    struct vpu_driver *p_vpu_driver;

    p_vpu_driver = (struct vpu_driver *)fd;

    //return p_vpu_driver->instance_pool.base+offset;
    return p_vpu_driver->video_memory.base + offset;//(>instance_pool.base+offset;
}