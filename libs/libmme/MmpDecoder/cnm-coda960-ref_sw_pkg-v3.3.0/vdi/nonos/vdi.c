//------------------------------------------------------------------------------
// File: vdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

 #if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
 #elif defined(linux) || defined(__linux) || defined(ANDROID)
 #else


#include <stdio.h>
#include <stdlib.h>
#include "../vdi.h"
#include "../vdi_osal.h"
#include "../mm.h"

#if 0
#if (REQUIRED_VPU_MEMORY_SIZE > VPUDRV_INIT_VIDEO_MEMORY_SIZE_IN_BYTE)
#error "Warnning : VPU memory will be overflow"
#endif
#endif

#define VPU_BIT_REG_SIZE		(0x4000*MAX_NUM_VPU_CORE)
#define VPU_BIT_REG_BASE		0x10000000

#define VDI_SRAM_BASE_ADDR		0x00
#	define VDI_SRAM_SIZE			0x20000 	// FHD MAX size, 0x17D00  4K MAX size 0x34600

#define VPU_DRAM_PHYSICAL_BASE	0x00
#define VPU_DRAM_SIZE			(128*1024*1024)

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif

#define VDI_SYSTEM_ENDIAN VDI_LITTLE_ENDIAN

typedef struct vpu_buffer_t vpudrv_buffer_t;	

typedef struct vpu_buffer_pool_t
{
	vpudrv_buffer_t vdb;
	int inuse;
} vpu_buffer_pool_t;

typedef struct  {
	unsigned long coreIdx;
	int vpu_fd;	
	vpu_instance_pool_t *pvip;
	int task_num;
	int clock_state;
	vpudrv_buffer_t vdb_video_memory;
	vpudrv_buffer_t vdb_register;
	vpu_buffer_t vpu_common_memory;
	vpu_buffer_pool_t vpu_buffer_pool[MAX_VPU_BUFFER_POOL];
	int vpu_buffer_pool_count;
} vdi_info_t;

static vdi_info_t s_vdi_info[MAX_VPU_CORE_NUM];
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
static vpu_instance_pool_t s_vip[MAX_VPU_CORE_NUM];	// it can be used for a buffer space to save context for app process. to support running VPU in multiple process. this space should be a shared buffer.
#else
static vpu_instance_pool_t s_vip;	// it can be used for a buffer space to save context for app process. to support running VPU in multiple process. this space should be a shared buffer.
#endif
static int swap_endian(unsigned char *data, int len, int endian);
static int allocate_common_memory(unsigned long coreIdx);



int vdi_probe(unsigned long coreIdx)
{
	int ret;

	ret = vdi_init(coreIdx);
	vdi_release(coreIdx);

	return ret;
}

int vdi_init(unsigned long coreIdx)
{
	int ret;
	vdi_info_t *vdi;
	int i;

	if (coreIdx > MAX_VPU_CORE_NUM)
		return 0;

	vdi = &s_vdi_info[coreIdx];

	if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00)
	{
		vdi->task_num++;		
		return 0;
	}

	vdi->vpu_fd = 1;
	osal_memset(&vdi->vpu_buffer_pool, 0x00, sizeof(vpu_buffer_pool_t)*MAX_VPU_BUFFER_POOL);

	if (!vdi_get_instance_pool(coreIdx))
	{
		VLOG(ERR, "[VDI] fail to create shared info for saving context \n");
		goto ERR_VDI_INIT;
	}

	vdi->vdb_video_memory.phys_addr = VPU_DRAM_PHYSICAL_BASE;
	vdi->vdb_video_memory.size = VPU_DRAM_SIZE;

#if 0
	if (REQUIRED_VPU_MEMORY_SIZE > vdi->vdb_video_memory.size)
	{
		VLOG(ERR, "[VDI] Warning : VPU memory will be overflow\n");
	}
