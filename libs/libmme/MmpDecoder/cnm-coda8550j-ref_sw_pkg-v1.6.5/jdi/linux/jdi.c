//------------------------------------------------------------------------------
// File: jdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(linux) || defined(__linux) || defined(ANDROID)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef	_KERNEL_
#include <linux/delay.h> 
#endif
#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>

#include "driver/jpu.h"
#include "../jdi.h"
#include "../../include/jpulog.h"

#if (JPU_PLATFORM_V4L2_ENABLE  == 0)

#define JPU_BIT_REG_SIZE		0x300
#define JPU_BIT_REG_BASE		(0x10000000 + 0x3000)
#define JDI_DRAM_PHYSICAL_BASE	0x00
#define JDI_DRAM_PHYSICAL_SIZE	(128*1024*1024)

#define SUPPORT_ALLOCATE_MEMORY_FROM_DRIVER
#define SUPPORT_INTERRUPT
#define JDI_SYSTEM_ENDIAN	JDI_LITTLE_ENDIAN

#define JPU_DEVICE_NAME "/dev/jpu"

typedef pthread_mutex_t	MUTEX_HANDLE;

typedef struct jpu_buffer_pool_t
{
	jpudrv_buffer_t jdb;
	int inuse;
} jpu_buffer_pool_t;


static int s_jpu_fd;
static jpu_instance_pool_t *s_pjip;
static int s_task_num;
static int s_clock_state;
#ifdef SUPPORT_ALLOCATE_MEMORY_FROM_DRIVER
#else
static jpudrv_buffer_t s_jdb_video_memory;
#endif
static jpudrv_buffer_t s_jdb_register;
static jpu_buffer_pool_t s_jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
static int s_jpu_buffer_pool_count = 0;

static int jpu_swap_endian(unsigned char *data, int len, int endian);

int jdi_probe()
{
	int ret;

	ret = jdi_init();
	jdi_release();

	return ret;
}

int jdi_init()
{
	pthread_mutexattr_t mutexattr;
	
	if (s_jpu_fd != -1 && s_jpu_fd != 0x00)
	{
		s_task_num++;
		return 0;
	}

#ifndef __JPU_PLATFORM_MME
#ifdef ANDROID
	system("/system/lib/modules/load_android.sh");
#else
    system("./load_jpu");
#endif
#endif
	
#ifdef __JPU_PLATFORM_MME
    s_jpu_fd = mme_util_get_jpu_fd();
#else
	s_jpu_fd = open(JPU_DEVICE_NAME, O_RDWR);
	if (s_jpu_fd < 0) {
		JLOG(ERR, "[JDI] Can't open jpu driver ret=0x%x\n", (int)s_jpu_fd);
		return -1;
	}
#endif

	memset((void *)&s_jpu_buffer_pool, 0x00, sizeof(jpu_buffer_pool_t)*MAX_JPU_BUFFER_POOL);
	s_jpu_buffer_pool_count = 0;

	if (!(s_pjip = jdi_get_instance_pool()))
	{
		JLOG(ERR, "[JDI] fail to create instance pool for saving context \n");
		goto ERR_JDI_INIT;
	}

	if(!s_pjip->instance_pool_inited)
	{
#ifndef __JPU_PLATFORM_MME
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init((pthread_mutex_t *)s_pjip->jpu_mutex, &mutexattr);	
		pthread_mutexattr_destroy(&mutexattr);
#endif
	}

	if (jdi_lock() < 0)
	{
		JLOG(ERR, "[JDI] fail to pthread_mutex_t lock function\n");
		goto ERR_JDI_INIT;
	}


#ifdef SUPPORT_ALLOCATE_MEMORY_FROM_DRIVER
#else
	if (ioctl(s_jpu_fd, JDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO, &s_jdb_video_memory) < 0)
	{
		JLOG(ERR, "[JDI] fail to get video memory information\n");
		return -1;
	}

	if (!s_pjip->instance_pool_inited)
		memset(&s_pjip->vmem, 0x00, sizeof(jpeg_mm_t));
	
	if (jmem_init(&s_pjip->vmem, (unsigned long)s_jdb_video_memory.phys_addr, s_jdb_video_memory.size) < 0)
	{
		JLOG(ERR, "[JDI] fail to init jpu memory management logic\n");
		goto ERR_JDI_INIT;

	}		
	
#endif	

#ifdef __JPU_PLATFORM_MME
    s_jdb_register.size = JPU_BIT_REG_SIZE;
	s_jdb_register.virt_addr = mme_util_get_jpu_reg_vir_addr();
	
#else
	s_jdb_register.size = JPU_BIT_REG_SIZE;
	s_jdb_register.virt_addr = (unsigned long)mmap(NULL, s_jdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, s_jpu_fd, 0);
	if ((void *)s_jdb_register.virt_addr == MAP_FAILED) 
	{
		JLOG(ERR, "[JDI] fail to map vpu registers \n");
		goto ERR_JDI_INIT;
	}
#endif
	

	s_task_num++;
	jdi_unlock();

	jdi_set_clock_gate(1);

	JLOG(INFO, "[JDI] success to init driver \n");
	return s_jpu_fd;
	
ERR_JDI_INIT:
	jdi_unlock();
	jdi_release();	
	return -1;
}

