
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/cdev.h> 
#include <linux/slab.h>
#include <linux/sched.h>
#include "../../../jpuapi/jpuconfig.h"

#define LOG_TAG "CNM_JPU_DRV"
#define JPU_CONFIG_ION_RESERVED_MEMORY 
#include "jpu.h"


// definitions to be changed as customer  configuration
//#define JPU_SUPPORT_CLOCK_CONTROL				/if you want to have clock gating scheme frame by frame

#define JPU_SUPPORT_ISR

#ifdef JPU_CONFIG_ION_RESERVED_MEMORY
#define JPU_SUPPORT_PLATFORM_DRIVER_REGISTER // working /arm/march-gdm/board-odyssues_fpga.c
#else
#define JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
#define JPU_SUPPORT_RESERVED_VIDEO_MEMORY
#endif

//#define JPU_IRQ_CONTROL
#define JPU_PLATFORM_DEVICE_NAME "gdm-mjpu"
#define JPU_CLK_NAME "jpege"
#define JPU_DEV_NAME "jpu"

#define ANA_JPU_BASE	0xED300000 // hw interim version update
#define JPU_BASE_ADDR ANA_JPU_BASE
#define JPU_REG_SIZE (0x4000*4)


#ifdef JPU_SUPPORT_ISR
#define JPU_IRQ_NUM (70+32) // hw v1.2 interrupt 70		
#endif

#ifndef VM_RESERVED	// for kernel up to 3.7.0 version
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

typedef struct jpu_drv_context_t {
	struct fasync_struct *async_queue;	
} jpu_drv_context_t;


/* To track the allocated memory buffer */
typedef struct jpudrv_buffer_pool_t {
	struct list_head list;
	struct jpudrv_buffer_t jb;
	struct file *filp;
} jpudrv_buffer_pool_t;

/* To track the instance index and buffer in instance pool */
typedef struct jpudrv_instanace_list_t {
	struct list_head list;
	unsigned long inst_idx;
	struct file *filp;
} jpudrv_instanace_list_t;


typedef struct jpudrv_instance_pool_t {
	unsigned char jpgInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
} jpudrv_instance_pool_t;

#if defined JPU_SUPPORT_RESERVED_VIDEO_MEMORY
#define JPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (16*1024*1024)
#define JPU_DRAM_PHYSICAL_BASE 0xdc000000 // kernel 192Mbase ( 128M + reserved region+64)
#include "jmm.h"
static jpu_mm_t s_jmem;	
static jpudrv_buffer_t s_video_memory = {0};
#elif defined JPU_CONFIG_ION_RESERVED_MEMORY
extern struct ion_device *ion_gdm;
struct gdm_codec_drv_data *gcodec_res;
#endif /*JPU_SUPPORT_RESERVED_VIDEO_MEMORY*/


static int jpu_hw_reset(void);
static void jpu_clk_disable(struct clk *clk);
static int jpu_clk_enable(struct clk *clk);
static struct clk *jpu_clk_get(struct device *dev);
static void jpu_clk_put(struct clk *clk);
// end customer definition

static int jpu_probe(struct platform_device *pdev);
static int jpu_remove(struct platform_device *pdev);
#ifdef CONFIG_PM
static int jpu_suspend(struct platform_device *pdev, pm_message_t state);
static int jpu_resume(struct platform_device *pdev);
#else
#define	jpu_suspend	NULL
#define	jpu_resume	NULL
#endif	

static struct platform_driver jpu_driver = {
	.driver = {
		   .name = JPU_PLATFORM_DEVICE_NAME,
		   },
	.probe = jpu_probe,
	.remove = jpu_remove,
	.suspend = jpu_suspend,
	.resume = jpu_resume,
};

static jpudrv_buffer_t s_jpu_instance_pool = {0};
static jpu_drv_context_t s_jpu_drv_context;
static int s_jpu_major;
static struct cdev s_jpu_cdev; 
static int s_jpu_open_count;
static struct clk *s_jpu_clk;
#ifdef JPU_SUPPORT_ISR
static int s_jpu_irq = JPU_IRQ_NUM;
static int s_interrupt_flag;
#endif
static u32 s_jpu_reg_phy_base = JPU_BASE_ADDR;
static void __iomem *s_jpu_reg_virt_base;


static wait_queue_head_t s_interrupt_wait_q;