#endif

	if (allocate_common_memory(coreIdx) < 0) 
	{
		VLOG(ERR, "[VDI] fail to get vpu common buffer from driver\n");
		goto ERR_VDI_INIT;
	}

	if (!vdi->pvip->instance_pool_inited)
		osal_memset(&vdi->pvip->vmem, 0x00, sizeof(video_mm_t));

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	ret = vmem_init(&vdi->pvip->vmem, (unsigned long)vdi->vdb_video_memory.phys_addr + (vdi->pvip->vpu_common_buffer.size*MAX_VPU_CORE_NUM),
		vdi->vdb_video_memory.size - (vdi->pvip->vpu_common_buffer.size*MAX_VPU_CORE_NUM));		
#else
	ret = vmem_init(&vdi->pvip->vmem, (unsigned long)vdi->vdb_video_memory.phys_addr + vdi->pvip->vpu_common_buffer.size,
		vdi->vdb_video_memory.size - vdi->pvip->vpu_common_buffer.size);
#endif
       vmem_lock_init(coreIdx);

	if (ret < 0)
	{
		VLOG(ERR, "[VDI] fail to init vpu memory management logic\n");
		goto ERR_VDI_INIT;
	}


	vdi->vdb_register.phys_addr = VPU_BIT_REG_BASE;
	vdi->vdb_register.virt_addr = VPU_BIT_REG_BASE;
	vdi->vdb_register.size = VPU_BIT_REG_SIZE;

	if (vdi_read_register(coreIdx, 0x018) == 0) // if BIT processor is not running.
	{
		for (i=0; i<64; i++)
			vdi_write_register(coreIdx, (i*4) + 0x100, 0); 
	}

	if (vdi_read_register(coreIdx, DBG_CONFIG_REPORT_1) == WAVE410_CODE) // WAVE410
	{
		if (vdi_read_register(coreIdx, WAVE4_VCPU_CUR_PC) == 0) // if BIT processor is not running.
		{
			for (i=0; i<64; i++)
				vdi_write_register(coreIdx, (i*4) + 0x100, 0x0); 
		}
	}
	else // CODA9XX
	{
		if (vdi_read_register(coreIdx, BIT_CUR_PC) == 0) // if BIT processor is not running.
		{
			for (i=0; i<64; i++)
				vdi_write_register(coreIdx, (i*4) + 0x100, 0x0); 
		}
	}

	vdi_set_clock_gate(coreIdx, 1);

	vdi->coreIdx = coreIdx;
	vdi->task_num++;	

	VLOG(INFO, "[VDI] success to init driver \n");

	return 0;

ERR_VDI_INIT:
	
	vdi_release(coreIdx);
	return -1;
}

int vdi_set_bit_firmware_to_pm(unsigned long coreIdx, const unsigned short *code)
{
	return 0;
}

int vdi_release(unsigned long coreIdx)
{
	int i;
	int ret;
	vpudrv_buffer_t vdb;
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
		return 0;

	if (vdi_lock(coreIdx) < 0)
	{
		VLOG(ERR, "[VDI] fail to handle lock function\n");
		return -1;
	}

	if (vdi->task_num > 1) // means that the opened instance remains 
	{
		vdi->task_num--;
		vdi_unlock(coreIdx);
		return 0;
	}
        vmem_lock_deinit(coreIdx);
	vmem_exit(&vdi->pvip->vmem);

	osal_memset(&vdi->vdb_register, 0x00, sizeof(vpudrv_buffer_t));

	// get common memory information to free virtual address
	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_common_memory.phys_addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr &&
			vdi->vpu_common_memory.phys_addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr + vdi->vpu_buffer_pool[i].vdb.size))
		{
			vdi->vpu_buffer_pool[i].inuse = 0;
			vdi->vpu_buffer_pool_count--;
			vdb = vdi->vpu_buffer_pool[i].vdb;
			break;
		}
	}

	if (vdb.size > 0)
		osal_memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));

	vdi->task_num--;
	vdi->vpu_fd = -1;

	vdi_unlock(coreIdx);

	osal_memset(vdi, 0x00, sizeof(vdi_info_t));
	
	return 0;
}

