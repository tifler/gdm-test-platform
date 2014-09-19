//------------------------------------------------------------------------------
// File: jdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#include <winioctl.h>
#include <malloc.h>
#include "driver/Public.h"

#include "../jdi.h"
#include "../../include/jpulog.h"
#include "hpi.h"


//#define SUPPORT_INTERRUPT
#ifdef CNM_FPGA_PLATFORM
#	define JPU_BIT_REG_SIZE		0x300
#	define JPU_BIT_REG_BASE		(0x10000000 + 0x3000)
#	define JDI_DRAM_PHYSICAL_BASE	0x00
#	define JDI_DRAM_PHYSICAL_SIZE	(128*1024*1024)
#	define JDI_SYSTEM_ENDIAN	JDI_BIG_ENDIAN
#else
#	define JDI_SYSTEM_ENDIAN	JDI_LITTLE_ENDIAN
#endif

typedef HANDLE MUTEX_HANDLE;

typedef struct jpudrv_buffer_t {
	ULONG size;
	PHYSICAL_ADDRESS phys_addr;
	ULONGLONG base;
	ULONGLONG virt_addr;	
	ULONGLONG mdl;	
} jpudrv_buffer_t;


typedef struct jpu_buffer_pool_t
{
	jpudrv_buffer_t jdb;
	int inuse;
} jpu_buffer_pool_t;

static int s_use_old_hpi_driver;
static HANDLE s_jpu_fd;	
static jpu_instance_pool_t *s_pjip;
static jpu_instance_pool_t s_jip; // for old hpi driver
static int s_task_num;
static int s_clock_state;
static jpudrv_buffer_t s_jdb_video_memory;
static jpudrv_buffer_t s_jdb_register;
static jpu_buffer_pool_t s_jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
static int s_jpu_buffer_pool_count;
static MUTEX_HANDLE s_vpu_mutex;	
static HDEVINFO s_hDevInfo;
static PSP_DEVICE_INTERFACE_DETAIL_DATA s_pDeviceInterfaceDetail;	


#ifdef CNM_FPGA_PLATFORM	
static HANDLE s_io_mutex;
#endif


int jpu_swap_endian(unsigned char *data, int len, int endian);
#pragma comment(lib,"setupapi.lib")
static int GetDevicePath(unsigned long coreIdx);

// for old hpi driver
#define VPU_DEVICE_NAME                 "\\\\.\\HPI"
#define FILE_DEVICE_HPI					0x8000
#define IOCTL_CREATE_SHARE_MEMORY		CTL_CODE(FILE_DEVICE_HPI, 0x802, METHOD_OUT_DIRECT, FILE_READ_ACCESS)
typedef struct _HPI_LIST_ITEM
{
	ULONG						Ibuffer;
	ULONG						InSize;
	ULONG						OutBufferL[2];
	ULONG                       OutSize;
} HPI_LIST_ITEM, *PHPI_LIST_ITEM;

HPI_LIST_ITEM					ioItem;

int jdi_probe()
{
	int ret;

	ret = jdi_init();
#ifdef CNM_FPGA_PLATFORM
#else
	jdi_release();
#endif
	return ret;
}