static spinlock_t s_jpu_lock = __SPIN_LOCK_UNLOCKED(s_jpu_lock);
static DEFINE_SEMAPHORE(s_jpu_sem);
static struct list_head s_jbp_head = LIST_HEAD_INIT(s_jbp_head);
static struct list_head s_jpu_inst_list_head = LIST_HEAD_INIT(s_jpu_inst_list_head);

/* implement to power management functions */
#define	ReadJpuRegister(addr)			*(volatile unsigned int *)(s_jpu_reg_virt_base + addr)
#define	WriteJpuRegister(addr, val)		*(volatile unsigned int *)(s_jpu_reg_virt_base + addr) = (unsigned int)val;	


#if defined JPU_CONFIG_ION_RESERVED_MEMORY
int jpu_res_init(struct gdm_codec_drv_data *gcdrv)
{
	gcdrv->iclient = ion_client_create(ion_gdm, "gdm-jpeg");
	if(IS_ERR_OR_NULL(gcdrv->iclient)) {
		jpu_loge("ion_client_create() return error, val %p\n", gcdrv->iclient);
		gcdrv->iclient = NULL;
	}
	return 0;
}
size_t jpu_utils_map_ion_handle(struct gdm_codec_drv_data *gcdrv,
		struct jb_dma_buf_data *dma, struct ion_handle *ion_handle,
		struct dma_buf *buf)
{
	dma->fence = NULL;
	dma->dma_buf = buf;

	dma->attachment = dma_buf_attach(dma->dma_buf, &gcdrv->pdev->dev);
	if (IS_ERR_OR_NULL(dma->attachment)) {
		//dev_err(gfd->dev, "dma_buf_attach() failed: %ld\n",
		//		PTR_ERR(dma->attachment));
		jpu_loge("dma_buf_attach() failed: %ld\n",PTR_ERR(dma->attachment)); 
		goto err_buf_map_attach;
	}

	if (ion_phys(gcdrv->iclient, ion_handle, (ion_phys_addr_t *)&dma->dma_addr, &dma->dma_buf->size)) 
	{
  	    jpu_loge("Failed to get phys. addr of buf = %lx \n", PTR_ERR(dma->attachment)); 
		goto err_buf_map_attachment;
	}
	dma->ion_handle = ion_handle;
	return dma->dma_buf->size;

err_buf_map_attachment:
	dma_buf_detach(dma->dma_buf, dma->attachment);
err_buf_map_attach:
	return 0;
}

void jpu_utils_free_dma_buf(struct gdm_codec_drv_data *gcdrv, struct ion_handle *ion_handle,
		struct jb_dma_buf_data *dma)
{
	if (!dma->dma_addr)
		return;

	if (dma->fence)
		sync_fence_put(dma->fence);

	dma_buf_detach(dma->dma_buf, dma->attachment);
	dma_buf_put(dma->dma_buf);
	ion_free(gcdrv->iclient, ion_handle);
	memset(dma, 0, sizeof(struct jb_dma_buf_data));
}
#endif