int vdi_get_common_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd==0x00)
		return -1;

	osal_memcpy(vb, &vdi->vpu_common_memory, sizeof(vpu_buffer_t));

	return 0;
}



int allocate_common_memory(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	vpudrv_buffer_t vdb = {0};
	int i;

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd==0x00)
		return -1;

	if (vdi->pvip->vpu_common_buffer.size == 0)
	{
		vdb.size = SIZE_COMMON*MAX_VPU_CORE_NUM;
		vdb.phys_addr = vdi->vdb_video_memory.phys_addr; // set at the beginning of base address
		vdb.virt_addr = vdi->vdb_video_memory.phys_addr;
		vdb.base = vdi->vdb_video_memory.base;
		
		// convert os driver buffer type to vpu buffer type		
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
		vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr + (coreIdx*SIZE_COMMON));
		vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base + (coreIdx*SIZE_COMMON));
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (coreIdx*SIZE_COMMON));
#else
		vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
		vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr);
		vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base);
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr);
#endif

		osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer, sizeof(vpudrv_buffer_t));
		
	}	
	else
	{
		vdb.size = SIZE_COMMON*MAX_VPU_CORE_NUM;
		vdb.phys_addr = vdi->vdb_video_memory.phys_addr; // set at the beginning of base address
		vdb.base =  vdi->vdb_video_memory.base;			
		vdb.virt_addr = vdi->vdb_video_memory.phys_addr;		

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (coreIdx*SIZE_COMMON)); 
#else
		vdi->pvip->vpu_common_buffer.virt_addr = vdb.virt_addr; 
#endif
		osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer, sizeof(vpudrv_buffer_t));

		VLOG(INFO, "[VDI] allocate_common_memory, physaddr=0x%x, virtaddr=0x%x\n", (int)vdi->pvip->vpu_common_buffer.phys_addr, (int)vdi->pvip->vpu_common_buffer.virt_addr);
	}

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].inuse == 0)
		{
			vdi->vpu_buffer_pool[i].vdb = vdb;
			vdi->vpu_buffer_pool_count++;
			vdi->vpu_buffer_pool[i].inuse = 1;
			break;
		}
	}

	VLOG(TRACE, "[VDI] vdi_get_common_memory physaddr=0x%x, size=%d, virtaddr=0x%x\n", vdi->vpu_common_memory.phys_addr, vdi->vpu_common_memory.size, vdi->vpu_common_memory.virt_addr);

	return 0;
}


vpu_instance_pool_t *vdi_get_instance_pool(unsigned long coreIdx)
{
	vdi_info_t *vdi;
	
	if (coreIdx > MAX_VPU_CORE_NUM)
		return NULL;

	vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd ==0x00 )
		return NULL;

	if (!vdi->pvip)
	{
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdi->pvip = &s_vip[coreIdx];
#else
		vdi->pvip = &s_vip;
#endif
		osal_memset(vdi->pvip, 0, sizeof(vpu_instance_pool_t));		
	}

	return (vpu_instance_pool_t *)vdi->pvip;
}

int vdi_open_instance(unsigned long coreIdx, unsigned long instIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	
	if(!vdi || !vdi || vdi->vpu_fd ==-1 || vdi->vpu_fd == 0x00)
		return -1;

	vdi->pvip->vpu_instance_num++;

	return 0;
}

int vdi_close_instance(unsigned long coreIdx, unsigned long instIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || !vdi || vdi->vpu_fd ==-1 || vdi->vpu_fd == 0x00)
		return -1;

	vdi->pvip->vpu_instance_num--;

	return 0;
}

int vdi_get_instance_num(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
		return -1;

	return vdi->pvip->vpu_instance_num;
}