int jdi_release()
{

    int ret = 0;

	JLOG(INFO, "[JDI] jdi_release s_jpu_fd =%d :: s_task_num=%d \n", s_jpu_fd, s_task_num);	
	if (s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return 0;
	
	if (jdi_lock() < 0)
	{
		JLOG(ERR, "[JDI] fail to pthread_mutex_t lock function\n");
		return -1;
	}

	if (s_task_num > 1) // means that the opened instance remains 
	{
		s_task_num--;
		jdi_unlock();
		return 0;
	}

#ifndef __JPU_PLATFORM_MME
	ret = munmap((void *)s_jdb_register.virt_addr, s_jdb_register.size);
	if(ret < 0)
		JLOG(ERR, "[JDI] jdi_release munmap s_jdb_register.virt_addr error\n");
#endif

	memset(&s_jdb_register, 0x00, sizeof(jpudrv_buffer_t));
	
	jdi_unlock();
	
#ifndef __JPU_PLATFORM_MME
#ifdef CNM_FPGA_PLATFORM

#else
	if(s_pjip)
	{
		munmap((void *)s_pjip, sizeof(jpu_instance_pool_t) + sizeof(MUTEX_HANDLE));
		s_pjip = NULL;	
	}
#endif
#endif

	s_task_num--;

#ifndef __JPU_PLATFORM_MME
	if (s_jpu_fd != -1 && s_jpu_fd != 0x00)
	{
	    JLOG(INFO, "[JDI] jdi_release close(s_jpu_fd)\n");	
		close(s_jpu_fd);
		s_jpu_fd = -1;

#ifdef CNM_FPGA_PLATFORM
		pthread_mutex_destroy(&s_io_mutex);		
		hpi_release(0);
#endif
	}
#endif

	return 0;
}



jpu_instance_pool_t *jdi_get_instance_pool()
{
	jpudrv_buffer_t jdb = {0};
	
	if(s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return NULL;

	if (!s_pjip)
	{
		jdb.size = sizeof(jpu_instance_pool_t) + sizeof(MUTEX_HANDLE);

#ifdef __JPU_PLATFORM_MME

        jdb.virt_addr = mme_util_get_jpu_instance_pool_buffer();
        jdb.phys_addr = jdb.virt_addr;
        jdb.base = jdb.virt_addr;
        jdb.ion_shared_fd = -1;

#else
		if (ioctl(s_jpu_fd, JDI_IOCTL_GET_INSTANCE_POOL, &jdb) < 0)
		{
			JLOG(ERR, "[JDI] fail to allocate get instance pool physical space=%d\n", jdb.size);
			return NULL;
		}
		
		jdb.virt_addr = (unsigned long)mmap(NULL,jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, s_jpu_fd, jdb.phys_addr);
		if ((void *)jdb.virt_addr == MAP_FAILED) 
		{
			JLOG(ERR, "[JDI] fail to map instance pool phyaddr=0x%x, size = %d\n", (int)jdb.phys_addr, (int)jdb.size);
			return NULL;
		}
#endif
		
		s_pjip = (jpu_instance_pool_t *)jdb.virt_addr;
		s_pjip->jpu_mutex = (void *)((unsigned long)s_pjip + sizeof(jpu_instance_pool_t));	//change the pointer of jpu_mutex to at end pointer of jpu_instance_pool_t to assign at allocated position.	
	}
	
	return (jpu_instance_pool_t *)s_pjip;
}


int jdi_open_instance(unsigned long instIdx)
{
	int inst_num;

	if(s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;


	if (ioctl(s_jpu_fd, JDI_IOCTL_OPEN_INSTANCE, &instIdx) < 0)
	{
		JLOG(ERR, "[VDI] fail to deliver open instance num instIdx=%d\n", (int)instIdx);
		return -1;
	}

	if (ioctl(s_jpu_fd, JDI_IOCTL_GET_INSTANCE_NUM, &inst_num) < 0)
	{
		JLOG(ERR, "[VDI] fail to deliver open instance num instIdx=%d\n", (int)instIdx);
		return -1;
	}

	s_pjip->jpu_instance_num = inst_num;

	return 0;
}


int jdi_close_instance(unsigned long instIdx)
{
	int inst_num;

	if(s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	if (ioctl(s_jpu_fd, JDI_IOCTL_CLOSE_INSTANCE, &instIdx) < 0)
	{
		JLOG(ERR, "[VDI] fail to deliver open instance num instIdx=%d\n", (int)instIdx);
		return -1;
	}

	if (ioctl(s_jpu_fd, JDI_IOCTL_GET_INSTANCE_NUM, &inst_num) < 0)
	{
		JLOG(ERR, "[VDI] fail to deliver open instance num instIdx=%d\n", (int)instIdx);
		return -1;
	}

	s_pjip->jpu_instance_num = inst_num;

	return 0;
}

int jdi_get_instance_num(unsigned long coreIdx)
{
	if(s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	return s_pjip->jpu_instance_num;
}



int jdi_hw_reset()
{
	if(s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	return ioctl(s_jpu_fd, JDI_IOCTL_RESET, 0);
}


static void resotre_mutex_in_dead(pthread_mutex_t *mutex)
{
    int mutex_value;
	if (!mutex)
		return;
#ifdef ANDROID
    mutex_value = mutex->value;
#else
    memcpy(&mutex_value, mutex, sizeof(mutex_value));
#endif
	if (mutex_value == (int)0xdead10cc) // destroy by device driver
	{
		pthread_mutexattr_t mutexattr;
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(mutex, &mutexattr);
	}
}

int jdi_lock()
{
#ifndef __JPU_PLATFORM_MME
	resotre_mutex_in_dead(s_pjip->jpu_mutex);
	pthread_mutex_lock(s_pjip->jpu_mutex);	
#endif
	return 0;
}
void jdi_unlock()
{
#ifndef __JPU_PLATFORM_MME
	pthread_mutex_unlock((pthread_mutex_t *)s_pjip->jpu_mutex);
#endif
}



void jdi_write_register(unsigned int addr, unsigned int data)
{
	unsigned long *reg_addr;

	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return ;
	reg_addr = (unsigned long *)(addr + s_jdb_register.virt_addr);
	*(volatile unsigned int *)reg_addr = data;	
}


unsigned int jdi_read_register(unsigned int addr)
{
	unsigned long *reg_addr;

	reg_addr = (unsigned long *)(addr + s_jdb_register.virt_addr);
	return *(volatile unsigned int *)reg_addr;

}


int jdi_write_memory(unsigned int addr, unsigned char *data, int len, int endian)
{
	jpudrv_buffer_t jdb;
	unsigned long offset;
	int i;


	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	memset(&jdb, 0x00, sizeof(unsigned));

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 1)
		{
			jdb = s_jpu_buffer_pool[i].jdb;
			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;
		}
	}

	if (!jdb.size) {
		JLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
		return -1;
	}
	
	offset = addr - (unsigned long)jdb.phys_addr;
	
	jpu_swap_endian(data, len, endian);
	memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);	


	return len;
}

int jdi_read_memory(unsigned int addr, unsigned char *data, int len, int endian)
{
	jpudrv_buffer_t jdb;
	unsigned long offset;
	int i;

	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	memset(&jdb, 0x00, sizeof(unsigned));

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 1)
		{
			jdb = s_jpu_buffer_pool[i].jdb;
			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;		
			}
		}

	if (!jdb.size)
		return -1;


	offset = addr - (unsigned long)jdb.phys_addr;	

	memcpy(data, (const void *)((unsigned long)jdb.virt_addr+offset), len);
	jpu_swap_endian(data, len,  endian);


	return len;
}

int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
	int i;
	int ret = 0;
	unsigned long offset;
	jpudrv_buffer_t jdb;

	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	jdi_lock();

	memset(&jdb, 0x00, sizeof(unsigned));

	jdb.size = vb->size;
	     
#ifdef SUPPORT_ALLOCATE_MEMORY_FROM_DRIVER
	if (ioctl(s_jpu_fd, JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY, &jdb) < 0)
	{
		JLOG(ERR, "[JDI] fail to jdi_allocate_dma_memory size=%d\n", vb->size);	
		jdi_unlock();
		return -1;
	}
#else
	jdb.phys_addr = (unsigned long)jmem_alloc(&s_pjip->vmem, jdb.size, 0);

	if (jdb.phys_addr == (unsigned long)-1)
	{
		jdi_unlock();
		return -1; // not enough memory
	}

	
	offset = (unsigned long)(jdb.phys_addr - s_jdb_video_memory.phys_addr);
	jdb.base = (unsigned long )s_jdb_video_memory.base + offset;
#endif
	
	vb->phys_addr = (unsigned long)jdb.phys_addr;
	vb->base = (unsigned long)jdb.base;

	//map to virtual address
	jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE,
		MAP_SHARED, s_jpu_fd, jdb.phys_addr);
	if ((void *)vb->virt_addr == MAP_FAILED) 
	{
		memset(vb, 0x00, sizeof(jpu_buffer_t));
		jdi_unlock();
		return -1;
	}
	vb->virt_addr = jdb.virt_addr;


	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 0)
		{
			s_jpu_buffer_pool[i].jdb = jdb;
			s_jpu_buffer_pool_count++;
			s_jpu_buffer_pool[i].inuse = 1;
			break;
		}
	}
	jdi_unlock();
	return 0;
}