static int jpu_alloc_dma_buffer(jpudrv_buffer_t *jb)
{
#if defined JPU_CONFIG_ION_RESERVED_MEMORY
		struct dma_buf *buf;
		dma_addr_t map_dma;
		unsigned int ret;
		int ion_shared_fd = -1;
		struct ion_handle *buffer_ihandle;
		int ion_buf_idx = 0;
#endif	

#if defined JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	jb->phys_addr = (unsigned long long)jmem_alloc(&s_jmem, jb->size, 0);
	if (jb->phys_addr  == (unsigned long)-1)
	{
		printk(KERN_ERR "[JPUDRV] Physical memory allocation error size=%d\n", jb->size);
		return -1;
	}

	jb->base = (unsigned long)(s_video_memory.base + (jb->phys_addr - s_video_memory.phys_addr));
	
#elif defined JPU_CONFIG_ION_RESERVED_MEMORY
    
    buffer_ihandle = ion_alloc(gcodec_res->iclient,
    	(size_t)PAGE_ALIGN(jb->size), SZ_4K,
    	ION_HEAP_CARVEOUT_MASK, 0);
    
    if (IS_ERR(buffer_ihandle)) {
    	jpu_loge("failed to ion_alloc\n");
    	return -1;
    }
	
	ion_shared_fd = ion_share_dma_buf_fd(gcodec_res->iclient, buffer_ihandle);
	if(ion_shared_fd < 0)
	{
    	jpu_loge("failed to ion_shared_fd\n");
    	return -1;
	}
	jb->ion_shared_fd = ion_shared_fd;
	
	buf = dma_buf_get(ion_shared_fd);
    if (IS_ERR_OR_NULL(buf)) {
    	jpu_loge("ion_share_dma_buf() failed\n");
    	goto err_share_dma_buf;
    }		
	
	ion_buf_idx = (jb->ion_shared_fd%16);	
	
    ret = jpu_utils_map_ion_handle(gcodec_res,
    	&gcodec_res->dma_buf_data[ion_buf_idx], buffer_ihandle, buf);
    if (!ret)
    	goto err_map;
    
    map_dma = gcodec_res->dma_buf_data[ion_buf_idx].dma_addr;
    jb->phys_addr = map_dma;

	jpu_logd("jpu_alloc_dma_buffer jb->phys_addr = 0x%lx \n", jb->phys_addr);
	
    jb->base = (void*)ion_map_kernel(gcodec_res->iclient,
    	buffer_ihandle);
    if (IS_ERR_OR_NULL(jb->base)) {
    	jpu_loge("ION memory mapping failed - %ld\n", PTR_ERR(jb->base));
    	ret = PTR_ERR(jb->base);
    	goto err_map;
    }
    memset(jb->base, 0xff, PAGE_ALIGN(jb->size));
	jpu_logd("========> jpu_alloc_dma_buffer ion_buf_idx =%d phys_addr = 0x%lx virtual_addr = 0x%lx base_addr = 0x%lx ========> \n", ion_buf_idx, jb->phys_addr, jb->virt_addr, jb->base); 
#else
	jb->base = (unsigned long)dma_alloc_coherent(NULL, PAGE_ALIGN(jb->size), (dma_addr_t *) (&jb->phys_addr), GFP_DMA | GFP_KERNEL);
	if ((void *)(jb->base) == NULL) 
	{
		printk(KERN_ERR "[JPUDRV] Physical memory allocation error size=%d\n", jb->size);
		return -1;
	}
#endif
	return 0;

#if defined JPU_CONFIG_ION_RESERVED_MEMORY
err_map:
	dma_buf_put(buf);
err_share_dma_buf:
	ion_free(gcodec_res->iclient, buffer_ihandle);
	return -1;
#endif

}

static void jpu_free_dma_buffer(jpudrv_buffer_t *jb)
{
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (jb->base) 
		jmem_free(&s_jmem, jb->phys_addr, 0);
#elif defined JPU_CONFIG_ION_RESERVED_MEMORY
    if (jb->base)
    {
    	struct ion_handle *free_ion_handle;
		int ion_buf_idx = 0;
		ion_buf_idx = (jb->ion_shared_fd%16);
		
        jpu_logd("<------------ jpu_free_dma_buffer ion_buf_idx =%d phys_addr = 0x%lx virtual_addr = 0x%lx base_addr = 0x%lx <------------ \n", ion_buf_idx, jb->phys_addr, jb->virt_addr, jb->base); 
		free_ion_handle = gcodec_res->dma_buf_data[ion_buf_idx].ion_handle;
    	jpu_utils_free_dma_buf(gcodec_res, free_ion_handle, &gcodec_res->dma_buf_data[ion_buf_idx]);		
    }
#else
	if (jb->base) 
	{
		dma_free_coherent(0, PAGE_ALIGN(jb->size), (void *)jb->base, jb->phys_addr);
	}
#endif
}

static int jpu_free_instances(struct file *filp) 
{
	u32 inst_idx;
	jpudrv_instanace_list_t *jil, *n;
	jpudrv_instance_pool_t *jip;
	void *jip_base;
	void* jdi_mutex_base;
	const int PTHREAD_MUTEX_T_DESTROY_VALUE = 0xdead10cc;

	list_for_each_entry_safe(jil, n, &s_jpu_inst_list_head, list) 
	{
		if (jil->filp == filp) {
			inst_idx = jil->inst_idx;
			jip_base = (void *)(s_jpu_instance_pool.base);
			jip = (jpudrv_instance_pool_t *)jip_base;
			if (jip) {
				memset(&jip->jpgInstPool[inst_idx], 0x00, 4);	// only first 4 byte is key point to free the corresponding instance.				
#define PTHREAD_MUTEX_T_HANDLE_SIZE 4
				jdi_mutex_base = (jip_base + (s_jpu_instance_pool.size - PTHREAD_MUTEX_T_HANDLE_SIZE));
				if (jdi_mutex_base) {
					memcpy(jdi_mutex_base, &PTHREAD_MUTEX_T_DESTROY_VALUE, PTHREAD_MUTEX_T_HANDLE_SIZE); 				
				}
			}
			s_jpu_open_count--;
			list_del(&jil->list);
			kfree(jil);		
		}
	}

	return s_jpu_open_count;
}