int vdi_hw_reset(unsigned long coreIdx) // DEVICE_ADDR_SW_RESET
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	int ret;

	if(!vdi || !vdi || vdi->vpu_fd ==-1 || vdi->vpu_fd == 0x00)
		return -1;

	// to do any action for hw reset


	return 0;
}

int vdi_lock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	
	return 0;
}

int vdi_lock_check(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	return 0;
}

void vdi_unlock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];		
}


int vdi_disp_lock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	
	return 0;
}

void vdi_disp_unlock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
}

int vmem_lock_init(unsigned long coreIdx)
{

}

int vmem_lock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	
	return 0;
}

void vmem_unlock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
}

int vmem_lock_deinit(unsigned long coreIdx)
{

}

int vdi_write_register(unsigned long coreIdx, unsigned int addr, unsigned int data)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	unsigned long *reg_addr;
	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (coreIdx*VPU_CORE_BASE_OFFSET));		
#else
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
	*(volatile unsigned long *)reg_addr = data;	

	return 0;
}



unsigned int vdi_read_register(unsigned long coreIdx, unsigned int addr)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	unsigned long *reg_addr;
	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER	
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (coreIdx*VPU_CORE_BASE_OFFSET));	
#else
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
	return *(volatile unsigned long *)reg_addr;
}



int vdi_write_memory(unsigned long coreIdx, unsigned int addr, unsigned char *data, int len, int endian)
{	
	vdi_info_t *vdi;
	vpudrv_buffer_t vdb = {0};
	unsigned long offset;
	int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].inuse == 1)
		{
			vdb = vdi->vpu_buffer_pool[i].vdb;
			if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size))
				break;
		}
	}

	if (!vdb.size) {
		VLOG(ERR, "address 0x%08x is not mapped address!!!\n", addr);
		return -1;
	}

	offset = addr - (unsigned long)vdb.phys_addr;


	swap_endian(data, len, endian);
	osal_memcpy((void *)((unsigned long)vdb.virt_addr+offset), data, len);	


	return len;

}

int vdi_read_memory(unsigned long coreIdx, unsigned int addr, unsigned char *data, int len, int endian)
{
	vdi_info_t *vdi;
	vpudrv_buffer_t vdb = {0};
	unsigned long offset;
	int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].inuse == 1)
		{
			vdb = vdi->vpu_buffer_pool[i].vdb;
			if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size))
				break;		
		}
	}

	if (!vdb.size)
		return -1;

	offset = addr - (unsigned long)vdb.phys_addr;

	osal_memcpy(data, (const void *)((unsigned long)vdb.virt_addr+offset), len);
	swap_endian(data, len,  endian);

	return len;
}



int vdi_allocate_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi;
	int i;
	unsigned long offset;
	vpudrv_buffer_t vdb = {0};

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;	

	vdb.size = vb->size;
	vmem_lock(coreIdx);
	vdb.phys_addr = (unsigned long)vmem_alloc(&vdi->pvip->vmem, vdb.size, 0);
	vmem_unlock(coreIdx);
	
	if (vdb.phys_addr == (unsigned long)-1)
		return -1; // not enough memory

	offset = (unsigned long)(vdb.phys_addr - vdi->vdb_video_memory.phys_addr);
	vdb.base = (unsigned long )vdi->vdb_video_memory.base + offset;
	vdb.virt_addr = vdb.phys_addr;

	vb->phys_addr = (unsigned long)vdb.phys_addr;
	vb->base = (unsigned long)vdb.base;
	vb->virt_addr = (unsigned long)vb->phys_addr;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].inuse == 0)
		{
			vdi->vpu_buffer_pool[i].vdb = vdb;
			vdi->vpu_buffer_pool_count++;
			vdi->vpu_buffer_pool[i].inuse = 1;
			break;
		}
	}

	return 0;
}