int jdi_init()
{
	int ret;

	if (s_jpu_fd != (HANDLE)-1 && s_jpu_fd != (HANDLE)0x00)
	{
		s_task_num++;		
		return 0;
	}

	if (GetDevicePath(0)) 
	{
		s_jpu_fd = CreateFile(s_pDeviceInterfaceDetail->DevicePath,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (s_jpu_fd == INVALID_HANDLE_VALUE) {
			JLOG(ERR, "Fail to open new hpi driver.  Error:%d", GetLastError());
			return 0;
		}
		s_use_old_hpi_driver = 0;
	}
	else
	{
		s_jpu_fd = CreateFile(VPU_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, 0,  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (s_jpu_fd == INVALID_HANDLE_VALUE) 
		{
			JLOG(ERR, "Fail to open old hpi driver.  Error:%d", GetLastError());
			return 0;
		}

		s_use_old_hpi_driver = 1;	
		s_hDevInfo = NULL;
		s_pDeviceInterfaceDetail = NULL;
	}
	
	if (s_use_old_hpi_driver == 0)
	{
		s_jdb_register.phys_addr.QuadPart = -1; // -1 means let driver to map a memory of VPU Register.
		if (!DeviceIoControl(s_jpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
			&s_jdb_register, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
		{		
			JLOG(ERR, "Fail to map vpu register error:%d", GetLastError());
			if (s_jpu_fd)
				CloseHandle(s_jpu_fd);
			return 0;
		}
	}
	else
	{
		unsigned long ReturnedDataSize;
		ioItem.OutBufferL[0] = 0;
		ioItem.OutSize = 4;
		ioItem.Ibuffer = 0x0;
		ioItem.InSize = 0;

		if (!DeviceIoControl((HANDLE)s_jpu_fd, IOCTL_CREATE_SHARE_MEMORY, NULL, 0, ioItem.OutBufferL, ioItem.OutSize,  &ReturnedDataSize, NULL))
		{		
			JLOG(ERR, "Fail to map vpu register error:%d", GetLastError());
			if (s_jpu_fd)
				CloseHandle(s_jpu_fd);
			return 0;
		}

		s_jdb_register.virt_addr = (unsigned long)ioItem.OutBufferL[0];
	}

	

	if (!(s_pjip = jdi_get_instance_pool()))
	{
		JLOG(ERR, "[JDI] fail to create shared info for saving context \n");
		goto ERR_JDI_INIT;
	}
#ifdef CNM_FPGA_PLATFORM
	// act to starting like first loading.
	memset(s_pjip, 0x00, sizeof(jpu_instance_pool_t));	
#endif	

#ifdef UNICODE
	s_pjip->jpu_mutex = CreateMutex(NULL, FALSE, L"JPU_MUTEX");	
#else
	s_pjip->jpu_mutex = CreateMutex(NULL, FALSE, "JPU_MUTEX");	
#endif
	if (!s_pjip->jpu_mutex)
	{
		JLOG(ERR, "[JDI] fail to create a mutex\n");
		goto ERR_JDI_INIT;
	}
	
#ifdef CNM_FPGA_PLATFORM
	s_jdb_video_memory.phys_addr.QuadPart = JDI_DRAM_PHYSICAL_BASE;
	s_jdb_video_memory.size = JDI_DRAM_PHYSICAL_SIZE;
#else
	if (!DeviceIoControl(s_jpu_fd, VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO, NULL, 0,
		(LPVOID)&s_jdb_video_memory, sizeof(jpudrv_buffer_t), (LPDWORD)&ret,  NULL))
	{		
		JLOG(ERR, "[JDI] fail to get video memory which is allocated from driver\n");
		goto ERR_JDI_INIT;
	}
#endif
	
	if (!s_pjip->instance_pool_inited)
	{
		memset(&s_pjip->vmem, 0x00, sizeof(jpeg_mm_t));
		ret = jmem_init(&s_pjip->vmem, (unsigned long)s_jdb_video_memory.phys_addr.QuadPart, s_jdb_video_memory.size);
		if (ret < 0)
		{
			JLOG(ERR, "[JDI] fail to init jpu memory management logic\n");
			goto ERR_JDI_INIT;
		}
	}

	if (jdi_lock() < 0)
	{
		JLOG(ERR, "[JDI] fail to handle lock function\n");
		goto ERR_JDI_INIT;
	}



#ifdef CNM_FPGA_PLATFORM
#ifdef UNICODE
	s_io_mutex = CreateMutex(NULL, FALSE, L"JPU_IO_MUTEX");	
#else
	s_io_mutex = CreateMutex(NULL, FALSE, "JPU_IO_MUTEX");	
#endif

	hpi_init(0, JDI_DRAM_PHYSICAL_BASE);
#endif

	
	jdi_set_clock_gate(1);


	memset(&s_jpu_buffer_pool, 0x00, sizeof(jpu_buffer_pool_t)*MAX_JPU_BUFFER_POOL);
	s_jpu_buffer_pool_count = 0;

	s_task_num = 1;
	

	jdi_unlock();
	
	if (s_use_old_hpi_driver == 0)
		JLOG(INFO, "[JDI] success to init driver with new driver\n");
	else
		JLOG(INFO, "[JDI] success to init driver with old driver\n");
	return 0;

ERR_JDI_INIT:
	jdi_unlock();
	jdi_release();	
	return -1;
}

int jdi_release(void)
{
	int ret;

	if (s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return 0;
	
	
	if (jdi_lock() < 0)
	{
		JLOG(ERR, "[JDI] fail to handle lock function\n");
		return -1;
	}
	
	
	if (s_task_num > 1) // means that the opened instance remains 
	{
		s_task_num--;
		jdi_unlock();
		return 0;
	}


	s_task_num--;
	if (s_jpu_fd != (HANDLE)-1 && s_jpu_fd != (HANDLE)0x00)
	{
		if (s_use_old_hpi_driver == 0)
		{
			if (!DeviceIoControl(s_jpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
				&s_jdb_register, sizeof(jpudrv_buffer_t), (LPDWORD)&ret,  NULL))
			{		
				JLOG(ERR, "Fail to unmap vpu register error:%d", GetLastError());
				return 0;
			}
		}


		CloseHandle(s_jpu_fd);
		s_jpu_fd = (HANDLE)-1;

#ifdef CNM_FPGA_PLATFORM
		CloseHandle(s_io_mutex);
		hpi_release(0);

		jmem_exit(&s_pjip->vmem); // this is only for CNN_FPGA_PLATFORM. do not agree to call jmem_exit in real case. becuase it is stored in the sha
#endif
		if (s_hDevInfo) {
			SetupDiDestroyDeviceInfoList(s_hDevInfo);
		}

		if (s_pDeviceInterfaceDetail) {
			free(s_pDeviceInterfaceDetail);
		}

	}

	jdi_unlock();

	if (s_pjip->jpu_mutex)
	{
		CloseHandle(s_pjip->jpu_mutex);
		s_pjip->jpu_mutex = NULL;
	}

	
	if (s_use_old_hpi_driver)
	{
		s_pjip = NULL;
	}
	
	s_jpu_fd = (HANDLE)-1;

	return 0;
}




jpu_instance_pool_t *jdi_get_instance_pool()
{
	jpudrv_buffer_t jdb = {0};
	unsigned long ret;
		

	if(s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00 )
		return NULL;

	if (!s_pjip)
	{
		jdb.size = sizeof(jpu_instance_pool_t) + sizeof(MUTEX_HANDLE);

		if (s_use_old_hpi_driver)
		{
			s_pjip = &s_jip;
		}
		else
		{
			if (!DeviceIoControl(s_jpu_fd, VDI_IOCTL_GET_INSTANCE_POOL, NULL, 0,
				(LPVOID)&jdb, sizeof(jpudrv_buffer_t), (LPDWORD)&ret,  NULL))
			{		

				JLOG(ERR, "[JDI] fail to allocate get instance pool physical space=%d, error=0x%x\n", jdb.size, GetLastError());
				return NULL;
			}

			s_pjip = (jpu_instance_pool_t *)(jdb.virt_addr);
		}
		
		
		JLOG(INFO, "[JDI] instance pool physaddr=0x%x, virtaddr=0x%x, base=0x%x, size=%d\n", (int)jdb.phys_addr.QuadPart, (int)jdb.virt_addr, (int)jdb.base, (int)jdb.size);
	}


	return (jpu_instance_pool_t *)s_pjip;
}


int jdi_open_instance(unsigned long instIdx)
{
	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	s_pjip->jpu_instance_num++;

	return 0;
}

int jdi_close_instance(unsigned long instIdx)
{
	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	s_pjip->jpu_instance_num--;

	return 0;
}


int jdi_get_instance_num()
{
	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	return s_pjip->jpu_instance_num;
}


int jdi_hw_reset()
{
	if(s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	return 0;
}

int jdi_lock()
{
	const int MUTEX_TIMEOUT = INFINITE;	// ms

	if (!s_pjip || !s_pjip->jpu_mutex)
		return -1;

	if (WaitForSingleObject(s_pjip->jpu_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0)		
		return -1;
	return 0;
}

void jdi_unlock()
{
	if (!s_pjip || !s_pjip->jpu_mutex)
		return;


	ReleaseMutex(s_pjip->jpu_mutex);	
	
}

void jdi_write_register(unsigned int addr, unsigned int data)
{

	unsigned long *reg_addr = NULL;
	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return ;

#ifdef CNM_FPGA_PLATFORM
	hpi_write_register(0, (void *)s_jdb_register.virt_addr, JPU_BIT_REG_BASE+addr, data, s_io_mutex);
#else
	reg_addr = (unsigned long *)(addr + (void *)s_jdb_register.virt_addr);
	*(volatile unsigned long *)reg_addr = data;	
#endif
}
unsigned int jdi_read_register(unsigned int addr)
{

	unsigned long *reg_addr = NULL;

	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

#ifdef CNM_FPGA_PLATFORM
	return hpi_read_register(0, (void *)s_jdb_register.virt_addr, JPU_BIT_REG_BASE+addr, s_io_mutex);
#else
	reg_addr = (unsigned long *)(addr + (void *)s_jdb_register.virt_addr);
	return *(volatile unsigned long *)reg_addr;
#endif

	
}

int jdi_write_memory(unsigned int addr, unsigned char *data, int len, int endian)
{	
	jpudrv_buffer_t jdb = {0, };
	unsigned long offset;
	int i;

		if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 1)
		{
			jdb = s_jpu_buffer_pool[i].jdb;
			if (addr >= jdb.phys_addr.QuadPart && addr < (jdb.phys_addr.QuadPart + jdb.size))
				break;
		}
	}

	if (!jdb.size) {
		JLOG(ERR, "address 0x%08x is not mapped address!!!\n", addr);
		return -1;
	}

	offset = addr - (unsigned long)jdb.phys_addr.QuadPart;
	
#ifdef CNM_FPGA_PLATFORM
	hpi_write_memory(0, (void *)s_jdb_register.virt_addr, addr, data, len, endian, s_io_mutex);	
	memcpy((BYTE *)jdb.virt_addr+offset, data, len);	
#else
	jpu_swap_endian(data, len, endian);
	memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);	
#endif	
	return len;	
}


int jdi_read_memory(unsigned int addr, unsigned char *data, int len, int endian)
{
	jpudrv_buffer_t jdb = {0};
	unsigned long offset;
	int i;


	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 1)
		{
			jdb = s_jpu_buffer_pool[i].jdb;
			if (addr >= jdb.phys_addr.QuadPart && addr < (jdb.phys_addr.QuadPart + jdb.size))
				break;		
		}
	}

	if (!jdb.size)
		return -1;

	offset = addr - (unsigned long)jdb.phys_addr.QuadPart;

#ifdef CNM_FPGA_PLATFORM
	hpi_read_memory(0, (void *)s_jdb_register.virt_addr, addr, data, len, endian, s_io_mutex);	
#else
	memcpy(data, (const void *)((unsigned long)jdb.virt_addr+offset), len);
	jpu_swap_endian(data, len,  endian);
#endif

	return len;
}
int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
	int i;
	int ret = 0;
	unsigned long offset;
	jpudrv_buffer_t jdb = {0, };

	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	jdi_lock();

	jdb.size = vb->size;
	jdb.phys_addr.QuadPart = (LONGLONG)jmem_alloc(&s_pjip->vmem, jdb.size, 0);

	if (jdb.phys_addr.QuadPart == (unsigned long)-1)
	{
		jdi_unlock();
		return -1; // not enough memory
	}

	offset = (unsigned long)(jdb.phys_addr.QuadPart - s_jdb_video_memory.phys_addr.QuadPart);
	jdb.base = (unsigned long )s_jdb_video_memory.base + offset;

	vb->phys_addr = (unsigned long)jdb.phys_addr.QuadPart;
	vb->base = (unsigned long)jdb.base;	
#ifdef CNM_FPGA_PLATFORM	
	jdb.virt_addr = (unsigned long)malloc(jdb.size);
	vb->virt_addr = (unsigned long)jdb.virt_addr;
#else
	if (!DeviceIoControl(s_jpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0,
		&jdb, sizeof(jpudrv_buffer_t), (LPDWORD)&ret,  NULL))
	{
		jdi_unlock();
		return -1;
	}
	vb->virt_addr = (unsigned long)jdb.virt_addr;
#endif

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
	jpudrv_buffer_t jdb = {0, };


	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return ;

	if (vb->size == 0)
		return ;

	jdi_lock();

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].jdb.phys_addr.QuadPart == vb->phys_addr)
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

	jmem_free(&s_pjip->vmem, (unsigned long)jdb.phys_addr.QuadPart, 0);