void jdi_free_dma_memory(jpu_buffer_t *vb)
{
	int i;
	int ret = 0;
	jpudrv_buffer_t jdb;


	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return ;

	if (vb->size == 0)
		return ;

	jdi_lock();

	memset(&jdb, 0x00, sizeof(unsigned));


	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr)
		{
			s_jpu_buffer_pool[i].inuse = 0;
			s_jpu_buffer_pool_count--;
			jdb = s_jpu_buffer_pool[i].jdb;
			break;
		}
	}

	if (!jdb.size)
	{
		JLOG(ERR, "[JDI] invalid buffer to free address = 0x%x\n", (int)jdb.virt_addr);
		jdi_unlock();
		return ;
	}

#ifdef SUPPORT_ALLOCATE_MEMORY_FROM_DRIVER
	ioctl(s_jpu_fd, JDI_IOCTL_FREE_PHYSICALMEMORY, &jdb);
#else
	jmem_free(&s_pjip->vmem, (unsigned long)jdb.phys_addr, 0);
#endif

	if (munmap((void *)jdb.virt_addr, jdb.size) != 0)
	{
		JLOG(ERR, "[JDI] fail to vdi_free_dma_memory virtual address = 0x%x\n", (int)jdb.virt_addr);			
	}
	
	memset(vb, 0, sizeof(jpu_buffer_t));
	jdi_unlock();
}