int vdi_attach_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi;
	int i;
	int ret = 0;
	unsigned long offset;
	vpudrv_buffer_t vdb = {0};

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];

	if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return ;
	
	vdb.size = vb->size;
	vdb.phys_addr = vb->phys_addr;
	offset = (unsigned long)(vdb.phys_addr - vdi->vdb_video_memory.phys_addr);
	vdb.base = (unsigned long )vdi->vdb_video_memory.base + offset;
	vdb.virt_addr = vb->virt_addr;
	

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr)
		{
			vdi->vpu_buffer_pool[i].vdb = vdb;
			vdi->vpu_buffer_pool[i].inuse = 1;
			break;
		}
		else
		{
			if (vdi->vpu_buffer_pool[i].inuse == 0)
			{
				vdi->vpu_buffer_pool[i].vdb = vdb;
				vdi->vpu_buffer_pool_count++;
				vdi->vpu_buffer_pool[i].inuse = 1;
				break;
			}
		}		
	}

	return 0;
}

int vdi_dettach_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi;
	int i;
	int ret = 0;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];

	if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	if (vb->size == 0)
		return -1;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr)
		{
			vdi->vpu_buffer_pool[i].inuse = 0;
			vdi->vpu_buffer_pool_count--;
			break;
		}
	}

	return 0;
}


void vdi_free_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi;
	int i;
	vpudrv_buffer_t vdb = {0};

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];

	if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return ;

	if (vb->size == 0)
		return ;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr)
		{
			vdi->vpu_buffer_pool[i].inuse = 0;
			vdi->vpu_buffer_pool_count--;
			vdb = vdi->vpu_buffer_pool[i].vdb;
			break;
		}
	}

	if (!vdb.size)
	{
		VLOG(ERR, "[VDI] invalid buffer to free address = 0x%x\n", (int)vdb.virt_addr);
		return ;
	}
	vmem_lock(coreIdx);
	vmem_free(&vdi->pvip->vmem, (unsigned long)vdb.phys_addr, 0);
	vmem_unlock(coreIdx);
	osal_memset(vb, 0, sizeof(vpu_buffer_t));
}
		

int vdi_get_sram_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	vpudrv_buffer_t vdb = {0};
	int ret;

	if(!vb || !vdi)
		return -1;

	if (VDI_SRAM_SIZE > 0)	// if we can know the sram address directly in vdi layer, we use it first for sdram address
	{
		vb->phys_addr = VDI_SRAM_BASE_ADDR+(coreIdx*VDI_SRAM_SIZE);		
		vb->size = VDI_SRAM_SIZE;
	}

	return 0;
}



int vdi_set_clock_gate(unsigned long coreIdx, int enable)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	int ret;
	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	vdi->clock_state = enable;

	return 0;	
}

int vdi_get_clock_gate(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	return vdi->clock_state;
}


int vdi_wait_bus_busy(unsigned long coreIdx, int timeout, unsigned int gdi_busy_flag)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	//VDI must implement timeout action in this function for multi-vpu core scheduling efficiency.
	//the setting small value as timeout gives a chance to wait the other vpu core.
#ifdef SUPPORT_INTERRUPT

#else
	while(1)
	{
		if (vdi_read_register(coreIdx, gdi_busy_flag) == 0x77)
			break;

		//Sleep(1);	// 1ms sec
		//if (count++ > timeout)
		//	return -1;
	}	
#endif

	return 0;
}

int vdi_wait_vpu_busy(unsigned long coreIdx, int timeout, unsigned int addr_bit_busy_flag)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	//VDI must implement timeout action in this function for multi-vpu core scheduling efficiency.
	//the setting small value as timeout gives a chance to wait the other vpu core.
#ifdef SUPPORT_INTERRUPT

#else
	while(1)
	{
		if (vdi_read_register(coreIdx, addr_bit_busy_flag) == 0)
			break;

		//Sleep(1);	// 1ms sec
		//if (count++ > timeout)
		//	return -1;
	}	
#endif

	return 0;
}