static int jpu_free_buffers(struct file *filp)
{
	jpudrv_buffer_pool_t *pool, *n;
	jpudrv_buffer_t jb;

	list_for_each_entry_safe(pool, n, &s_jbp_head, list) 
	{
		if (pool->filp == filp)
		{
			jb = pool->jb;
			if (jb.base) 
			{
				printk(KERN_INFO "jb.base dma free 0x%lx \n \n",jb.base);	
				jpu_free_dma_buffer(&jb);
				list_del(&pool->list);
				kfree(pool);
			}
		}
		
	}

	return 0;
}


#ifdef JPU_SUPPORT_ISR
static irqreturn_t jpu_irq_handler(int irq, void *dev_id)
{
	jpu_drv_context_t *dev = (jpu_drv_context_t *)dev_id;
#ifdef JPU_IRQ_CONTROL
	disable_irq_nosync(s_jpu_irq);
#endif
	
	if (dev->async_queue)
		kill_fasync(&dev->async_queue, SIGIO, POLL_IN);	// notify the interrupt to userspace
	
	
	s_interrupt_flag = 1;
	
	//printk(KERN_INFO "jpu_irq_handler\n");	
	wake_up_interruptible(&s_interrupt_wait_q);
	
	return IRQ_HANDLED;
}
#endif

static int jpu_open(struct inode *inode, struct file *filp)
{
	spin_lock(&s_jpu_lock);


	filp->private_data = (void *)(&s_jpu_drv_context);
	spin_unlock(&s_jpu_lock);
	return 0;
}