int jdi_set_clock_gate(int enable)
{
	if (s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return 0;

	s_clock_state = enable;

	return ioctl(s_jpu_fd, JDI_IOCTL_SET_CLOCK_GATE, &enable);	
}

int jdi_get_clock_gate()
{
	return s_clock_state;
}

int jdi_wait_interrupt(int timeout)
{
#ifdef SUPPORT_INTERRUPT
	int ret;
	ret = ioctl(s_jpu_fd, JDI_IOCTL_WAIT_INTERRUPT, timeout);
	if (ret != 0)
		ret = -1;
	return ret;
#else
	Int64 elapse, cur;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =0;
		
	gettimeofday(&tv, NULL);
	elapse = tv.tv_sec*1000 + tv.tv_usec/1000;

	while(1)
	{
		if (jdi_read_register(MJPEG_PIC_STATUS_REG))
			break;

		gettimeofday(&tv, NULL);
		cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

		if ((cur - elapse) > timeout)
		{
			return -1;
		}
#ifdef	_KERNEL_	//do not use in real system. use SUPPORT_INTERRUPT;
		udelay(1*1000);
#else
		usleep(1*1000);
#endif // _KERNEL_
	}
	return 0;
#endif
}

void jdi_log(int cmd, int step)
{
	int i;

	switch(cmd)
	{
	case JDI_LOG_CMD_PICRUN:
		if (step == 1)	// 
			JLOG(INFO, "\n**PIC_RUN start\n");
		else
			JLOG(INFO, "\n**PIC_RUN end \n");		
		break;
	}

	JLOG(INFO, "\nClock Status=%d\n", jdi_get_clock_gate());

	for (i=0; i<=0x238; i=i+16)
	{
		JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
			jdi_read_register(i), jdi_read_register(i+4),
			jdi_read_register(i+8), jdi_read_register(i+0xc));
	}
}