int vdi_wait_interrupt(unsigned long coreIdx, int timeout, unsigned int addr_bit_int_reason)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;

	//VDI must implement timeout action in this function for multi-vpu core scheduling efficiency.
	//the setting small value as timeout gives a chance to wait the other vpu core.
#ifdef SUPPORT_INTERRUPT
	
#else
	while(1)
	{
		if (vdi_read_register(coreIdx, BIT_INT_STS))
		{
			if (vdi_read_register(coreIdx, addr_bit_int_reason))
	                	break;
		}

		//Sleep(1);	// 1ms sec
		//if (count++ > timeout)
		//	return -1;
	}	
#endif

	return 0;
}

static int read_pinfo_buffer(int coreIdx, int addr)
{
	int ack;
	int rdata;
#define VDI_LOG_GDI_PINFO_ADDR  (0x1068)
#define VDI_LOG_GDI_PINFO_REQ   (0x1060)
#define VDI_LOG_GDI_PINFO_ACK   (0x1064)
#define VDI_LOG_GDI_PINFO_DATA  (0x106c)
	//------------------------------------------
	// read pinfo - indirect read
	// 1. set read addr     (GDI_PINFO_ADDR)
	// 2. send req          (GDI_PINFO_REQ)
	// 3. wait until ack==1 (GDI_PINFO_ACK)
	// 4. read data         (GDI_PINFO_DATA)
	//------------------------------------------
	vdi_write_register(coreIdx, VDI_LOG_GDI_PINFO_ADDR, addr);
	vdi_write_register(coreIdx, VDI_LOG_GDI_PINFO_REQ, 1);

	ack = 0;
	while (ack == 0)
	{
		ack = vdi_read_register(coreIdx, VDI_LOG_GDI_PINFO_ACK);
	}

	rdata = vdi_read_register(coreIdx, VDI_LOG_GDI_PINFO_DATA);

	//VLOG(INFO, "[READ PINFO] ADDR[%x], DATA[%x]", addr, rdata);
	return rdata;
}


static void printf_gdi_info(int coreIdx, int num)
{
	int i;
	int bus_info_addr;
	int tmp;
	

	VLOG(INFO, "\n**GDI information for GDI_10\n");
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
	VLOG(INFO, "GDI_BUS_STATUS = %x\n", vdi_read_register(coreIdx, VDI_LOG_GDI_BUS_STATUS));

	for (i=0; i < num; i++)
	{
		VLOG(INFO, "index = %02d", i);
#define VDI_LOG_GDI_INFO_CONTROL 0x1400
		bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i*0x14;

		tmp = read_pinfo_buffer(coreIdx, bus_info_addr);	//TiledEn<<20 ,GdiFormat<<17,IntlvCbCr,<<16 GdiYuvBufStride
		VLOG(INFO, " control = 0x%08x", tmp);

		bus_info_addr += 4;
		tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
		VLOG(INFO, " pic_size = 0x%08x", tmp);

		bus_info_addr += 4;
		tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
		VLOG(INFO, " y-top = 0x%08x", tmp);

		bus_info_addr += 4;
		tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
		VLOG(INFO, " cb-top = 0x%08x", tmp);

		bus_info_addr += 4;
		tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
		VLOG(INFO, " cr-top = 0x%08x", tmp);
		VLOG(INFO, "\n");		
	}
}