#ifdef CNM_FPGA_PLATFORM		
	free((void *)vb->virt_addr);
#else

	if (!DeviceIoControl(s_jpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
		&jdb, sizeof(jpudrv_buffer_t), (LPDWORD)&ret,  NULL))
	{		
		JLOG(ERR, "[JDI] fail to unmap physical memory virtual address = 0x%x\n", (int)jdb.virt_addr);
		jdi_unlock();
		return;
	}
#endif

	
			
	memset(vb, 0, sizeof(jpu_buffer_t));
	jdi_unlock();
	
}

int jdi_set_clock_gate(int enable)
{
	int ret;

	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;

	s_clock_state = enable;

	DeviceIoControl(s_jpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &s_clock_state, 
		sizeof(unsigned long), NULL, 0, (LPDWORD)&ret, NULL);

	return 0;	
}

int jdi_get_clock_gate()
{
	return s_clock_state;
}




int jdi_wait_interrupt(int timeout)
{
#ifdef SUPPORT_INTERRUPT

	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return ;


	DeviceIoControl(s_jpu_fd, VDI_IOCTL_WAIT_INTERRUPT, &timeout, sizeof(unsigned long),
		NULL, 0, NULL, NULL);
#else
	LONGLONG  elapsed;
	LONGLONG  tick_per_sec;
	LONGLONG  tick_start;
	LONGLONG  tick_end;
	LARGE_INTEGER  li;

	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;


	QueryPerformanceFrequency(&li);
	tick_per_sec = (li.QuadPart);


	QueryPerformanceCounter(&li);

	tick_start = li.QuadPart;

	while(1)
	{
		if (jdi_read_register(MJPEG_PIC_STATUS_REG))
			break;

		if (tick_per_sec)
		{
			QueryPerformanceCounter(&li);
			tick_end = li.QuadPart;
			elapsed = (LONGLONG)((tick_end - tick_start)/(tick_per_sec/1000.0));
			if (elapsed > timeout)
				return -1;
		}
		Sleep(0);
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

#ifdef CNM_FPGA_PLATFORM
int jdi_set_timing_opt()
{
	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;


	return hpi_set_timing_opt(0, (void *)s_jdb_register.virt_addr, s_io_mutex);
}

int jdi_set_clock_freg(int Device, int OutFreqMHz, int InFreqMHz )
{
	if(!s_pjip || s_jpu_fd == (HANDLE)-1 || s_jpu_fd == (HANDLE)0x00)
		return -1;


	return ics307m_set_clock_freg((void *)s_jdb_register.virt_addr, Device, OutFreqMHz, InFreqMHz);
}
#endif

int GetDevicePath(unsigned long coreIdx)
{

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	SP_DEVINFO_DATA DeviceInfoData;
	ULONG size;
	int count, i, index;
	BOOL status = TRUE;
	TCHAR *DeviceName = NULL;
	TCHAR *DeviceLocation = NULL;

	//
	//  Retreive the device information for all PLX devices.
	//
	s_hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_vpudrv,
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE |
		DIGCF_PRESENT);

	//
	//  Initialize the SP_DEVICE_INTERFACE_DATA Structure.
	//
	DeviceInterfaceData.cbSize  = sizeof(SP_DEVICE_INTERFACE_DATA);

	//
	//  Determine how many devices are present.
	//
	count = 0;
	while(SetupDiEnumDeviceInterfaces(s_hDevInfo,
		NULL,
		&GUID_DEVINTERFACE_vpudrv,
		count++,  //Cycle through the available devices.
		&DeviceInterfaceData)
		);

	if (coreIdx+1 >= (unsigned long)count)
		return 0;
	//
	// Since the last call fails when all devices have been enumerated,
	// decrement the count to get the true device count.
	//
	count--;

	//
	//  If the count is zero then there are no devices present.
	//
	if (count == 0) {
		JLOG(INFO, "No VPU devices are present and enabled in the system.\n");
		return FALSE;
	}

	//
	//  Initialize the appropriate data structures in preparation for
	//  the SetupDi calls.
	//
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	//
	//  Loop through the device list to allow user to choose
	//  a device.  If there is only one device, select it
	//  by default.
	//
	i = 0;
	while (SetupDiEnumDeviceInterfaces(s_hDevInfo,
		NULL,
		(LPGUID)&GUID_DEVINTERFACE_vpudrv,
		i,
		&DeviceInterfaceData)) 
	{

		//
		// Determine the size required for the DeviceInterfaceData
		//
		SetupDiGetDeviceInterfaceDetail(s_hDevInfo,
			&DeviceInterfaceData,
			NULL,
			0,
			&size,
			NULL);

		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			JLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
			return FALSE;
		}

		s_pDeviceInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(size);

		if (!s_pDeviceInterfaceDetail) {
			JLOG(INFO, "Insufficient memory.\n");
			return FALSE;
		}

		//
		// Initialize structure and retrieve data.
		//
		s_pDeviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		status = SetupDiGetDeviceInterfaceDetail(s_hDevInfo,
			&DeviceInterfaceData,
			s_pDeviceInterfaceDetail,
			size,
			NULL,
			&DeviceInfoData);

		free(s_pDeviceInterfaceDetail);

		if (!status) {
			JLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
			return status;
		}

		//
		//  Get the Device Name
		//  Calls to SetupDiGetDeviceRegistryProperty require two consecutive
		//  calls, first to get required buffer size and second to get
		//  the data.
		//
		SetupDiGetDeviceRegistryProperty(s_hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC,
			NULL,
			(PBYTE)DeviceName,
			0,
			&size);

		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			JLOG(INFO, "SetupDiGetDeviceRegistryProperty failed, Error: %d", GetLastError());
			return FALSE;
		}

		DeviceName = (TCHAR*) malloc(size);
		if (!DeviceName) {
			JLOG(INFO, "Insufficient memory.\n");
			return FALSE;
		}

		status = SetupDiGetDeviceRegistryProperty(s_hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC,
			NULL,
			(PBYTE)DeviceName,
			size,
			NULL);
		if (!status) {
			JLOG(INFO, "SetupDiGetDeviceRegistryProperty failed, Error: %d",
				GetLastError());
			free(DeviceName);
			return status;
		}

		//
		//  Now retrieve the Device Location.
		//
		SetupDiGetDeviceRegistryProperty(s_hDevInfo,
			&DeviceInfoData,
			SPDRP_LOCATION_INFORMATION,
			NULL,
			(PBYTE)DeviceLocation,
			0,
			&size);

		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			DeviceLocation = (TCHAR*) malloc(size);

			if (DeviceLocation != NULL) {

				status = SetupDiGetDeviceRegistryProperty(s_hDevInfo,
					&DeviceInfoData,
					SPDRP_LOCATION_INFORMATION,
					NULL,
					(PBYTE)DeviceLocation,
					size,
					NULL);
				if (!status) {
					free(DeviceLocation);
					DeviceLocation = NULL;
				}
			}

		} else {
			DeviceLocation = NULL;
		}

		//
		// If there is more than one device print description.
		//
		if (count > 1) {
			JLOG(INFO, "%d- ", i);
		}

		JLOG(INFO, "%s\n", DeviceName);

		if (DeviceLocation) {
			JLOG(INFO, "        %s\n", DeviceLocation);
		}

		free(DeviceName);
		DeviceName = NULL;

		if (DeviceLocation) {
			free(DeviceLocation);
			DeviceLocation = NULL;
		}

		i++; // Cycle through the available devices.
	}

	//
	//  Select device.
	//
	index = 0;
	if (count > 1) {
		index = coreIdx;
	}

	//
	//  Get information for specific device.
	//
	status = SetupDiEnumDeviceInterfaces(s_hDevInfo,
		NULL,
		(LPGUID)&GUID_DEVINTERFACE_vpudrv,
		index,
		&DeviceInterfaceData);

	if (!status) {
		JLOG(INFO, "SetupDiEnumDeviceInterfaces failed, Error: %d", GetLastError());
		return status;
	}

	//
	// Determine the size required for the DeviceInterfaceData
	//
	SetupDiGetDeviceInterfaceDetail(s_hDevInfo,
		&DeviceInterfaceData,
		NULL,
		0,
		&size,
		NULL);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		JLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
		return FALSE;
	}

	s_pDeviceInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(size);

	if (!s_pDeviceInterfaceDetail) {
		JLOG(INFO, "Insufficient memory.\n");
		return FALSE;
	}

	//
	// Initialize structure and retrieve data.
	//
	s_pDeviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	status = SetupDiGetDeviceInterfaceDetail(s_hDevInfo,
		&DeviceInterfaceData,
		s_pDeviceInterfaceDetail,
		size,
		NULL,
		&DeviceInfoData);
	if (!status) {
		JLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
		return status;
	}

	return status;
}

#endif //#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