int jpu_swap_endian(unsigned char *data, int len, int endian)
{
	unsigned long *p;
	unsigned long v1, v2, v3;
	int i;
	int swap = 0;
	p = (unsigned long *)data;

	if(endian == JDI_SYSTEM_ENDIAN)
		swap = 0;	
	else
		swap = 1;

	if (swap)
	{		
		if (endian == JDI_LITTLE_ENDIAN ||
			endian == JDI_BIG_ENDIAN) {
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
		} else {
			int sys_endian = JDI_SYSTEM_ENDIAN;
			int swap4byte = 0;
			swap = 0;
			if (endian == JDI_32BIT_LITTLE_ENDIAN) {
				if (sys_endian == JDI_BIG_ENDIAN) {
					swap = 1;
				}
			} else {
				if (sys_endian == JDI_BIG_ENDIAN) {
					swap4byte = 1;
				} else if (sys_endian == JDI_LITTLE_ENDIAN) {
					swap4byte = 1;
					swap = 1;
				} else {
					swap = 1;
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


#ifdef __JPU_PLATFORM_MME
int jdi_register_dma_memory(jpu_buffer_t *vb)
{
	int i;
	int ret = 0;
	unsigned long offset;
	jpudrv_buffer_t jdb;
	
	memset(&jdb, 0x00, sizeof(unsigned));

	jdb.size = vb->size;
    jdb.phys_addr = vb->phys_addr;
    jdb.base = vb->virt_addr;
    jdb.virt_addr = vb->virt_addr;

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 0)
		{
			s_jpu_buffer_pool[i].jdb = jdb;
			s_jpu_buffer_pool_count++;
			s_jpu_buffer_pool[i].inuse = 1;
			break;
		}
	}
	
	return 0;
}

void jdi_unregister_dma_memory(jpu_buffer_t *vb)
{
	int i;
	int ret = 0;
	jpudrv_buffer_t jdb;

	
	memset(&jdb, 0x00, sizeof(unsigned));


	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr)
		{
			s_jpu_buffer_pool[i].inuse = 0;
			s_jpu_buffer_pool_count--;
			jdb = s_jpu_buffer_pool[i].jdb;
			break;
		}
	}
	
}

#endif

#endif
#endif