void vdi_log(unsigned long coreIdx, int cmd, int step)
{
	// BIT_RUN command
	enum {
		SEQ_INIT = 1,
		SEQ_END = 2,
		PIC_RUN = 3,
		SET_FRAME_BUF = 4,
		ENCODE_HEADER = 5,
		ENC_PARA_SET = 6,
		DEC_PARA_SET = 7,
		DEC_BUF_FLUSH = 8,
		RC_CHANGE_PARAMETER	= 9,
		VPU_SLEEP = 10,
		VPU_WAKE = 11,
		ENC_ROI_INIT = 12,
		FIRMWARE_GET = 0xf,
		VPU_RESET = 0x10,
	};

	int i;

	switch(cmd)
	{
	case SEQ_INIT:
		if (step == 1)	// 
			VLOG(INFO, "\n**SEQ_INIT start\n");
		else if (step == 2)	// 
			VLOG(INFO, "\n**SEQ_INIT timeout\n");		
		else
			VLOG(INFO, "\n**SEQ_INIT end \n");		
		break;
	case SEQ_END:
		if (step == 1)	// 
			VLOG(INFO, "\n**SEQ_END start\n");
		else if (step == 2)
			VLOG(INFO, "\n**SEQ_END timeout\n");		
		else
			VLOG(INFO, "\n**SEQ_END end\n");		
		break;
	case PIC_RUN:
		if (step == 1)	// 
			VLOG(INFO, "\n**PIC_RUN start\n");
		else if (step == 2)
			VLOG(INFO, "\n**PIC_RUN timeout\n");
		else
			VLOG(INFO, "\n**PIC_RUN end\n");		
		break;
	case SET_FRAME_BUF:
		if (step == 1)	// 
			VLOG(INFO, "\n**SET_FRAME_BUF start\n");
		else if (step == 2)
			VLOG(INFO, "\n**SET_FRAME_BUF timeout\n");
		else
			VLOG(INFO, "\n**SET_FRAME_BUF end\n");		
		break;
	case ENCODE_HEADER:
		if (step == 1)	// 
			VLOG(INFO, "\n**ENCODE_HEADER start\n");
		else if (step == 2)
			VLOG(INFO, "\n**ENCODE_HEADER timeout\n");
		else
			VLOG(INFO, "\n**ENCODE_HEADER end\n");		
		break;
	case RC_CHANGE_PARAMETER:
		if (step == 1)	// 
			VLOG(INFO, "\n**RC_CHANGE_PARAMETER start\n");
		else if (step == 2)
			VLOG(INFO, "\n**RC_CHANGE_PARAMETER timeout\n");
		else
			VLOG(INFO, "\n**RC_CHANGE_PARAMETER end\n");		
		break;

	case DEC_BUF_FLUSH:
		if (step == 1)	// 
			VLOG(INFO, "\n**DEC_BUF_FLUSH start\n");
		else if (step == 2)
			VLOG(INFO, "\n**DEC_BUF_FLUSH timeout\n");
		else
			VLOG(INFO, "\n**DEC_BUF_FLUSH end ");		
		break;
	case FIRMWARE_GET:
		if (step == 1)	// 
			VLOG(INFO, "\n**FIRMWARE_GET start\n");
		else if (step == 2)
			VLOG(INFO, "\n**FIRMWARE_GET timeout\n");
		else
			VLOG(INFO, "\n**FIRMWARE_GET end\n");		
		break;
	case VPU_RESET:
		if (step == 1)	// 
			VLOG(INFO, "\n**VPU_RESET start\n");
		else if (step == 2)
			VLOG(INFO, "\n**VPU_RESET timeout\n");
		else
			VLOG(INFO, "\n**VPU_RESET end\n");
		break;
	case ENC_PARA_SET:
		if (step == 1)	// 
			VLOG(INFO, "\n**ENC_PARA_SET start\n");
		else if (step == 2)
			VLOG(INFO, "\n**ENC_PARA_SET timeout\n");
		else
			VLOG(INFO, "\n**ENC_PARA_SET end\n");
		break;
	case DEC_PARA_SET:
		if (step == 1)	// 
			VLOG(INFO, "\n**DEC_PARA_SET start\n");
		else if (step == 2)
			VLOG(INFO, "\n**DEC_PARA_SET timeout\n");
		else
			VLOG(INFO, "\n**DEC_PARA_SET end\n");
		break;
	default:
		if (step == 1)	// 
			VLOG(INFO, "\n**ANY CMD start\n");
		else if (step == 2)
			VLOG(INFO, "\n**ANY CMD timeout\n");
		else
			VLOG(INFO, "\n**ANY CMD end\n");
		break;
	}


	for (i=0; i<0x200; i=i+16)
	{
		VLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
			vdi_read_register(coreIdx, i), vdi_read_register(coreIdx, i+4),
			vdi_read_register(coreIdx, i+8), vdi_read_register(coreIdx, i+0xc));
	}
	if ((cmd == PIC_RUN && step== 0) || cmd == VPU_RESET)
	{
		printf_gdi_info(coreIdx, 32);


#define VDI_LOG_MBC_BUSY 0x0440
#define VDI_LOG_MC_BASE	 0x0C00
#define VDI_LOG_MC_BUSY	 0x0C04
#define VDI_LOG_ROT_SRC_IDX	 (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX	 (0x400 + 0x110)

		VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MBC_BUSY));
		VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MC_BUSY));
		VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n", (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 20) & 0x3F, (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
		VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_SRC_IDX));
		VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_DST_IDX));
	}
}