static long jpu_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	int ret = 0; 	

	switch (cmd) 
	{
	case JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY:
		{
			jpudrv_buffer_pool_t *jbp;

			down(&s_jpu_sem);

			jbp = kzalloc(sizeof(*jbp), GFP_KERNEL);
			if (!jbp)
			{
				up(&s_jpu_sem);
				return -ENOMEM;
			}

			ret = copy_from_user(&(jbp->jb), (jpudrv_buffer_t *)arg, sizeof(jpudrv_buffer_t));
			if (ret) 
			{
				kfree(jbp);
				up(&s_jpu_sem);
				return -EFAULT;
			}

			ret = jpu_alloc_dma_buffer(&(jbp->jb));
			if (ret == -1) 
			{
				ret = -ENOMEM;
				kfree(jbp);
				up(&s_jpu_sem);
				break;
			}

			ret = copy_to_user((void __user *)arg, &(jbp->jb), sizeof(jpudrv_buffer_t));
			if (ret) 
			{
				kfree(jbp);
				ret = -EFAULT;
				up(&s_jpu_sem);
				break;
			}

			jbp->filp = filp;
			spin_lock(&s_jpu_lock);
			list_add(&jbp->list, &s_jbp_head);
			spin_unlock(&s_jpu_lock);		

			up(&s_jpu_sem);

		}
		break;
	case JDI_IOCTL_FREE_PHYSICALMEMORY:
		{

			jpudrv_buffer_pool_t *jbp, *n;
			jpudrv_buffer_t jb;

			down(&s_jpu_sem);

			ret = copy_from_user(&jb, (jpudrv_buffer_t *)arg, sizeof(jpudrv_buffer_t));
			if (ret)
			{
				up(&s_jpu_sem);
				return -EACCES;
			}

			if (jb.base) 
				jpu_free_dma_buffer(&jb);

			spin_lock(&s_jpu_lock);
			list_for_each_entry_safe(jbp, n, &s_jbp_head, list) 
			{
				if (jbp->jb.base == jb.base) 
				{
					list_del(&jbp->list);
					kfree(jbp);
					break;
				}
			}
			spin_unlock(&s_jpu_lock);

			up(&s_jpu_sem);

		}
		break;

	case JDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO:
		{
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
			if (s_video_memory.base != 0) 
			{
				ret = copy_to_user((void __user *)arg, &s_video_memory, sizeof(jpudrv_buffer_t));
				if (ret != 0) 
					ret = -EFAULT;
			} 
			else
			{
				ret = -EFAULT;
			}			
#endif
		}
		break;

#ifdef JPU_SUPPORT_ISR
	case JDI_IOCTL_WAIT_INTERRUPT:
		{
			u32 timeout;
			timeout = (u32) arg;
			if (!wait_event_interruptible_timeout(s_interrupt_wait_q, s_interrupt_flag != 0, msecs_to_jiffies(timeout))) 
			{
				ret = -ETIME;
				break;
			} 
			
			if (signal_pending(current)) 
			{
				ret = -ERESTARTSYS;
				break;
			} 
			
			s_interrupt_flag = 0;	
#ifdef JPU_IRQ_CONTROL
			enable_irq(s_jpu_irq);
#endif
		}
		break;
#endif	//JPU_SUPPORT_ISR
	case JDI_IOCTL_SET_CLOCK_GATE:
		{
			u32 clkgate;
			
			if (get_user(clkgate, (u32 __user *) arg))
				return -EFAULT;
			
#ifdef JPU_SUPPORT_CLOCK_CONTROL	
			if (clkgate)
				jpu_clk_enable(s_jpu_clk);
			else
				jpu_clk_disable(s_jpu_clk);
#endif
			
		}
		break;
	case JDI_IOCTL_GET_INSTANCE_POOL:
		{
			down(&s_jpu_sem);
			
			if (s_jpu_instance_pool.base != 0) 
			{
				ret = copy_to_user((void __user *)arg, &s_jpu_instance_pool, sizeof(jpudrv_buffer_t));
				if (ret != 0) 
					ret = -EFAULT;
			} 
			else
			{
				ret = copy_from_user(&s_jpu_instance_pool, (jpudrv_buffer_t *)arg, sizeof(jpudrv_buffer_t));
				if (ret == 0)
				{
					if (jpu_alloc_dma_buffer(&s_jpu_instance_pool) != -1)
					{
						memset((void *)s_jpu_instance_pool.base, 0x0, s_jpu_instance_pool.size); 
						ret = copy_to_user((void __user *)arg, &s_jpu_instance_pool, sizeof(jpudrv_buffer_t));
						if (ret == 0)
						{
							// success to get memory for instance pool
							up(&s_jpu_sem);
							break;
						}
					}
				}
				
				ret = -EFAULT;
			}
			up(&s_jpu_sem);
		}
		break;
	case JDI_IOCTL_OPEN_INSTANCE:
		{
			u32 inst_idx;
			jpudrv_instanace_list_t *jil;

			jil = kzalloc(sizeof(*jil), GFP_KERNEL);
			if (!jil)
				return -ENOMEM;

			if (get_user(inst_idx, (u32 __user *) arg))
				return -EFAULT;

			jil->inst_idx = inst_idx;
			jil->filp = filp;
			spin_lock(&s_jpu_lock);
			list_add(&jil->list, &s_jpu_inst_list_head);
			s_jpu_open_count++;
			spin_unlock(&s_jpu_lock);
            printk(KERN_INFO "[JPUDRV] JDI_IOCTL_OPEN_INSTANCE inst_idx=%d, open_count=%d\n", (int)inst_idx, s_jpu_open_count);  
		}
		break;
	case JDI_IOCTL_CLOSE_INSTANCE:
		{
			u32 inst_idx;
			jpudrv_instanace_list_t *jil, *n;

			if (get_user(inst_idx, (u32 __user *) arg))
				return -EFAULT;

			spin_lock(&s_jpu_lock);

			list_for_each_entry_safe(jil, n, &s_jpu_inst_list_head, list) 
			{
				if (jil->inst_idx == inst_idx)
				{
					s_jpu_open_count--;
					list_del(&jil->list);
					kfree(jil);
					printk(KERN_INFO "[JPUDRV] JDI_IOCTL_CLOSE_INSTANCE inst_idx=%d, open_count=%d\n", (int)inst_idx, s_jpu_open_count);
					break;
				}
			}
			spin_unlock(&s_jpu_lock);
		}
		break;
	case JDI_IOCTL_GET_INSTANCE_NUM:
		{
			down(&s_jpu_sem);

			ret = copy_to_user((void __user *)arg, &s_jpu_open_count, sizeof(int));
			if (ret != 0) 
				ret = -EFAULT;

			//printk(KERN_INFO "[JPUDRV] VDI_IOCTL_GET_INSTANCE_NUM open_count=%d\n", s_vpu_open_count);

			up(&s_jpu_sem);
		}
		break;
	case JDI_IOCTL_RESET:
		{
			jpu_hw_reset();
		}
		break;
	default:
		{
			printk(KERN_ERR "No such IOCTL, cmd is %d\n", cmd);
		}
		break;
	}
	return ret;
}

static ssize_t jpu_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
	return -1;
}

static ssize_t jpu_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
	
	return -1;
}

static int jpu_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "jpu_release s_jpu_open_count=%d\n", s_jpu_open_count);	
    jpu_logi("[VPUDRV] vpu_release\n");
	
	spin_lock(&s_jpu_lock);


	if (s_jpu_open_count > 0) 
	{
		/* found and free the not handled buffer by user applications */
		jpu_free_buffers(filp); 	

		/* found and free the not closed instance by user applications */
		jpu_free_instances(filp);
	}

	if (s_jpu_instance_pool.base)
	{
		jpu_free_dma_buffer(&s_jpu_instance_pool);
		s_jpu_instance_pool.base = 0;
		printk(KERN_INFO "[JPUDRV] jpu_release - free s_jpu_instance_pool\n");	
	}

	spin_unlock(&s_jpu_lock);

	return 0;
}


static int jpu_fasync(int fd, struct file *filp, int mode)
{
	struct jpu_drv_context_t *dev = (struct jpu_drv_context_t *)filp->private_data;
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}


static int jpu_map_to_register(struct file *fp, struct vm_area_struct *vm)
{
	unsigned long pfn;

	vm->vm_flags |= VM_IO | VM_RESERVED;
	vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
	pfn = s_jpu_reg_phy_base >> PAGE_SHIFT;
	
	return remap_pfn_range(vm, vm->vm_start, pfn, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int jpu_map_to_physical_memory(struct file *fp, struct vm_area_struct *vm)
{
	vm->vm_flags |= VM_IO | VM_RESERVED;
	vm->vm_page_prot = pgprot_writecombine(vm->vm_page_prot);

	return remap_pfn_range(vm, vm->vm_start, vm->vm_pgoff, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int jpu_map_to_instance_pool_memory(struct file *fp, struct vm_area_struct *vm)
{
	return remap_pfn_range(vm, vm->vm_start, vm->vm_pgoff, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

/*!
 * @brief memory map interface for jpu file operation
 * @return  0 on success or negative error code on error
 */
static int jpu_mmap(struct file *fp, struct vm_area_struct *vm)
{
	if (vm->vm_pgoff) 
	{
		if(vm->vm_pgoff == (s_jpu_instance_pool.phys_addr>>PAGE_SHIFT))
			return  jpu_map_to_instance_pool_memory(fp, vm);

		return jpu_map_to_physical_memory(fp, vm);
	}
	else
		return jpu_map_to_register(fp, vm);
	
}

struct file_operations jpu_fops = {
	.owner = THIS_MODULE,
	.open = jpu_open,
	.read = jpu_read,
	.write = jpu_write,
	.unlocked_ioctl = jpu_ioctl,
	.release = jpu_release,
	.fasync = jpu_fasync,
	.mmap = jpu_mmap,
};

static int jpu_probe(struct platform_device *pdev)
{
	int err = 0;

	struct resource *res = NULL;

	#if defined JPU_CONFIG_ION_RESERVED_MEMORY	
	struct gdm_codec_drv_data *gdata = NULL;
	if (gcodec_res) {
		jpu_loge("GDM codec already initialized\n");
		return -EINVAL;
	}	
	#endif
	
	printk(KERN_INFO "jpu_probe\n");	

	if (pdev)
	{
		#if defined JPU_CONFIG_ION_RESERVED_MEMORY	
		gdata = devm_kzalloc(&pdev->dev, sizeof(*gdata), GFP_KERNEL);
		if (gdata == NULL)
			return -ENOMEM;
        pdev->id = 0;
        gdata->pdev = pdev;
        platform_set_drvdata(pdev, gdata);
        gcodec_res = gdata;		
		#endif
		
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);		
	}

	if (res) 	// if platform driver is implemented
	{
		s_jpu_reg_phy_base = res->start;
		s_jpu_reg_virt_base = ioremap(res->start, res->end - res->start);
		printk(KERN_INFO "[JPUDRV] : jpu base address get from platform driver base addr=0x%u, virtualbase=0x%p\n", s_jpu_reg_phy_base , s_jpu_reg_virt_base);		
	}
	else
	{
		s_jpu_reg_virt_base = ioremap(s_jpu_reg_phy_base, JPU_REG_SIZE);
		printk(KERN_INFO "[JPUDRV] : jpu base address get from defined value base addr=0x%u, virtualbase=0x%p\n", s_jpu_reg_phy_base, s_jpu_reg_virt_base);		
	}

	/* get the major number of the character device */  
	if ((alloc_chrdev_region(&s_jpu_major, 0, 1, JPU_DEV_NAME)) < 0) 
	{  
		err = -EBUSY;
		printk(KERN_ERR "could not allocate major number\n");  		
		goto ERROR_PROVE_DEVICE;
	}

	/* initialize the device structure and register the device with the kernel */  
	cdev_init(&s_jpu_cdev, &jpu_fops);  
	if ((cdev_add(&s_jpu_cdev, s_jpu_major, 1)) < 0) 
	{  
		err = -EBUSY;
		printk(KERN_ERR "could not allocate chrdev\n");  
		goto ERROR_PROVE_DEVICE;  
	}  

	if (pdev)
		s_jpu_clk = jpu_clk_get(&pdev->dev);
	else
		s_jpu_clk = jpu_clk_get(NULL);
	
	if (!s_jpu_clk) {
		printk(KERN_ERR "[JPUDRV] : fail to get clock controller, but, do not treat as error, \n");			
	}
	else {
		printk(KERN_INFO "[JPUDRV] : get clock controller s_jpu_clk=0x%p\n", s_jpu_clk);					
	}
	
#ifdef JPU_SUPPORT_CLOCK_CONTROL	
#else
	jpu_clk_enable(s_jpu_clk);
#endif

	
#ifdef JPU_SUPPORT_ISR	
	if(pdev)
		res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res) 	// if platform driver is implemented
	{
		s_jpu_irq = res->start;
		printk(KERN_INFO "[JPUDRV] : jpu irq number get from platform driver irq=0x%x\n", s_jpu_irq );		
	}
	else
	{
		printk(KERN_INFO "[JPUDRV] : jpu irq number get from defined value irq=0x%x\n", s_jpu_irq );
	}


	err = request_irq(s_jpu_irq, jpu_irq_handler, IRQF_TRIGGER_RISING, "JPU_CODEC_IRQ", (void *)(&s_jpu_drv_context));
	if (err)
	{
		printk(KERN_ERR "[JPUDRV] :  fail to register interrupt handler\n");
		goto ERROR_PROVE_DEVICE;
	}
#endif

#if defined JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	s_video_memory.size = JPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE;
	s_video_memory.phys_addr = JPU_DRAM_PHYSICAL_BASE;
	s_video_memory.base = (unsigned long)ioremap(s_video_memory.phys_addr, PAGE_ALIGN(s_video_memory.size));
	if (!s_video_memory.base)
	{	
		printk(KERN_ERR "[JPUDRV] :  fail to remap video memory physical phys_addr=0x%x, base=0x%x, size=%d\n", (int)s_video_memory.phys_addr, (int)s_video_memory.base, (int)s_video_memory.size);
		goto ERROR_PROVE_DEVICE;
	}
	if (jmem_init(&s_jmem, s_video_memory.phys_addr, s_video_memory.size) < 0)
	{
		printk(KERN_ERR "[JPUDRV] :  fail to init vmem system\n");
		goto ERROR_PROVE_DEVICE;
	}
	printk(KERN_INFO "[JPUDRV] success to probe jpu device with reserved video memory phys_addr=0x%x, base = 0x%x\n",(int) s_video_memory.phys_addr, (int)s_video_memory.base);
#elif defined JPU_CONFIG_ION_RESERVED_MEMORY
	jpu_res_init(gcodec_res);
#else
	printk(KERN_INFO "[JPUDRV] success to probe jpu device with non reserved video memory\n");
#endif	
				


	return 0;


ERROR_PROVE_DEVICE:
	
	if (s_jpu_major)
		unregister_chrdev_region(s_jpu_major, 1);		

	if (s_jpu_reg_virt_base)
		iounmap(s_jpu_reg_virt_base);

	return err;
}

static int jpu_remove(struct platform_device *pdev)
{
#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER	

	printk(KERN_INFO "jpu_remove\n");

	if (s_jpu_instance_pool.base)
	{
		jpu_free_dma_buffer(&s_jpu_instance_pool);
		s_jpu_instance_pool.base = 0;
	}
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (s_video_memory.base) 
	{
		iounmap((void *)s_video_memory.base);
		s_video_memory.base = 0;
		jmem_exit(&s_jmem);		
	}	
#endif	
	
	cdev_del(&s_jpu_cdev);  
	if (s_jpu_major > 0) 
	{
		unregister_chrdev_region(s_jpu_major, 1);
		s_jpu_major = 0;
	}

#ifdef JPU_SUPPORT_ISR
	if (s_jpu_irq)
		free_irq(s_jpu_irq, &s_jpu_drv_context);		
#endif

	if (s_jpu_reg_virt_base)
		iounmap(s_jpu_reg_virt_base);

	jpu_clk_put(s_jpu_clk);	
		
#endif //JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
	return 0;
}

#ifdef CONFIG_PM
static int jpu_suspend(struct platform_device *pdev, pm_message_t state)
{
	jpu_clk_disable(s_jpu_clk);
	return 0;

}
static int jpu_resume(struct platform_device *pdev)

{
	jpu_clk_enable(s_jpu_clk);

	return 0;
}
#else
//#define	jpu_suspend	NULL
//#define	jpu_resume	NULL
#endif				/* !CONFIG_PM */




static int __init jpu_init(void)
{
	int res;
	res = 0;
	printk(KERN_INFO "jpu_init\n");	
	init_waitqueue_head(&s_interrupt_wait_q);
	s_jpu_instance_pool.base = 0;	

#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
	res = platform_driver_register(&jpu_driver);
#else
	//res = platform_driver_register(&jpu_driver);
	res = jpu_probe(NULL);
#endif

	printk(KERN_INFO "end jpu_init result=0x%x\n", res);	
	return res;
}

static void __exit jpu_exit(void)
{
#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER	
	printk(KERN_INFO "jpu_exit\n");
    platform_driver_unregister(&jpu_driver);
#else	
	if (s_jpu_instance_pool.base)
	{
		jpu_free_dma_buffer(&s_jpu_instance_pool);
		s_jpu_instance_pool.base = 0;
	}
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (s_video_memory.base) 
	{
		iounmap((void *)s_video_memory.base);
		s_video_memory.base = 0;
	}
#endif		

	cdev_del(&s_jpu_cdev);  
	if (s_jpu_major > 0) 
	{
		unregister_chrdev_region(s_jpu_major, 1);
		s_jpu_major = 0;
	}

#ifdef JPU_SUPPORT_ISR
	if (s_jpu_irq)
		free_irq(s_jpu_irq, &s_jpu_drv_context);		
#endif	
	
	jpu_clk_put(s_jpu_clk);

	
	if (s_jpu_reg_virt_base) {
		iounmap(s_jpu_reg_virt_base);
		s_jpu_reg_virt_base = (void *)0x00;
	}

#endif //JPU_SUPPORT_PLATFORM_DRIVER_REGISTER


#ifdef JPU_SUPPORT_CLOCK_CONTROL
#else
    jpu_clk_disable(s_jpu_clk);
#endif

	
	return;
}

MODULE_AUTHOR("A customer using C&M JPU, Inc.");
MODULE_DESCRIPTION("JPU linux driver");
MODULE_LICENSE("GPL");

module_init(jpu_init);
module_exit(jpu_exit);

int jpu_hw_reset(void)
{
	printk(KERN_INFO "request jpu reset from application. \n");
	return 0;
}


struct clk *jpu_clk_get(struct device *dev)
{
	return clk_get(dev, JPU_CLK_NAME);

}
void jpu_clk_put(struct clk *clk)
{
	if (!(clk == NULL || IS_ERR(clk)))
		clk_put(clk);
}
int jpu_clk_enable(struct clk *clk)
{
	if (!(clk == NULL || IS_ERR(clk))) {
		printk(KERN_INFO "jpu_clk_enable\n");	
		return clk_enable(clk);
	}
	return 0;

}

void jpu_clk_disable(struct clk *clk)
{
	if (!(clk == NULL || IS_ERR(clk))) {
		printk(KERN_INFO "jpu_clk_disable\n");	
		clk_disable(clk);
	}
}