int swap_endian(unsigned char *data, int len, int endian)
{
	unsigned int *p;
	unsigned int v1, v2, v3;
	int i;
	const int sys_endian = VDI_SYSTEM_ENDIAN;
	int swap = 0;
	p = (unsigned int *)data;

	if(endian == sys_endian)
		swap = 0;	
	else
		swap = 1;

	if (swap)
	{		
		if (endian == VDI_LITTLE_ENDIAN ||
			endian == VDI_BIG_ENDIAN) {
				for (i=0; i<len/4; i+=2)
				{
					v1 = p[i];
					v2  = ( v1 >> 24) & 0xFF;
					v2 |= ((v1 >> 16) & 0xFF) <<  8;
					v2 |= ((v1 >>  8) & 0xFF) << 16;
					v2 |= ((v1 >>  0) & 0xFF) << 24;
					v3 =  v2;
					v1  = p[i+1];
					v2  = ( v1 >> 24) & 0xFF;
					v2 |= ((v1 >> 16) & 0xFF) <<  8;
					v2 |= ((v1 >>  8) & 0xFF) << 16;
					v2 |= ((v1 >>  0) & 0xFF) << 24;
					p[i]   =  v2;
					p[i+1] = v3;
				}
		} 
		else
		{
			int swap4byte = 0;
			if (endian == VDI_32BIT_LITTLE_ENDIAN)
			{
				if (sys_endian == VDI_BIG_ENDIAN)
				{
					swap = 1;
					swap4byte = 0;
				}
				else if (sys_endian == VDI_32BIT_BIG_ENDIAN)
				{
					swap = 1;
					swap4byte = 1;
				}	
				else if (sys_endian == VDI_LITTLE_ENDIAN)
				{
					swap = 0;
					swap4byte = 1;
				}				
			}
			else	// VDI_32BIT_BIG_ENDIAN
			{
				if (sys_endian == VDI_LITTLE_ENDIAN)
				{
					swap = 1;
					swap4byte = 0;
				}
				else if (sys_endian == VDI_32BIT_LITTLE_ENDIAN)
				{
					swap = 1;
					swap4byte = 1;
				}
				else if (sys_endian == VDI_BIG_ENDIAN)
				{
					swap = 0;
					swap4byte = 1;
				}
			}
			if (swap) {
				for (i=0; i<len/4; i++) {
					v1 = p[i];
					v2  = ( v1 >> 24) & 0xFF;
					v2 |= ((v1 >> 16) & 0xFF) <<  8;
					v2 |= ((v1 >>  8) & 0xFF) << 16;
					v2 |= ((v1 >>  0) & 0xFF) << 24;
					p[i] = v2;
				}
			}
			if (swap4byte) {
				for (i=0; i<len/4; i+=2) {
					v1 = p[i];
					v2 = p[i+1];
					p[i]   = v2;
					p[i+1] = v1;
				}
			}
		}
	}

	return swap;
}




#endif	
