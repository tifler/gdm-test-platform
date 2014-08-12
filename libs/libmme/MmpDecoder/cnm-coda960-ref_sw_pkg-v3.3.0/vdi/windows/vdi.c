//------------------------------------------------------------------------------
// File: vdi.c
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

#include "vpuconfig.h"
#include "vdi.h"
#include "vdi_osal.h"
#include "../mm.h"



typedef HANDLE MUTEX_HANDLE;

#ifdef CNM_FPGA_PLATFORM


#define ACLK_MAX					30
#define ACLK_MIN					16
#define CCLK_MAX					30
#define CCLK_MIN					16


#ifdef CNM_FPGA_USB_INTERFACE

// chipsnmedia clock generator in FPGA

#define DEVICE0_ADDR_COMMAND        0x1f000000
#define DEVICE0_ADDR_PARAM0         0x1f000004
#define DEVICE1_ADDR_COMMAND        0x1f000008
#define DEVICE1_ADDR_PARAM0         0x1f00000c
#define DEVICE_ADDR_SW_RESET		0x1f000010



static void * usb_init(unsigned long core_idx, unsigned long dram_base);
static void usb_release(void * base, unsigned long core_idx);
static int usb_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data, MUTEX_HANDLE io_mutex);
static unsigned int usb_read_register(unsigned long core_idx, void * base, unsigned int addr, MUTEX_HANDLE io_mutex);
static int usb_write_memory(unsigned long core_idx,void * base, unsigned int addr, unsigned char *data, int len, int endian, MUTEX_HANDLE io_mutex);
static int usb_read_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, MUTEX_HANDLE io_mutex);
static int usb_hw_reset(void * base, MUTEX_HANDLE io_mutex);

static int usb_set_timing_opt(unsigned long core_idx, void * base, MUTEX_HANDLE io_mutex);
static int usb_ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz, int InFreqMHz, MUTEX_HANDLE io_mutex);
static void usb_write_reg_timing(unsigned long addr, unsigned int data);

#endif	//CNM_FPGA_USB_INTERFACE

#ifdef CNM_FPGA_PCI_INTERFACE

#ifdef SUPPORT_128BIT_BUS
#define HPI_BUS_LEN 16	
#define HPI_BUS_LEN_ALIGN 15
#else
#define HPI_BUS_LEN 8
#define HPI_BUS_LEN_ALIGN 7
#endif


/*------------------------------------------------------------------------
ChipsnMedia HPI register definitions
------------------------------------------------------------------------*/
#define HPI_CHECK_STATUS			1
#define HPI_WAIT_TIME				0x100000
#define HPI_BASE					0x20030000
#define HPI_ADDR_CMD				(0x00<<2)
#define HPI_ADDR_STATUS				(0x01<<2)
#define HPI_ADDR_ADDR_H				(0x02<<2)
#define HPI_ADDR_ADDR_L				(0x03<<2)
#define HPI_ADDR_ADDR_M				(0x06<<2)
#define HPI_ADDR_DATA				(0x80<<2)

#ifdef SUPPORT_128BIT_BUS
#define HPI_CMD_WRITE_VALUE			((16 << 4) + 2)
#define HPI_CMD_READ_VALUE			((16 << 4) + 1)
#else
#define HPI_CMD_WRITE_VALUE			((8 << 4) + 2)
#define HPI_CMD_READ_VALUE			((8 << 4) + 1)
#endif

#define HPI_MAX_PKSIZE 256

// chipsnmedia clock generator in FPGA

#define	DEVICE0_ADDR_COMMAND		0x75
#define DEVICE0_ADDR_PARAM0			0x76
#define	DEVICE0_ADDR_PARAM1			0x77
#define	DEVICE1_ADDR_COMMAND		0x78
#define DEVICE1_ADDR_PARAM0			0x79
#define	DEVICE1_ADDR_PARAM1			0x7a
#define DEVICE_ADDR_SW_RESET		0x7b

static int hpi_init(unsigned long core_idx, unsigned long dram_base);
static void hpi_release(unsigned long core_idx);
static int hpi_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data, HANDLE io_mutex);
static unsigned int hpi_read_register(unsigned long core_idx, void * base, unsigned int addr, HANDLE io_mutex);
static int hpi_write_memory(unsigned long core_idx,void * base, unsigned int addr, unsigned char *data, int len, int endian, HANDLE io_mutex);
static int hpi_read_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, HANDLE io_mutex);
static int hpi_hw_reset(void * base);
static int hpi_set_timing_opt(unsigned long core_idx, void * base, HANDLE io_mutex);
static int hpi_ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz, int InFreqMHz);

#endif // CNM_FPGA_PCI_INTERFACE


#endif	//#ifdef CNM_FPGA_PLATFORM


//#define SUPPORT_INTERRUPT
#ifdef CNM_FPGA_PLATFORM
#	define VPU_BIT_REG_SIZE		(0x4000*MAX_NUM_VPU_CORE)
#	define VPU_BIT_REG_BASE		0x10000000
#	define VDI_SRAM_BASE_ADDR	0x00
#	define VDI_SRAM_SIZE		0x20000
#	ifdef CNM_FPGA_PCI_INTERFACE
#	define VDI_SYSTEM_ENDIAN	VDI_BIG_ENDIAN
#	endif
#	ifdef CNM_FPGA_USB_INTERFACE
#	define VDI_SYSTEM_ENDIAN	VDI_LITTLE_ENDIAN
#	endif
#	define VDI_DRAM_PHYSICAL_BASE	0x80000000
#	define VDI_DRAM_PHYSICAL_SIZE	(128*1024*1024)
#else
#	define VDI_SRAM_BASE_ADDR	0x00	// if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#	define VDI_SRAM_SIZE		0x20000			// FHD MAX size, 0x17D00  4K MAX size 0x34600
#	define VDI_SYSTEM_ENDIAN	VDI_LITTLE_ENDIAN
#endif

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif


typedef struct vpu_buffer_pool_t
{
	vpudrv_buffer_t vdb;
	int inuse;
} vpu_buffer_pool_t;

typedef struct _vdi_info_t {
	unsigned long coreIdx;
	HANDLE vpu_fd;	
	BYTE *vip;          // for old hpi driver
	vpu_instance_pool_t *pvip;
	int task_num;
	int clock_state;
	vpudrv_buffer_t vdb_video_memory;
	vpudrv_buffer_t vdb_register;
	vpu_buffer_t vpu_common_memory;
	vpu_buffer_pool_t vpu_buffer_pool[MAX_VPU_BUFFER_POOL];
	int vpu_buffer_pool_count;

	MUTEX_HANDLE vpu_mutex;
	MUTEX_HANDLE vpu_disp_mutex;

#ifdef CNM_FPGA_PLATFORM	
	HANDLE io_mutex;
#endif
	HDEVINFO hDevInfo;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetail;	
} vdi_info_t;

static vdi_info_t s_vdi_info[MAX_VPU_CORE_NUM];

#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE)
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
static vpu_instance_pool_t s_vip[MAX_VPU_CORE_NUM] = {0, };	// it can be used for a buffer space to save context for app process. to support running VPU in multiple process. this space should be a shared buffer.
#else
static vpu_instance_pool_t s_vip;	// it can be used for a buffer space to save context for app process. to support running VPU in multiple process. this space should be a shared buffer.
#endif
#endif

static int s_use_old_hpi_driver;

int swap_endian(unsigned char *data, int len, int endian);
static int allocate_common_memory(unsigned long coreIdx);


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



int vdi_probe(unsigned long coreIdx)
{
	int ret;

	ret = vdi_init(coreIdx);
#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_USB_INTERFACE
	if (ret == 0) 
	{
		// to probe PROM is programed. only read operation can check an error of USB
		if(vdi_read_register(coreIdx, 0x1044) == -1) 
			ret = -2;
	}
#endif
#else
	vdi_release(coreIdx);
#endif
	return ret;
}
int vdi_init(unsigned long coreIdx)
{
	int ret;
	vdi_info_t *vdi;

	if (coreIdx > MAX_VPU_CORE_NUM)
		return 0;

	vdi = &s_vdi_info[coreIdx];
		
	if (vdi->vpu_fd != (HANDLE)-1 && vdi->vpu_fd != (HANDLE)0x00)
	{
		vdi->task_num++;		
		return 0;
	}

#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE)
	vdi->vpu_fd = usb_init(coreIdx, VDI_DRAM_PHYSICAL_BASE);
	if (!vdi->vpu_fd) {
		VLOG(ERR, "usb_init failed.  Error:%d\n", vdi->vpu_fd);
		goto ERR_VDI_INIT;		
	}	

	vdi->vdb_register.phys_addr.QuadPart = -1;
	vdi->vdb_register.virt_addr = -1;
	vdi->vdb_register.size = 0;

#else
	if (GetDevicePath(coreIdx))
	{
		vdi->vpu_fd = CreateFile(vdi->pDeviceInterfaceDetail->DevicePath,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (vdi->vpu_fd == INVALID_HANDLE_VALUE) {
			VLOG(ERR, "CreateFile failed.  Error:%d", GetLastError());
			goto ERR_VDI_INIT;
		}

		s_use_old_hpi_driver = 0;
	}
	else
	{
		vdi->vpu_fd = CreateFile(VPU_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, 0,  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (vdi->vpu_fd == INVALID_HANDLE_VALUE) 
		{
			VLOG(ERR, "Fail to open old hpi driver.  Error:%d", GetLastError());
			return -1;
		}

		s_use_old_hpi_driver = 1;	
		vdi->hDevInfo = NULL;
		vdi->pDeviceInterfaceDetail = NULL;
	}

    

	if (s_use_old_hpi_driver == 0)
	{
		vdi->vdb_register.phys_addr.QuadPart = -1; // -1 means let driver to map a memory of VPU Register.
		if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
			&vdi->vdb_register, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
		{		
			VLOG(ERR, "Fail to map vpu register error:%d", GetLastError());
			goto ERR_VDI_INIT;
		}
	}
	else
	{
		unsigned long ReturnedDataSize;
		ioItem.OutBufferL[0] = 0;
		ioItem.OutSize = 4;
		ioItem.Ibuffer = 0x0;
		ioItem.InSize = 0;

		if (!DeviceIoControl((HANDLE)vdi->vpu_fd, IOCTL_CREATE_SHARE_MEMORY, NULL, 0, ioItem.OutBufferL, ioItem.OutSize,  &ReturnedDataSize, NULL))
		{		
			VLOG(ERR, "Fail to map vpu register error:%d", GetLastError());
			if (vdi->vpu_fd)
				CloseHandle(vdi->vpu_fd);
			return 0;
		}

		vdi->vdb_register.virt_addr = (ULONGLONG)ioItem.OutBufferL[0];
	}
#endif


	memset(&vdi->vpu_buffer_pool, 0x00, sizeof(vpu_buffer_pool_t)*MAX_VPU_BUFFER_POOL);

	if (!vdi_get_instance_pool(coreIdx))
	{
		VLOG(ERR, "[VDI] fail to create shared info for saving context \n");
		goto ERR_VDI_INIT;
	}

#ifdef CNM_FPGA_PLATFORM
	// act to starting like first loading.
	memset(vdi->pvip, 0x00, sizeof(vpu_instance_pool_t)+(sizeof(MUTEX_HANDLE)*3));
#endif
#ifdef UNICODE
	vdi->vpu_mutex = CreateMutex(NULL, FALSE, L"VPU_MUTEX");
#else
	vdi->vpu_mutex = CreateMutex(NULL, FALSE, "VPU_MUTEX");
#endif
	if (!vdi->vpu_mutex)
	{
		VLOG(ERR, "[VDI] fail to create mutex for vpu instance\n");
		goto ERR_VDI_INIT;
	}
#ifdef UNICODE
	vdi->vpu_disp_mutex = CreateMutex(NULL, FALSE, L"VPU_DISP_MUTEX");
#else
	vdi->vpu_disp_mutex = CreateMutex(NULL, FALSE, "VPU_DISP_MUTEX");
#endif
	if (!vdi->vpu_disp_mutex)
	{
		VLOG(ERR, "[VDI] fail to create mutext for vpu display lockl\n");
		goto ERR_VDI_INIT;
	}
	
	if (s_use_old_hpi_driver == 0)
	{
#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE)
#else
		if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO, NULL, 0,
			(LPVOID) &vdi->vdb_video_memory, sizeof(vpudrv_buffer_t), (LPDWORD) &ret, NULL))
		{

			VLOG(ERR, "[VDI] fail to get video memory which is allocated from driver\n");
			goto ERR_VDI_INIT;
		}
#endif
#ifdef CNM_FPGA_PLATFORM
		vdi->vdb_video_memory.phys_addr.QuadPart = VDI_DRAM_PHYSICAL_BASE;
		vdi->vdb_video_memory.size = VDI_DRAM_PHYSICAL_SIZE;
#endif
	}
	else {
#ifdef CNM_FPGA_PLATFORM
		vdi->vdb_video_memory.phys_addr.QuadPart = VDI_DRAM_PHYSICAL_BASE;
		vdi->vdb_video_memory.size = VDI_DRAM_PHYSICAL_SIZE;
#endif
	}

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
	ret = vmem_init(&vdi->pvip->vmem, (unsigned long)vdi->vdb_video_memory.phys_addr.QuadPart + (vdi->pvip->vpu_common_buffer.size*MAX_VPU_CORE_NUM),
		vdi->vdb_video_memory.size - (vdi->pvip->vpu_common_buffer.size*MAX_VPU_CORE_NUM));		
#else
	ret = vmem_init(&vdi->pvip->vmem, (unsigned long)vdi->vdb_video_memory.phys_addr.QuadPart + (vdi->pvip->vpu_common_buffer.size),
		vdi->vdb_video_memory.size - vdi->pvip->vpu_common_buffer.size);
#endif

#ifdef UNICODE
        vdi->pvip->vmem_mutex = CreateMutex(NULL, FALSE, L"VMEM_MUTEX");
#else
        vdi->pvip->vmem_mutex = CreateMutex(NULL, FALSE, "VMEM_MUTEX");
#endif

	if (ret < 0)
	{
		VLOG(ERR, "[VDI] fail to init vpu memory management logic\n");
		goto ERR_VDI_INIT;
	}

	vdi_set_clock_gate(coreIdx, 1);

#ifdef CNM_FPGA_PLATFORM	
#else
	if (vdi_read_register(coreIdx, 0x018) == 0) // if BIT processor is not running.
	{
		int i;
		for (i=0; i<64; i++)
			vdi_write_register(coreIdx, (i*4) + 0x100, 0x00); 
	}
#endif

	
	if (vdi_lock(coreIdx) < 0)
	{
		VLOG(ERR, "[VDI] fail to handle lock function\n");
		goto ERR_VDI_INIT;
	}


#ifdef CNM_FPGA_PLATFORM		
#ifdef UNICODE
	vdi->io_mutex = OpenMutex(SYNCHRONIZE, FALSE, L"VPU_IO_MUTEX");	
#else
	vdi->io_mutex = OpenMutex(SYNCHRONIZE, FALSE, "VPU_IO_MUTEX");	
#endif
	if (vdi->io_mutex == NULL)
	{
#ifdef UNICODE
		vdi->io_mutex = CreateMutex(NULL, FALSE, L"VPU_IO_MUTEX");	
#else
		vdi->io_mutex = CreateMutex(NULL, FALSE, "VPU_IO_MUTEX");	
#endif
	}
#ifdef CNM_FPGA_PCI_INTERFACE
	hpi_init(coreIdx, VDI_DRAM_PHYSICAL_BASE);
#endif
#endif
	
	vdi->coreIdx = coreIdx;
	vdi->task_num++;	
	vdi_unlock(coreIdx);
	
#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE)
	VLOG(INFO, "[VDI] success to init driver with USB interface\n");
#else
	if (s_use_old_hpi_driver == 0)
		VLOG(INFO, "[VDI] success to init driver with new hpi\n");
	else
		VLOG(INFO, "[VDI] success to init driver with old hpi\n");
#endif

	return 0;

ERR_VDI_INIT:
	vdi_unlock(coreIdx);
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
	vpudrv_buffer_t vdb;
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if (!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
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

    CloseHandle((HANDLE)vdi->pvip->vmem_mutex);	// that is OK for windows API
	vmem_exit(&vdi->pvip->vmem);
#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE)
#else
	if (s_use_old_hpi_driver == 0)
	{
		int ret;
		if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
			&vdi->vdb_register, sizeof(vpudrv_buffer_t), (LPDWORD) &ret, NULL))
		{
			VLOG(ERR, "Fail to unmap vpu register error:%d", GetLastError());
			return 0;
		}
	}
#endif

	vdb.size = 0;
	// get common memory information to free virtual address
	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_common_memory.phys_addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart &&
			vdi->vpu_common_memory.phys_addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart + vdi->vpu_buffer_pool[i].vdb.size))
		{
			vdi->vpu_buffer_pool[i].inuse = 0;
			vdi->vpu_buffer_pool_count--;
			vdb = vdi->vpu_buffer_pool[i].vdb;
			break;
		}
	}

	if (vdb.size > 0)
	{
#ifdef CNM_FPGA_PLATFORM

#else
		int ret;
		if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
			&vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
		{		
			VLOG(ERR, "[VDI] fail to unmap physical memory virtual address = 0x%llu\n", (int)vdb.virt_addr);
		}
#endif
		memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
	}
	

	vdi->task_num--;

	if (vdi->vpu_fd != (HANDLE)-1 && vdi->vpu_fd != (HANDLE)0x00)
	{
		
#ifdef CNM_FPGA_PLATFORM
		CloseHandle(vdi->io_mutex);
#ifdef CNM_FPGA_PCI_INTERFACE
		hpi_release(coreIdx);
		CloseHandle(vdi->vpu_fd);
#endif
#ifdef CNM_FPGA_USB_INTERFACE
		usb_release(vdi->vpu_fd, coreIdx);
#endif		
#endif
		vdi->vpu_fd = (HANDLE)-1;
		if (vdi->hDevInfo) {
			SetupDiDestroyDeviceInfoList(vdi->hDevInfo);
		}

		if (vdi->pDeviceInterfaceDetail) {
			free(vdi->pDeviceInterfaceDetail);
		}

        if (vdi->vip)
            free(vdi->vip);
	}

	vdi_unlock(coreIdx);

	if (vdi->vpu_mutex)
	{
		CloseHandle(vdi->vpu_mutex);
		vdi->vpu_mutex = NULL;
	}

	if (vdi->vpu_disp_mutex)
	{
		CloseHandle(vdi->vpu_disp_mutex);
		vdi->vpu_disp_mutex = NULL;
	}


	memset(vdi, 0x00, sizeof(vdi_info_t));
		

	return 0;
}

int vdi_get_common_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd==(HANDLE)0x00)
		return -1;

	osal_memcpy(vb, &vdi->vpu_common_memory, sizeof(vpu_buffer_t));

	return 0;
}

int allocate_common_memory(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	vpudrv_buffer_t vdb = {0};
	int i;
	int ret = 0;
	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd==(HANDLE)0x00)
		return -1;


	if (vdi->pvip->vpu_common_buffer.size == 0)
	{
		vdb.size = SIZE_COMMON*MAX_VPU_CORE_NUM;
		vdb.phys_addr = vdi->vdb_video_memory.phys_addr; // set at the beginning of base address
		vdb.base =  vdi->vdb_video_memory.base;

		
#ifdef CNM_FPGA_PLATFORM
		vdb.virt_addr = (ULONGLONG)vdb.phys_addr.QuadPart;		
#else
		if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
			&vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
		{
			return -1;
		}
#endif

		VLOG(INFO, "[VDI] allocate_common_memory, physaddr=0x%x, virtaddr=0x%x\n", vdb.phys_addr.QuadPart, vdb.virt_addr);
		// convert os driver buffer type to vpu buffer type
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
		vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr.QuadPart + (coreIdx*SIZE_COMMON));
		vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base + (coreIdx*SIZE_COMMON));
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (coreIdx*SIZE_COMMON));
#else
		vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
		vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr.QuadPart);
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
			
#ifdef CNM_FPGA_PLATFORM
		vdb.virt_addr = vdb.phys_addr.QuadPart;		
#else
		if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
			&vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
		{
			return -1;
		}		
#endif	
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (coreIdx*SIZE_COMMON)); 
#else
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)vdb.virt_addr; 
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
	vpudrv_buffer_t vdb = {0};
	
	if (coreIdx > MAX_VPU_CORE_NUM)
		return NULL;

	vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00 )
		return NULL;
	

	if (!vdi->pvip)
	{
#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE)
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdi->pvip = &s_vip[coreIdx];
#else
		vdi->pvip = &s_vip;
#endif
#else
        vdb.size = sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*3;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdb.size  *= MAX_VPU_CORE_NUM;
#endif
		if (s_use_old_hpi_driver)
		{
			vdi->vip = (BYTE *) osal_malloc(vdb.size); //&vdi->vip;
			vdi->pvip = (vpu_instance_pool_t *) (vdi->vip + (coreIdx*(sizeof(vpu_instance_pool_t) +sizeof(MUTEX_HANDLE) * 3)));
		}
		else
		{
			unsigned long ret;
			if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_GET_INSTANCE_POOL, NULL, 0,
				(LPVOID) &vdb, sizeof(vpudrv_buffer_t), (LPDWORD) &ret, NULL))
			{

				VLOG(ERR, "[VDI] fail to allocate get instance pool physical space=%d, error=0x%x\n", vdb.size, GetLastError());
				return NULL;
			}

			vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr + (coreIdx*(sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*3)));
		}
#endif
		VLOG(INFO, "[VDI] instance pool physaddr=0x%x, virtaddr=0x%x, base=0x%x, size=%d\n", (int)vdb.phys_addr.QuadPart, (int)vdb.virt_addr, (int)vdb.base, (int)vdb.size);
	}

	return (vpu_instance_pool_t *)vdi->pvip;
}

int vdi_open_instance(unsigned long coreIdx, unsigned long instIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	vdi->pvip->vpu_instance_num++;

	return 0;
}

int vdi_close_instance(unsigned long coreIdx, unsigned long instIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	vdi->pvip->vpu_instance_num--;

	return 0;
}

int vdi_get_instance_num(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	
	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	return vdi->pvip->vpu_instance_num;
}

int vdi_hw_reset(unsigned long coreIdx) // DEVICE_ADDR_SW_RESET
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	int ret;

	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_RESET, NULL, 0, NULL, 0, (LPDWORD)&ret, NULL);

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_hw_reset((void *)vdi->vdb_register.virt_addr);
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	return usb_hw_reset(vdi->vpu_fd, vdi->io_mutex);
#endif	
#endif	
}

int vdi_lock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	const unsigned int MUTEX_TIMEOUT = INFINITE;	// ms

	if(!vdi || !vdi->vpu_mutex)
		return -1;
	
	if (WaitForSingleObject(vdi->vpu_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0) {
	    return -1;
	}
	return 0;
}

int vdi_lock_check(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int ret;

    ret = WaitForSingleObject((MUTEX_HANDLE *)vdi->pvip->vpu_mutex, 0);
    if(ret == 0) 
    {
        vdi_unlock(coreIdx);
        return -1;
    }
    else
    {
        return 0;
    }
}

void vdi_unlock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	
	if(!vdi || !vdi->vpu_mutex)
		return;

	ReleaseMutex(vdi->vpu_mutex);	
	
}

int vdi_disp_lock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
    const int MUTEX_TIMEOUT = 10000;    // ms

	if(!vdi || !vdi->vpu_disp_mutex) 
		return -1;

	if (WaitForSingleObject(vdi->vpu_disp_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0)
		return -1;

    return 0;
}

void vdi_disp_unlock(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || !vdi->vpu_disp_mutex)
		return ;


	ReleaseMutex(vdi->vpu_disp_mutex);

}

int vmem_lock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    const unsigned int MUTEX_TIMEOUT = INFINITE;	// ms

    if (WaitForSingleObject((HANDLE)vdi->pvip->vmem_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0) {
        return -1;
    }
    return 0;
}

void vmem_unlock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    ReleaseMutex((HANDLE)vdi->pvip->vmem_mutex);	
}


int vdi_write_register(unsigned long coreIdx, unsigned int addr, unsigned int data)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	unsigned long *reg_addr = NULL;
	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;
#ifdef CNM_FPGA_PLATFORM		
#if 0
	if(!vdi->clock_state)
	{ 
		VLOG(ERR, "[VDI] vdi_write_register clock is in off. enter infinite loop\n");
		sleep(10);
		exit(1); 
	}
#endif
#endif

#ifdef CNM_FPGA_PLATFORM
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_write_register(coreIdx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE + (coreIdx*VPU_CORE_BASE_OFFSET) + addr, data, vdi->io_mutex);	
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	return usb_write_register(coreIdx, vdi->vpu_fd, VPU_BIT_REG_BASE + (coreIdx*VPU_CORE_BASE_OFFSET) + addr, data, vdi->io_mutex);	
#endif	
#else
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_write_register(coreIdx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE+ addr, data, vdi->io_mutex);
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	return usb_write_register(coreIdx, vdi->vpu_fd, VPU_BIT_REG_BASE + addr, data, vdi->io_mutex);
#endif
#endif	
#else
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (coreIdx*VPU_CORE_BASE_OFFSET));	
#else
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
	*(volatile unsigned long *)reg_addr = data;	
#endif
    return 0;
}



unsigned int vdi_read_register(unsigned long coreIdx, unsigned int addr)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	unsigned long *reg_addr = NULL;

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

#ifdef CNM_FPGA_PLATFORM		
#if 0
	if(!vdi->clock_state)
	{ 
		VLOG(ERR, "[VDI] vdi_read_register clock is in off. enter infinite loop\n");
		sleep(10);
		exit(1); 
	}
#endif
#endif

#ifdef CNM_FPGA_PLATFORM

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_read_register(coreIdx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE + (coreIdx*VPU_CORE_BASE_OFFSET) + addr, vdi->io_mutex);	
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	return usb_read_register(coreIdx, vdi->vpu_fd, VPU_BIT_REG_BASE + (coreIdx*VPU_CORE_BASE_OFFSET) + addr, vdi->io_mutex);	
#endif
#else
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_read_register(coreIdx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE+addr, vdi->io_mutex);
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	return usb_read_register(coreIdx, vdi->vpu_fd, VPU_BIT_REG_BASE+addr, vdi->io_mutex);
#endif	
#endif

#else

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (coreIdx*VPU_CORE_BASE_OFFSET));	
#else
	reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
		
	return *(volatile unsigned long *)reg_addr;
#endif
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

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].inuse == 1)
		{
			vdb = vdi->vpu_buffer_pool[i].vdb;
			if (addr >= vdb.phys_addr.QuadPart && addr < (vdb.phys_addr.QuadPart + vdb.size))
				break;
		}
	}

	if (!vdb.size) {
		VLOG(ERR, "address 0x%08x is not mapped address!!!\n", addr);
		return -1;
	}

	offset = addr - (unsigned long)vdb.phys_addr.QuadPart;
		

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
	hpi_write_memory(coreIdx, (void *)vdi->vdb_register.virt_addr, addr, data, len, endian, vdi->io_mutex);	
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	usb_write_memory(coreIdx, vdi->vpu_fd, addr, data, len, endian, vdi->io_mutex);	
#endif	
#else
	swap_endian(data, len, endian);
	osal_memcpy((void *)(vdb.virt_addr+offset), data, len);	
#endif	

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

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].inuse == 1)
		{
			vdb = vdi->vpu_buffer_pool[i].vdb;
			if (addr >= vdb.phys_addr.QuadPart && addr < (vdb.phys_addr.QuadPart + vdb.size))
				break;		
		}
	}

	if (!vdb.size)
		return -1;

	offset = addr - (unsigned long)vdb.phys_addr.QuadPart;

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
	hpi_read_memory(coreIdx, (void *)vdi->vdb_register.virt_addr, addr, data, len, endian, vdi->io_mutex);	
#endif
#ifdef CNM_FPGA_USB_INTERFACE
	usb_read_memory(coreIdx, vdi->vpu_fd, addr, data, len, endian, vdi->io_mutex);	
#endif	
#else
	osal_memcpy(data, (const void *)(vdb.virt_addr+offset), len);
	swap_endian(data, len,  endian);
#endif
	
	return len;
}

int vdi_allocate_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
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

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;	

	
	vdb.size = vb->size;
        vmem_lock(coreIdx);
	vdb.phys_addr.QuadPart = (ULONGLONG)vmem_alloc(&vdi->pvip->vmem, vdb.size, 0);
	if ((ULONG)vdb.phys_addr.QuadPart == (ULONG)-1)
	{
		vmem_unlock(coreIdx);
		return -1; // not enough memory
	}
	offset = (unsigned long)(vdb.phys_addr.QuadPart - vdi->vdb_video_memory.phys_addr.QuadPart);
	vdb.base = (unsigned long )vdi->vdb_video_memory.base + offset;
	
	vb->phys_addr = (unsigned long)vdb.phys_addr.QuadPart;
	vb->base = (unsigned long)vdb.base;
#ifdef CNM_FPGA_PLATFORM	
	vb->virt_addr = (unsigned long)vb->phys_addr;
#else
	if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0,
		&vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
	{
		vmem_unlock(coreIdx);
		return -1;
	}
	vb->virt_addr = (unsigned long)vdb.virt_addr;
#endif
	vmem_unlock(coreIdx);
	
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

	VLOG(INFO, "[VDI] vdi_allocate_dma_memory, physaddr=0x%p, virtaddr=0x%p, size=%d\n", vb->phys_addr, vb->virt_addr, vb->size);

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

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;	

	vdb.size = vb->size;
	vdb.phys_addr.QuadPart = vb->phys_addr;
	offset = (unsigned long)(vdb.phys_addr.QuadPart - vdi->vdb_video_memory.phys_addr.QuadPart);
	vdb.base = (unsigned long )vdi->vdb_video_memory.base + offset;
#ifdef CNM_FPGA_PLATFORM
	vdb.virt_addr = vb->phys_addr;
#else
	vdb.virt_addr = vb->virt_addr;
#endif

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart == vb->phys_addr)
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

	if(!vb || !vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	if (vb->size == 0)
		return -1;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart == vb->phys_addr)
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
	int ret = 0;
	vpudrv_buffer_t vdb = {0};

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	coreIdx = 0;
#endif
	vdi = &s_vdi_info[coreIdx];
	if(!vb || !vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return ;

	if (vb->size == 0)
		return ;

	for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
	{
		if (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart == vb->phys_addr)
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
	vmem_free(&vdi->pvip->vmem, (unsigned long)vdb.phys_addr.QuadPart, 0);
#ifdef CNM_FPGA_PLATFORM	
#else
	if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
		&vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
	{		
		VLOG(ERR, "[VDI] fail to unmap physical memory virtual address = 0x%x\n", (int)vdb.virt_addr);
		vmem_unlock(coreIdx);
		return;
	}
#endif
	vmem_unlock(coreIdx);

	osal_memset(vb, 0, sizeof(vpu_buffer_t));
}

int vdi_get_sram_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	vpudrv_buffer_t vdb = {0};

	if(!vb || !vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	if (VDI_SRAM_SIZE > 0)	// if we can know the sram address directly in vdi layer, we use it first for sdram address
	{
#ifdef CNM_FPGA_PLATFORM
		vb->phys_addr = VDI_SRAM_BASE_ADDR;		// CNM FPGA platform has different SRAM per core		
#else
		vb->phys_addr = VDI_SRAM_BASE_ADDR+(coreIdx*VDI_SRAM_SIZE);		
#endif
		vb->size = VDI_SRAM_SIZE;
		return 0;
	}

	
	return 0;
}

int vdi_set_clock_gate(unsigned long coreIdx, int enable)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	int ret;
	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	vdi->clock_state = enable;

	DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &vdi->clock_state, 
		sizeof(unsigned long), NULL, 0, (LPDWORD)&ret, NULL);

	return 0;	
}

int vdi_get_clock_gate(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	return vdi->clock_state;
}


int vdi_wait_bus_busy(unsigned long coreIdx, int timeout, unsigned int gdi_busy_flag)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	LONGLONG  elapsed;
	LONGLONG  tick_per_sec;
	LONGLONG  tick_start;
	LONGLONG  tick_end;
	LARGE_INTEGER  li;

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	QueryPerformanceFrequency(&li);
	tick_per_sec = (li.QuadPart);


	QueryPerformanceCounter(&li);
	tick_start = li.QuadPart;

	while(1)
	{
		if (vdi_read_register(coreIdx, gdi_busy_flag) == 0x77)
			break;
		if (tick_per_sec)
		{
			QueryPerformanceCounter(&li);
			tick_end = li.QuadPart;
			elapsed = (LONGLONG)((tick_end - tick_start)/(tick_per_sec/1000.0));
			if (elapsed > timeout) {
				VLOG(ERR, "[VDI] vdi_wait_bus_busy timeout, PC=0x%x\n", vdi_read_register(coreIdx, 0x018));
				return -1;
			}
		}
		Sleep(0);
	}
	return 0;

}

int vdi_wait_vpu_busy(unsigned long coreIdx, int timeout, unsigned int addr_bit_busy_flag)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	LONGLONG  elapsed;
	LONGLONG  tick_per_sec;
	LONGLONG  tick_start;
	LONGLONG  tick_end;
	LARGE_INTEGER  li;

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	QueryPerformanceFrequency(&li);
	tick_per_sec = (li.QuadPart);


	QueryPerformanceCounter(&li);
	tick_start = li.QuadPart;

	while(1)
	{
		if (vdi_read_register(coreIdx, addr_bit_busy_flag) == 0)
			break;
		if (tick_per_sec)
		{
			QueryPerformanceCounter(&li);
			tick_end = li.QuadPart;
			elapsed = (LONGLONG)((tick_end - tick_start)/(tick_per_sec/1000.0));
			if (elapsed > timeout) {
				VLOG(ERR, "[VDI] vdi_wait_vpu_busy timeout, PC=0x%x\n", vdi_read_register(coreIdx, 0x018));
				return -1;
			}
		}
		Sleep(0);
	}
	return 0;

}
int vdi_wait_interrupt(unsigned long coreIdx, int timeout, unsigned int addr_bit_int_reason)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
#ifdef SUPPORT_INTERRUPT

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_WAIT_INTERRUPT, &timeout, sizeof(unsigned long),
		NULL, 0, NULL, NULL);
#else
	LONGLONG  elapsed;
	LONGLONG  tick_per_sec;
	LONGLONG  tick_start;
	LONGLONG  tick_end;
	LARGE_INTEGER  li;

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	QueryPerformanceFrequency(&li);
	tick_per_sec = (li.QuadPart);


	QueryPerformanceCounter(&li);

	tick_start = li.QuadPart;

	while(1)
	{
		//if (coreIdx == 1)
		//	VLOG(INFO, "coreIdx=%d, reason=0x%x PC=0x%x, CMD=0x%x\n", coreIdx, vdi_read_register(coreIdx, 0x174), vdi_read_register(coreIdx, 0x018), vdi_read_register(coreIdx, 0x164));

		if (vdi_read_register(coreIdx, BIT_INT_STS))
		{
			if (vdi_read_register(coreIdx, addr_bit_int_reason))
	                	break;
		}
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

static void printf_gdi_info(int coreIdx, int num, int reset)
{
	int i;
	int bus_info_addr;
	int tmp;	
	VLOG(INFO, "\n**GDI information for GDI_10\n");
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
	VLOG(INFO, "GDI_BUS_STATUS = %x\n", vdi_read_register(coreIdx, VDI_LOG_GDI_BUS_STATUS));
	for (i=0; i < num; i++)
	{
		
#define VDI_LOG_GDI_INFO_CONTROL 0x1400
		bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i*0x14;
		if (reset)
		{
			vdi_write_register(coreIdx, bus_info_addr, 0x00);
			bus_info_addr += 4;
			vdi_write_register(coreIdx, bus_info_addr, 0x00);
			bus_info_addr += 4;
			vdi_write_register(coreIdx, bus_info_addr, 0x00);
			bus_info_addr += 4;
			vdi_write_register(coreIdx, bus_info_addr, 0x00);
			bus_info_addr += 4;
			vdi_write_register(coreIdx, bus_info_addr, 0x00);


		}
		else 
		{
			VLOG(INFO, "index = %02d", i);

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
		printf_gdi_info(coreIdx, 32, 0);

#define VDI_LOG_MBC_BUSY 0x0444
#define VDI_LOG_MC_BASE	 0x0C00
#define VDI_LOG_MC_BUSY	 0x0C04
#define VDI_LOG_ROT_SRC_IDX	 (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX	 (0x400 + 0x110)

		VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MBC_BUSY));
		VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MC_BUSY));
		VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n", (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 20) & 0x3F, (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
		

		VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_SRC_IDX));
		VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_DST_IDX));

		VLOG(INFO, "P_MC_PIC_INDEX_0 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x200));
		VLOG(INFO, "P_MC_PIC_INDEX_1 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x20c));
		VLOG(INFO, "P_MC_PIC_INDEX_2 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x218));
		VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x230));
		VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x23C));
		VLOG(INFO, "P_MC_PIC_INDEX_4 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x248));
		VLOG(INFO, "P_MC_PIC_INDEX_5 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x254));
		VLOG(INFO, "P_MC_PIC_INDEX_6 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x260));
		VLOG(INFO, "P_MC_PIC_INDEX_7 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x26C));
		VLOG(INFO, "P_MC_PIC_INDEX_8 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x278));
		VLOG(INFO, "P_MC_PIC_INDEX_9 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x284));
		VLOG(INFO, "P_MC_PIC_INDEX_a = %x\n", vdi_read_register(coreIdx, MC_BASE+0x290));
		VLOG(INFO, "P_MC_PIC_INDEX_b = %x\n", vdi_read_register(coreIdx, MC_BASE+0x29C));
		VLOG(INFO, "P_MC_PIC_INDEX_c = %x\n", vdi_read_register(coreIdx, MC_BASE+0x2A8));
		VLOG(INFO, "P_MC_PIC_INDEX_d = %x\n", vdi_read_register(coreIdx, MC_BASE+0x2B4));

		VLOG(INFO, "P_MC_PICIDX_0 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x028));
		VLOG(INFO, "P_MC_PICIDX_1 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x02C));
		


		
		
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



int GetDevicePath(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	SP_DEVINFO_DATA DeviceInfoData;
	ULONG size;
	int count, i, index;
	BOOL status = TRUE;
	TCHAR *DeviceName = NULL;
	TCHAR *DeviceLocation = NULL;

	//
	//  Retrieve the device information for all PLX devices.
	//
	vdi->hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_vpudrv,
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
	while(SetupDiEnumDeviceInterfaces(vdi->hDevInfo,
		NULL,
		&GUID_DEVINTERFACE_vpudrv,
		count++,  //Cycle through the available devices.
		&DeviceInterfaceData)
		);

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#else
	if (coreIdx+1 >= (unsigned long)count)
		return 0;
#endif
	//
	// Since the last call fails when all devices have been enumerated,
	// decrement the count to get the true device count.
	//
	count--;

	//
	//  If the count is zero then there are no devices present.
	//
	if (count == 0) {
		VLOG(INFO, "No VPU devices are present and enabled in the system.\n");
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
	while (SetupDiEnumDeviceInterfaces(vdi->hDevInfo,
		NULL,
		(LPGUID)&GUID_DEVINTERFACE_vpudrv,
		i,
		&DeviceInterfaceData)) 
	{

		//
		// Determine the size required for the DeviceInterfaceData
		//
		SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
			&DeviceInterfaceData,
			NULL,
			0,
			&size,
			NULL);

		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
			return FALSE;
		}

		vdi->pDeviceInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(size);

		if (!vdi->pDeviceInterfaceDetail) {
			VLOG(INFO, "Insufficient memory.\n");
			return FALSE;
		}

		//
		// Initialize structure and retrieve data.
		//
		vdi->pDeviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		status = SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
			&DeviceInterfaceData,
			vdi->pDeviceInterfaceDetail,
			size,
			NULL,
			&DeviceInfoData);

		free(vdi->pDeviceInterfaceDetail);

		if (!status) {
			VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
			return status;
		}

		//
		//  Get the Device Name
		//  Calls to SetupDiGetDeviceRegistryProperty require two consecutive
		//  calls, first to get required buffer size and second to get
		//  the data.
		//
		SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC,
			NULL,
			(PBYTE)DeviceName,
			0,
			&size);

		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			VLOG(INFO, "SetupDiGetDeviceRegistryProperty failed, Error: %d", GetLastError());
			return FALSE;
		}

		DeviceName = (TCHAR*) malloc(size);
		if (!DeviceName) {
			VLOG(INFO, "Insufficient memory.\n");
			return FALSE;
		}

		status = SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC,
			NULL,
			(PBYTE)DeviceName,
			size,
			NULL);
		if (!status) {
			VLOG(INFO, "SetupDiGetDeviceRegistryProperty failed, Error: %d",
				GetLastError());
			free(DeviceName);
			return status;
		}

		//
		//  Now retrieve the Device Location.
		//
		SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
			&DeviceInfoData,
			SPDRP_LOCATION_INFORMATION,
			NULL,
			(PBYTE)DeviceLocation,
			0,
			&size);

		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			DeviceLocation = (TCHAR*) malloc(size);

			if (DeviceLocation != NULL) {

				status = SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
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
			VLOG(INFO, "%d- ", i);
		}

		VLOG(INFO, "%s\n", DeviceName);

		if (DeviceLocation) {
			VLOG(INFO, "        %s\n", DeviceLocation);
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
	status = SetupDiEnumDeviceInterfaces(vdi->hDevInfo,
		NULL,
		(LPGUID)&GUID_DEVINTERFACE_vpudrv,
		index,
		&DeviceInterfaceData);

	if (!status) {
		VLOG(INFO, "SetupDiEnumDeviceInterfaces failed, Error: %d", GetLastError());
		return status;
	}

	//
	// Determine the size required for the DeviceInterfaceData
	//
	SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
		&DeviceInterfaceData,
		NULL,
		0,
		&size,
		NULL);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
		return FALSE;
	}

    vdi->pDeviceInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(size);

	if (!vdi->pDeviceInterfaceDetail) {
		VLOG(INFO, "Insufficient memory.\n");
		return FALSE;
	}

	//
	// Initialize structure and retrieve data.
	//
	vdi->pDeviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	status = SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
		&DeviceInterfaceData,
		vdi->pDeviceInterfaceDetail,
		size,
		NULL,
		&DeviceInfoData);
	if (!status) {
		VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
		return status;
	}

	return status;
}

void vdi_set_sdram(unsigned long coreIdx, unsigned int addr, int len, unsigned char data, int endian)
{
#ifdef CNM_FPGA_PLATFORM
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	unsigned char *buf;

	if(!vdi || vdi->vpu_fd==(void *)-1 || vdi->vpu_fd == (void *)0x00)
		return ;

	buf = (unsigned char *)malloc(len);
	memset(buf, 0x00, len);
	vdi_write_memory(coreIdx, addr, buf, len, endian);
	free(buf);

#endif
}

#ifdef CNM_FPGA_PLATFORM
int vdi_set_timing_opt( unsigned long coreIdx )
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
//    int         id;

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

//     id = vdi_read_register(coreIdx, DBG_CONFIG_REPORT_1);
//     if (id == 0x4100)
//         return 0;

#ifdef CNM_FPGA_USB_INTERFACE
	return usb_set_timing_opt(coreIdx, vdi->vpu_fd, vdi->io_mutex);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_set_timing_opt(coreIdx, (void *)vdi->vdb_register.virt_addr, vdi->io_mutex);
#endif
}

int vdi_set_clock_freg( unsigned long coreIdx, int Device, int OutFreqMHz, int InFreqMHz )
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

#ifdef CNM_FPGA_USB_INTERFACE
	return usb_ics307m_set_clock_freg(vdi->vpu_fd, Device, OutFreqMHz, InFreqMHz, vdi->io_mutex);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
	return hpi_ics307m_set_clock_freg((void *)vdi->vdb_register.virt_addr, Device, OutFreqMHz, InFreqMHz);
#endif
}
void vdi_init_fpga(int core_idx)
{
	// init FPGA
	int aclk_freq, cclk_freq;


	vdi_hw_reset(core_idx);
	Sleep(500);
	vdi_set_timing_opt(core_idx);

	// Clock Default
	aclk_freq		= 30;
	cclk_freq		= 30;

	printf("Set default ACLK to %d\n", aclk_freq);
	vdi_set_clock_freg(core_idx, 0, aclk_freq, 10);	// ACLK	
	printf("Set default CCLK to %d\n", cclk_freq);
	vdi_set_clock_freg(core_idx, 1, cclk_freq, 10);	// CCLK

	Sleep(500);
}
#endif
#endif

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE

static void * s_hpi_base;
static unsigned long s_dram_base;

static int hpi_read_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int *data, HANDLE io_mutex);
static int hpi_write_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int data, HANDLE io_mutex);
static unsigned int pci_read_reg(unsigned int addr);
static void pci_write_reg(unsigned int addr, unsigned int data);
static void pci_write_memory(unsigned int addr, unsigned char *buf, int size);
static void pci_read_memory(unsigned int addr, unsigned char *buf, int size);

int vdi_load_prom_via_usb(unsigned long core_idx, int need_init, const char* path)
{
	return -1;
}

int hpi_init(unsigned long core_idx, unsigned long dram_base)
{
    if (core_idx>MAX_NUM_VPU_CORE)
        return -1;
	s_dram_base = dram_base;
	
	return 1;
}

void hpi_release(unsigned long core_idx)
{
}

int hpi_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data, HANDLE io_mutex)
{
	int status;

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return -1;
	
	s_hpi_base = base;

	pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
	pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

	pci_write_reg(HPI_ADDR_DATA, ((data >> 16) & 0xFFFF));
	pci_write_reg(HPI_ADDR_DATA + 4, (data & 0xFFFF));

	pci_write_reg(HPI_ADDR_CMD, HPI_CMD_WRITE_VALUE);

	
	do {
		status = pci_read_reg(HPI_ADDR_STATUS);
		status = (status>>1) & 1;
	} while (status == 0);

	ReleaseMutex(io_mutex);	

    return 0;
}



unsigned int hpi_read_register(unsigned long core_idx, void * base, unsigned int addr, HANDLE io_mutex)
{
	int status;
	unsigned int data;

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return -1;

	s_hpi_base = base;
	
	pci_write_reg(HPI_ADDR_ADDR_H, ((addr >> 16)&0xffff));
	pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));
	
	pci_write_reg(HPI_ADDR_CMD, HPI_CMD_READ_VALUE);

	do {
		status = pci_read_reg(HPI_ADDR_STATUS);
		status = status & 1;
	} while (status == 0);


	data = pci_read_reg(HPI_ADDR_DATA) << 16;
	data |= pci_read_reg(HPI_ADDR_DATA + 4);

	ReleaseMutex(io_mutex);	

	return data;
}


int hpi_write_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, HANDLE io_mutex)
{	
	unsigned char *pBuf;
	unsigned char lsBuf[HPI_BUS_LEN];
	int lsOffset;	

	
	if (addr < s_dram_base) {
		fprintf(stderr, "[HPI] invalid address base address is 0x%x\n", s_dram_base);
		return 0;
	}

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return 0;

	if (len==0) {
		ReleaseMutex(io_mutex);
		return 0;
	}
	


	addr = addr - s_dram_base;
	s_hpi_base = base; 

	lsOffset = addr - (addr/HPI_BUS_LEN)*HPI_BUS_LEN;
	if (lsOffset)
	{
		pci_read_memory((addr/HPI_BUS_LEN)*HPI_BUS_LEN, lsBuf, HPI_BUS_LEN);
        swap_endian(lsBuf, HPI_BUS_LEN, endian);	
		pBuf = (unsigned char *)malloc((len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
		if (pBuf)
		{
			memset(pBuf, 0x00, (len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
			memcpy(pBuf, lsBuf, HPI_BUS_LEN);
			memcpy(pBuf+lsOffset, data, len);
			swap_endian(pBuf, ((len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN), endian);	
			pci_write_memory((addr/HPI_BUS_LEN)*HPI_BUS_LEN, (unsigned char *)pBuf, ((len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));	
			free(pBuf);
		}
	}
	else
	{
		pBuf = (unsigned char *)malloc((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
		if (pBuf) {
			memset(pBuf, 0x00, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
			memcpy(pBuf, data, len);
			swap_endian(pBuf, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN, endian);	
			pci_write_memory(addr, (unsigned char *)pBuf,(len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);	
			free(pBuf);
		}
	}
	
	ReleaseMutex(io_mutex);	

	return len;	
}

int hpi_read_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, HANDLE io_mutex)
{
	unsigned char *pBuf;
	unsigned char lsBuf[HPI_BUS_LEN];
	int lsOffset;	
	
	if (addr < s_dram_base) {
		fprintf(stderr, "[HPI] invalid address base address is 0x%x\n", s_dram_base);
		return 0;
	}


	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return 0;

	if (len==0) {
		ReleaseMutex(io_mutex);
		return 0;
	}
	
	addr = addr - s_dram_base;
	s_hpi_base = base; 

	lsOffset = addr - (addr/HPI_BUS_LEN)*HPI_BUS_LEN;
	if (lsOffset)
	{
		pci_read_memory((addr/HPI_BUS_LEN)*HPI_BUS_LEN, lsBuf, HPI_BUS_LEN);	
		swap_endian(lsBuf, HPI_BUS_LEN, endian);	
		len = len-(HPI_BUS_LEN-lsOffset);
		pBuf = (unsigned char *)malloc(((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));
		if (pBuf)
		{
			memset(pBuf, 0x00, ((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));
			pci_read_memory((addr+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN, pBuf, ((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));	
			swap_endian(pBuf, ((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN), endian);	

			memcpy(data, lsBuf+lsOffset, HPI_BUS_LEN-lsOffset);
			memcpy(data+HPI_BUS_LEN-lsOffset, pBuf, len);

			free(pBuf);
		}
	}
	else
	{
		pBuf = (unsigned char *)malloc((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
		if (pBuf)
		{
			memset(pBuf, 0x00, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
			pci_read_memory(addr, pBuf, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
			swap_endian(pBuf, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN, endian);	
			memcpy(data, pBuf, len);
			free(pBuf);
		}
	}

	ReleaseMutex(io_mutex);	

	return len;
}


int hpi_hw_reset(void * base)
{

	s_hpi_base = base;
	pci_write_reg(DEVICE_ADDR_SW_RESET<<2, 1);		// write data 1	
	return 0;
}



int hpi_write_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int data, HANDLE io_mutex)
{
	int status;
	int i;

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return 0;

	pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
	pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

	pci_write_reg(HPI_ADDR_DATA, ((data >> 16) & 0xFFFF));
	pci_write_reg(HPI_ADDR_DATA + 4, (data & 0xFFFF));

	pci_write_reg(HPI_ADDR_CMD, HPI_CMD_WRITE_VALUE);

	i = 0;
	do {
		status = pci_read_reg(HPI_ADDR_STATUS);
		status = (status>>1) & 1;
		if (i++ > 10000)
		{
			ReleaseMutex(io_mutex);
			return 0;
		}
	} while (status == 0);

	ReleaseMutex(io_mutex);	

	return 1;
}



int hpi_read_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int *data, HANDLE io_mutex)
{
	int status;
	int i;

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return 0;


	pci_write_reg(HPI_ADDR_ADDR_H, ((addr >> 16)&0xffff));
	pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

	pci_write_reg(HPI_ADDR_CMD, HPI_CMD_READ_VALUE);

	i=0;
	do {
		status = pci_read_reg(HPI_ADDR_STATUS);
		status = status & 1;
		if (i++ > 10000)
		{
			ReleaseMutex(io_mutex);
			return 0;
		}
	} while (status == 0);


	*data = pci_read_reg(HPI_ADDR_DATA) << 16;
	*data |= pci_read_reg(HPI_ADDR_DATA + 4);

	ReleaseMutex(io_mutex);	

	return 1;
}


/*------------------------------------------------------------------------
	Usage : used to program output frequency of ICS307M
	Artument :
		Device		: first device selected if 0, second device if 1.
		OutFreqMHz	: Target output frequency in MHz.
		InFreqMHz	: Input frequency applied to device in MHz
					  this must be 10 here.
	Return : TRUE if success, FALSE if invalid OutFreqMHz.
------------------------------------------------------------------------*/



int hpi_ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz, int InFreqMHz)
{
	
	int		VDW, RDW, OD, SDW, tmp;
	int		min_clk ; 
	int		max_clk ;

	s_hpi_base = base;
	if ( Device == 0 )
	{   
		min_clk = ACLK_MIN ;
		max_clk = ACLK_MAX ;
	}
	else
	{   
		min_clk = CCLK_MIN ;
		max_clk = CCLK_MAX ;
	}
	
	if (OutFreqMHz < min_clk || OutFreqMHz > max_clk) {
	   // printf ("Target Frequency should be from %2d to %2d !!!\n", min_clk, max_clk);
		return 0;
	}

	if (OutFreqMHz >= min_clk && OutFreqMHz < 14) {
		switch (OutFreqMHz) {
		case 6: VDW=4; RDW=2; OD=10; break;
		case 7: VDW=6; RDW=2; OD=10; break;
		case 8: VDW=8; RDW=2; OD=10; break;
		case 9: VDW=10; RDW=2; OD=10; break;
		case 10: VDW=12; RDW=2; OD=10; break;
		case 11: VDW=14; RDW=2; OD=10; break;
		case 12: VDW=16; RDW=2; OD=10; break;
		case 13: VDW=18; RDW=2; OD=10; break;
		} 
	} else {
		VDW = OutFreqMHz - 8;	// VDW
		RDW = 3;				// RDW
		OD = 4;					// OD
	} 

	switch (OD) {			// change OD to SDW: s2:s1:s0 
		case 0: SDW = 0; break;
		case 1: SDW = 0; break;
		case 2: SDW = 1; break;
		case 3: SDW = 6; break;
		case 4: SDW = 3; break;
		case 5: SDW = 4; break;
		case 6: SDW = 7; break;
		case 7: SDW = 4; break;
		case 8: SDW = 2; break;
		case 9: SDW = 0; break;
		case 10: SDW = 0; break;
		default: SDW = 0; break;
	}

	
	if (Device == 0) {	// select device 1
		tmp = 0x20 | SDW;
		pci_write_reg((DEVICE0_ADDR_PARAM0)<<2, tmp);		// write data 0
		tmp = (VDW << 7)&0xff80 | RDW;
		pci_write_reg((DEVICE0_ADDR_PARAM1)<<2, tmp);		// write data 1
		tmp = 1;
		pci_write_reg((DEVICE0_ADDR_COMMAND)<<2, tmp);		// write command set
		tmp = 0;
		pci_write_reg((DEVICE0_ADDR_COMMAND)<<2, tmp);		// write command reset
	} else {			// select device 2
		tmp = 0x20 | SDW;
		pci_write_reg((DEVICE1_ADDR_PARAM0)<<2, tmp);		// write data 0
		tmp = (VDW << 7)&0xff80 | RDW;
		pci_write_reg((DEVICE1_ADDR_PARAM1)<<2, tmp);		// write data 1
		tmp = 1;
		pci_write_reg((DEVICE1_ADDR_COMMAND)<<2, tmp);		// write command set
		tmp = 0;
		pci_write_reg((DEVICE1_ADDR_COMMAND)<<2, tmp);		// write command reset
	}
	return 1;
}


int hpi_set_timing_opt(unsigned long core_idx, void * base, HANDLE io_mutex) 
{
	int i;
	UINT iAddr;
	UINT uData;
	UINT uuData;
	int iTemp;
	int testFail;
#define MIX_L1_Y_SADDR			(0x11000000 + 0x0138)
#define MIX_L1_CR_SADDR         (0x11000000 + 0x0140)

	s_hpi_base = base;

	i=2;
	// find HPI maximum timing register value
	do {
		i++;
		//iAddr = BIT_BASE + 0x100;
		iAddr = MIX_L1_Y_SADDR;
		uData = 0x12345678;
		testFail = 0;
		printf ("HPI Tw, Tr value: %d\r", i);

		pci_write_reg(0x70<<2, i);
		pci_write_reg(0x71<<2, i);
		if (i<15) 
			pci_write_reg(0x72<<2, 0);
		else
			pci_write_reg(0x72<<2, i*20/100);

		for (iTemp=0; iTemp<10000; iTemp++) {
			if (hpi_write_reg_limit(core_idx, iAddr, uData, io_mutex)==FALSE) {
				testFail = 1;
				break;
			}
			if (hpi_read_reg_limit(core_idx,iAddr, &uuData, io_mutex)==FALSE) {
				testFail = 1;
				break;
			} 
			if (uuData != uData) {
				testFail = 1;
				break;
			}
			else {
				if (hpi_write_reg_limit(core_idx, iAddr, 0, io_mutex)==FALSE) {
					testFail = 1;
					break;
				}
			}

			iAddr += 4;
			/*
			if (iAddr == BIT_BASE + 0x200)
			iAddr = BIT_BASE + 0x100;
			*/
			if (iAddr == MIX_L1_CR_SADDR)
				iAddr = MIX_L1_Y_SADDR;
			uData++;
		}
	} while (testFail && i < HPI_SET_TIMING_MAX);

	pci_write_reg(0x70<<2, i);
	pci_write_reg(0x71<<2, i+i*40/100);
	pci_write_reg(0x72<<2, i*20/100);

	printf ("\nOptimized HPI Tw value : %d\n", pci_read_reg(0x70<<2));
	printf ("Optimized HPI Tr value : %d\n", pci_read_reg(0x71<<2));
	printf ("Optimized HPI Te value : %d\n", pci_read_reg(0x72<<2));



	return i;
}


void pci_write_reg(unsigned int addr, unsigned int data)
{
	unsigned long *reg_addr = (unsigned long *)(addr + (unsigned long)s_hpi_base);
	*(volatile unsigned long *)reg_addr = data;	
}

unsigned int pci_read_reg(unsigned int addr)
{
	unsigned long *reg_addr = (unsigned long *)(addr + (unsigned long)s_hpi_base);
	return *(volatile unsigned long *)reg_addr;
}


void pci_read_memory(unsigned int addr, unsigned char *buf, int size)
{

	int status;
	int i, j, k;
	int data = 0;

	i = j = k = 0;
	
    for (i=0; i < size / HPI_MAX_PKSIZE; i++) 
	{
		pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));		
		pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

		pci_write_reg(HPI_ADDR_CMD, (((HPI_MAX_PKSIZE) << 4) + 1));
		
		do 
		{
			status = 0;
			status = pci_read_reg(HPI_ADDR_STATUS);
			status = status & 1;
		} while (status==0);
		
		for (j=0; j<HPI_MAX_PKSIZE/2; j++) 
		{
			data = pci_read_reg(HPI_ADDR_DATA + j * 4);
            buf[k  ] = (data >> 8) & 0xFF;
            buf[k+1] = data & 0xFF;
            k = k + 2;
		}
		
        addr += HPI_MAX_PKSIZE;
    }
	
    size = size % HPI_MAX_PKSIZE;
    
	if ( ((addr + size) & 0xFFFFFF00) != (addr & 0xFFFFFF00))
        size = size;
	
    if (size) 
	{
		pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
		pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));
		
		pci_write_reg(HPI_ADDR_CMD, (((size) << 4) + 1));
		
		do 
		{
			status = 0;
			status = pci_read_reg(HPI_ADDR_STATUS);
			status = status & 1;
			
		} while (status==0);
		
		for (j = 0; j < size / 2; j++) 
		{
			data = pci_read_reg(HPI_ADDR_DATA + j*4);
            buf[k  ] = (data >> 8) & 0xFF;
            buf[k+1] = data & 0xFF;
            k = k + 2;
		}
    }
}

void pci_write_memory(unsigned int addr, unsigned char *buf, int size)
{
	int status;
	int i, j, k;
	int data = 0;
	
	i = j = k = 0;
	
    for (i = 0; i < size/HPI_MAX_PKSIZE; i++)
	{
		pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
		pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));
		
		for (j=0; j < HPI_MAX_PKSIZE/2; j++) 
		{            
			data = (buf[k] << 8) | buf[k+1];
			pci_write_reg(HPI_ADDR_DATA + j * 4, data);
            k = k + 2;
		}
	
		pci_write_reg(HPI_ADDR_CMD, (((HPI_MAX_PKSIZE) << 4) + 2));
		
		do 
		{
			status = 0;
			status = pci_read_reg(HPI_ADDR_STATUS);
			status = (status >> 1) & 1;
		} while (status==0);
		
        addr += HPI_MAX_PKSIZE;
    }
	
    size = size % HPI_MAX_PKSIZE;
	
	if (size) 
	{
		pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
		pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));
		
		for (j = 0; j< size / 2; j++) 
		{
            data = (buf[k] << 8) | buf[k+1];
			pci_write_reg(HPI_ADDR_DATA + j * 4, data);
            k = k + 2;
		}
		
		pci_write_reg(HPI_ADDR_CMD, (((size) << 4) + 2));
		
		do 
		{
			status = 0;
			status = pci_read_reg(HPI_ADDR_STATUS);
			status = (status>>1) & 1;
			
		} while (status==0);
    }
}

#endif //#ifdef CNM_FPGA_PCI_INTERFACE


#ifdef CNM_FPGA_USB_INTERFACE


#define EP_CTRL_IN 0x80
#define EP_CTRL_OUT	0x00
#define EP_IN 0x86
#define EP_OUT 0x02


#define CNM_USB_VENDORID 0x04b4
#define CNM_USB_DEVICEID 0x1004
#define CNM_USB_CONFIG 1
#define CNM_USB_INTF 0

#define CNM_USB_BULK_TIMEOUT 2000

#define CNM_USB_MAX_BURST_SIZE 1024
#define CNM_USB_BUS_LEN 16
#define CNM_USB_BUS_LEN_ALIGN 15

#define CNM_USB_CMD_READ (0<<15)
#define CNM_USB_CMD_WRITE (1<<15)
#define CNM_USB_CMD_APB (0<<14)
#define CNM_USB_CMD_AXI (1<<14)
#define CNM_USB_CMD_SIZE(SIZE) (SIZE&0x3FFF)

#include "./libusb-1.0.9/libusb/libusb.h"
#pragma warning(disable: 4075)
#ifdef DXVA_UMOD_DRIVER
#else
#pragma comment(lib,".\\vdi\\windows\\libusb-1.0.9\\output\\windows\\libusb-1.0.lib")
#endif

static unsigned long s_dram_base;

static int usb_axi_read_burst(libusb_device_handle *usb, unsigned int addr, unsigned char *buf, int len);
static int usb_axi_write_burst(libusb_device_handle *usb, unsigned int addr, unsigned char *buf, int len);
extern int swap_endian(unsigned char *data, int len, int endian);

typedef struct cnm_usb_apb_write_req_data_t {
	unsigned int  addr;
	unsigned int  data;
} cnm_usb_apb_write_req_data_t;

typedef struct cnm_usb_apb_write_req_packet_t {
	unsigned int  cmd;
	cnm_usb_apb_write_req_data_t data[128];
	unsigned char dummy[508];	// to make the size of apb usb packet to 1536(512*3)
} cnm_usb_apb_write_req_packet_t;

typedef struct cnm_usb_apb_write_res_packet_t {
	unsigned char  data[1024];	
} cnm_usb_apb_write_res_packet_t;


typedef struct cnm_usb_apb_read_req_data_t {
	unsigned int  addr;	
} cnm_usb_apb_read_req_data_t;

typedef struct cnm_usb_apb_read_res_packet_t {
	unsigned char  data[1024];	
} cnm_usb_apb_read_res_packet_t;

typedef struct cnm_usb_apb_read_req_packet_t {
	unsigned int  cmd;
	cnm_usb_apb_read_req_data_t data[128];
	unsigned char dummy[1020];	// to make the size of apb usb packet to 1536(512*3)
} cnm_usb_apb_read_req_packet_t;

typedef struct cnm_usb_axi_write_req_packet_t {
	unsigned int  cmd;
	unsigned int  addr;
	unsigned char data[CNM_USB_MAX_BURST_SIZE];
	unsigned char dummy[1536-CNM_USB_MAX_BURST_SIZE-8];	
} cnm_usb_axi_write_req_packet_t;

typedef struct cnm_usb_axi_write_res_packet_t {
	unsigned char  dummy[1024];	
} cnm_usb_axi_write_res_packet_t;

typedef struct cnm_usb_axi_read_req_packet_t {
	unsigned int  cmd;
	unsigned int  addr;
	unsigned char dummy[1528];	
} cnm_usb_axi_read_req_packet_t;

typedef struct cnm_usb_axi_read_res_packet_t {
	unsigned char  data[CNM_USB_MAX_BURST_SIZE];	
	unsigned char  dummy[(1024-CNM_USB_MAX_BURST_SIZE)];	
} cnm_usb_axi_read_res_packet_t;


int vdi_load_prom_via_usb(unsigned long core_idx, int need_init, const char* path)
{

	unsigned int flag = 0;

#define	USB_BURST_SIZE	4096
#define	STARTCFG	0xF6
#define	ENDCFG		0xF7
#define	STATUSCFG	0xF8
#define REVERSE(b)		((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16
	

	int ret, per, pre_per;
	FILE *fp;
	unsigned int len = 0;
	BYTE *strmBuf, nBuffer[64], buffer[USB_BURST_SIZE];
	unsigned int i = 0, j = 0, actual_length;
	int byte, ff_count;
	int cur_pos = -1;
	libusb_device_handle *usb;
	int res = 0;
	vdi_info_t *vdi;

	if (GetFileAttributes(path) == (DWORD)-1)
	{
		VLOG(ERR, "vdi_load_prom_via_usb:File not exist:%s\n", path);
		return 0;
	}

	if ( need_init )
	{
		vdi_init(core_idx);
	}

	vdi = &s_vdi_info[core_idx];

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;
	
	usb = (libusb_device_handle *)vdi->vpu_fd;

	libusb_clear_halt(usb, EP_CTRL_IN);
	libusb_clear_halt(usb, EP_CTRL_OUT);
	libusb_clear_halt(usb, EP_IN);
	libusb_clear_halt(usb, EP_OUT);	


	printf("%s writing START\n", path);

	fp = fopen(path, "rb");

	ff_count = 0;
	while (!feof(fp)) {
		byte = fgetc(fp);
		if (byte == 0xff) {
			if (cur_pos == -1) cur_pos = ftell(fp)-1;
			ff_count++;
		} else {
			cur_pos = -1;
		}

		if (ff_count == 32) break;
	}

	if (ff_count != 32) {
		fprintf(stderr, "vdi_load_prom_via_usb failed to find 32 0xff\n");
		goto ERR_USB_INIT;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp) - cur_pos;
	//printf("cur_pos = %02d\n", cur_pos);

	fseek(fp, cur_pos, SEEK_SET);

	strmBuf = (BYTE *)malloc(len);
	memset(strmBuf, 0, len);
	fread(strmBuf, len, 1, fp);
	fclose(fp);

#if 0
	for(i=0; i<256; i=i+16){
		printf("%03X = ", i);
		for(j=0; j<16; j++)
			printf("%02X ", strmBuf[i+j]);
		printf("\n");
	}
#endif
	ret = libusb_control_transfer(usb, LIBUSB_RECIPIENT_ENDPOINT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, ENDCFG, 0, 0, nBuffer, sizeof(nBuffer), CNM_USB_BULK_TIMEOUT);
	if(ret < 0)
		goto ERR_USB_INIT;

	ret = libusb_control_transfer(usb, LIBUSB_RECIPIENT_ENDPOINT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, STARTCFG, 0, 0, nBuffer, sizeof(nBuffer), CNM_USB_BULK_TIMEOUT);
	if(ret < 0)
		goto ERR_USB_INIT;

	for(i=0; i<len; i++) 
		strmBuf[i] = (unsigned char)(REVERSE(strmBuf[i]));

	for(j=0; j<len; j+= USB_BURST_SIZE){
		ret = (len-j) < USB_BURST_SIZE ? (len-j) : USB_BURST_SIZE;
		memcpy(buffer, strmBuf + j, ret);
		per = (j>>10)*100/(len>>10);
		if ( !(per%5) && per != pre_per )
				printf("%3d%%\r", per);
		pre_per = per;

		if(ret < USB_BURST_SIZE){
			for(i=0; ret+i<USB_BURST_SIZE; i++) buffer[ret+i] = 0;
			ret = ret + i;
		}
		ret = libusb_bulk_transfer(usb, EP_OUT, buffer, sizeof(buffer), &actual_length, CNM_USB_BULK_TIMEOUT);
		if (ret < 0 || actual_length != sizeof(buffer))
		{
			VLOG(ERR, "[VDI] vdi_load_prom_via_usb libusb_bulk_transfer[EP_OUT] req fail , ret=%d, actual_length=%d\n", ret, actual_length);
			goto ERR_USB_INIT;
		}
	}
	printf("100%%\n");
	free(strmBuf);

	ret = libusb_control_transfer(usb, LIBUSB_RECIPIENT_ENDPOINT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, ENDCFG, 0, 0, nBuffer, sizeof(nBuffer), CNM_USB_BULK_TIMEOUT);
	if(ret < 0)
		goto ERR_USB_INIT;

	ret = libusb_control_transfer(usb, LIBUSB_RECIPIENT_ENDPOINT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, STATUSCFG, 0, 0, nBuffer, sizeof(nBuffer), CNM_USB_BULK_TIMEOUT);
	if(ret < 0)
		goto ERR_USB_INIT;

	if(nBuffer[2]) 
		printf("\nvdi_load_prom_via_usb %s writing SUCCESS\n", path);
	else 
	{ 
		printf("\nvdi_load_prom_via_usb %s writing FAIL\n", path);
		printf("\nvdi_load_prom_via_usb %s writing FAIL\n", path);
		printf("\nvdi_load_prom_via_usb %s writing FAIL\n", path);
		goto ERR_USB_INIT; 
	}

	res = 0;

	if (need_init && usb)
	{
		vdi_release(core_idx);
	}

	return res;

ERR_USB_INIT:
	res = -1;
	if (need_init && usb)
	{
		vdi_release(core_idx);
	}

	return res;
}


struct libusb_device_handle *s_ref_usb; // it will have the context of first core.

void * usb_init(unsigned long core_idx, unsigned long dram_base)
{
	int ret;
	struct libusb_device_handle *usb;

    if (core_idx>MAX_NUM_VPU_CORE)
        return (void *)0;

	s_dram_base = dram_base;
	
	if (s_ref_usb && core_idx > 0)
		return (void *) s_ref_usb;
	
	ret = libusb_init(NULL);
	if (ret < 0)
	{
		VLOG(ERR, "libusb_init failed.  Error:%d\n", ret);
		goto ERR_USB_INIT;
	}

	usb = libusb_open_device_with_vid_pid(NULL, CNM_USB_VENDORID, CNM_USB_DEVICEID);
	if (!usb)
	{
		VLOG(ERR, "libusb_open_device_with_vid_pid error \n");		
		goto ERR_USB_INIT;
	}

	ret = libusb_set_configuration(usb, CNM_USB_CONFIG);
	if (ret < 0) {
		VLOG(ERR, "libusb_set_configuration error %d\n", ret);
		goto ERR_USB_INIT;
	}

	ret = libusb_claim_interface(usb, CNM_USB_INTF);
	if (ret < 0) {
		VLOG(ERR, "usb_claim_interface error %d\n", ret);
		goto ERR_USB_INIT;
	}

	
	//libusb_reset_device(vdi->vpu_fd);
	libusb_clear_halt(usb, EP_CTRL_IN);
	libusb_clear_halt(usb, EP_CTRL_OUT);
	libusb_clear_halt(usb, EP_IN);
	libusb_clear_halt(usb, EP_OUT);	

	s_ref_usb = usb;

	return (void *) usb;

ERR_USB_INIT:

	return (void *)0;
}

void usb_release(void * base, unsigned long core_idx)
{
	libusb_device_handle *usb = (libusb_device_handle *)base;

	if (core_idx > 0 && !s_ref_usb) // if libusb's been already closed
		return;

	libusb_release_interface(usb, 0);
	libusb_close(usb);
	libusb_exit(NULL);
	s_ref_usb = NULL;
}


int usb_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data, MUTEX_HANDLE io_mutex)
{
	int ret;
	libusb_device_handle *usb = (libusb_device_handle *)base;
	cnm_usb_apb_write_req_packet_t req;
	cnm_usb_apb_write_res_packet_t res;
	int actual_length;
	
	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return -1;

	req.cmd = CNM_USB_CMD_WRITE | CNM_USB_CMD_APB | CNM_USB_CMD_SIZE(4);
	
	req.data[0].addr = addr;
	req.data[0].data = data;
	
	ret = libusb_bulk_transfer(usb, EP_OUT, (unsigned char *)&req, sizeof(cnm_usb_apb_write_req_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
	if (ret < 0 || actual_length != sizeof(cnm_usb_apb_write_req_packet_t))
	{
		VLOG(ERR, "[VDI] usb_write_register error libusb_bulk_transfer[EP_OUT] req fail , ret=%d, actual_length=%d\n", ret, actual_length);
		return -1;
	}

	ret = libusb_bulk_transfer(usb, EP_IN, (unsigned char *)&res, sizeof(cnm_usb_apb_write_res_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
	if (ret < 0 || actual_length != sizeof(cnm_usb_apb_write_res_packet_t))
	{
		VLOG(ERR, "[VDI] usb_write_register error libusb_bulk_transfer[EP_OUT]res fail , ret=%d, actual_length=%d\n", ret, actual_length);
		return -1;
	}

	ReleaseMutex(io_mutex);	

	return 1;
}



unsigned int usb_read_register(unsigned long core_idx, void * base, unsigned int addr, MUTEX_HANDLE io_mutex)
{
	
	libusb_device_handle *usb = (libusb_device_handle *)base;
	int ret;
	int actual_length;
	unsigned int data;
	cnm_usb_apb_read_req_packet_t req;
	cnm_usb_apb_read_res_packet_t res;

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return -1;

	req.cmd = CNM_USB_CMD_READ | CNM_USB_CMD_APB | CNM_USB_CMD_SIZE(4);
	req.data[0].addr = addr;
	
	ret = libusb_bulk_transfer(usb, EP_OUT, (unsigned char *)&req, sizeof(cnm_usb_apb_write_req_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
	if (ret < 0 || actual_length != sizeof(cnm_usb_apb_write_req_packet_t))
	{
		VLOG(ERR, "[VDI] usb_read_register error libusb_bulk_transfer[EP_OUT] req fail, ret=%d, actual_length=%d\n", ret, actual_length);
		return -1;
	}

	ret = libusb_bulk_transfer(usb, EP_IN, (unsigned char *)&res, sizeof(cnm_usb_apb_write_res_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
	if (ret < 0 || actual_length != sizeof(cnm_usb_apb_write_res_packet_t))
	{
		VLOG(ERR, "[VDI] usb_write_register error libusb_bulk_transfer[EP_IN] res fail, ret=%d, actual_length=%d\n", ret, actual_length);		
		return -1;
	}
	
	data = (res.data[3]<<24 | res.data[2]<<16 | res.data[1]<<8 | res.data[0]);

	ReleaseMutex(io_mutex);	

	return data;
}




int usb_write_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, MUTEX_HANDLE io_mutex)
{	
	libusb_device_handle *usb = (libusb_device_handle *)base;
	unsigned char *pBuf;
	unsigned char lsBuf[CNM_USB_BUS_LEN];
	int lsOffset;	


	if (addr < s_dram_base) {
		fprintf(stderr, "[USB] invalid address base address is 0x%x\n", (int)s_dram_base);
		return 0;
	}

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return -1;

	if (len==0) {
		ReleaseMutex(io_mutex);	
		return 0;
	}
	
	addr = addr - s_dram_base;
	
	lsOffset = addr - (addr/CNM_USB_BUS_LEN)*CNM_USB_BUS_LEN;
	if (lsOffset)
	{
		usb_axi_read_burst(usb, (addr/CNM_USB_BUS_LEN)*CNM_USB_BUS_LEN, lsBuf, CNM_USB_BUS_LEN);
		pBuf = (unsigned char *)malloc((len+lsOffset+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
		if (pBuf)
		{
			memset(pBuf, 0x00, (len+lsOffset+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
			memcpy(pBuf, lsBuf, CNM_USB_BUS_LEN);
			swap_endian(pBuf, CNM_USB_BUS_LEN, endian);	
			memcpy(pBuf+lsOffset, data, len);
			swap_endian(pBuf, ((len+lsOffset+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN), endian);	
			usb_axi_write_burst(usb, (addr/CNM_USB_BUS_LEN)*CNM_USB_BUS_LEN, (unsigned char *)pBuf, ((len+lsOffset+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN));	
			free(pBuf);
		}
	}
	else
	{
		pBuf = (unsigned char *)malloc((len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
		if (pBuf) {
			memset(pBuf, 0x00, (len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
			memcpy(pBuf, data, len);
			swap_endian(pBuf, (len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN, endian);	
			usb_axi_write_burst(usb, addr, (unsigned char *)pBuf,(len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);	
			free(pBuf);
		}
	}

	
	ReleaseMutex(io_mutex);	

	return len;	
}

int usb_read_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, MUTEX_HANDLE io_mutex)
{
	unsigned char *pBuf;
	unsigned char lsBuf[CNM_USB_BUS_LEN];
	int lsOffset;	

	libusb_device_handle *usb = (libusb_device_handle *)base;

	if (addr < s_dram_base) {
		fprintf(stderr, "[USB] invalid address base address is 0x%x\n", (int)s_dram_base);
		return 0;
	}


	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return -1;

	if (len==0) {
		ReleaseMutex(io_mutex);	
		return 0;
	}
	
	addr = addr - s_dram_base;

	lsOffset = addr - (addr/CNM_USB_BUS_LEN)*CNM_USB_BUS_LEN;
	if (lsOffset)
	{
		usb_axi_read_burst(usb, (addr/CNM_USB_BUS_LEN)*CNM_USB_BUS_LEN, lsBuf, CNM_USB_BUS_LEN);	
		swap_endian(lsBuf, CNM_USB_BUS_LEN, endian);	
		len = len-(CNM_USB_BUS_LEN-lsOffset);
		if (len == 0)
		{
			memcpy(data, lsBuf+lsOffset, CNM_USB_BUS_LEN-lsOffset);
		}
		else
		{
			pBuf = (unsigned char *)malloc(((len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN));
			if (pBuf)
			{
				memset(pBuf, 0x00, ((len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN));
				usb_axi_read_burst(usb, (addr+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN, pBuf, ((len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN));	
				swap_endian(pBuf, ((len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN), endian);	
	
				memcpy(data, lsBuf+lsOffset, CNM_USB_BUS_LEN-lsOffset);
				memcpy(data+CNM_USB_BUS_LEN-lsOffset, pBuf, len);
	
				free(pBuf);
			}
		}
	}
	else
	{
		pBuf = (unsigned char *)malloc((len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
		if (pBuf)
		{
			memset(pBuf, 0x00, (len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
			usb_axi_read_burst(usb, addr, pBuf, (len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN);
			swap_endian(pBuf, (len+CNM_USB_BUS_LEN_ALIGN)&~CNM_USB_BUS_LEN_ALIGN, endian);	
			memcpy(data, pBuf, len);
			free(pBuf);
		}
	}

	ReleaseMutex(io_mutex);	

	return len;
}


int usb_hw_reset(void * base, MUTEX_HANDLE io_mutex)
{
	int ret;
	libusb_device_handle *usb = (libusb_device_handle *)base;
	cnm_usb_apb_write_req_packet_t req;
	int actual_length;


	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return 0;

	req.cmd = CNM_USB_CMD_WRITE | CNM_USB_CMD_APB | CNM_USB_CMD_SIZE(4);

	req.data[0].addr = DEVICE_ADDR_SW_RESET;
	req.data[0].data = 1;

	ret = libusb_bulk_transfer(usb, EP_OUT, (unsigned char *)&req, sizeof(cnm_usb_apb_write_req_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
	if (ret < 0 || actual_length != sizeof(cnm_usb_apb_write_req_packet_t))
	{
		VLOG(ERR, "[VDI] usb_write_register error libusb_bulk_transfer[EP_OUT] req fail , ret=%d, actual_length=%d\n", ret, actual_length);
		return ret;
	}
#if defined(_MSC_VER)
	Sleep(1000);
#else
	usleep(1000*1000);
#endif

	ReleaseMutex(io_mutex);	
	return 0;
}


/*------------------------------------------------------------------------
	Usage : used to program output frequency of ICS307M
	Artument :
		Device		: first device selected if 0, second device if 1.
		OutFreqMHz	: Target output frequency in MHz.
		InFreqMHz	: Input frequency applied to device in MHz
					  this must be 10 here.
	Return : TRUE if success, FALSE if invalid OutFreqMHz.
------------------------------------------------------------------------*/



int usb_ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz, int InFreqMHz, MUTEX_HANDLE io_mutex)
{
	int		VDW, RDW, OD, SDW, tmp, tmp1, tmp2;
	int		min_clk; 
	int		max_clk;

	libusb_device_handle *usb = (libusb_device_handle *)base;

	if ( Device == 0 )
	{   
		min_clk = ACLK_MIN ;
		max_clk = ACLK_MAX ;
	}
	else
	{   
		min_clk = CCLK_MIN ;
		max_clk = CCLK_MAX ;
	}
	
	if (OutFreqMHz < min_clk || OutFreqMHz > max_clk) {
	   // printf ("Target Frequency should be from %2d to %2d !!!\n", min_clk, max_clk);
		return 0;
	}

	if (OutFreqMHz >= min_clk && OutFreqMHz < 14) {
		switch (OutFreqMHz) {
		case 6: VDW=4; RDW=2; OD=10; break;
		case 7: VDW=6; RDW=2; OD=10; break;
		case 8: VDW=8; RDW=2; OD=10; break;
		case 9: VDW=10; RDW=2; OD=10; break;
		case 10: VDW=12; RDW=2; OD=10; break;
		case 11: VDW=14; RDW=2; OD=10; break;
		case 12: VDW=16; RDW=2; OD=10; break;
		case 13: VDW=18; RDW=2; OD=10; break;
		} 
	} else {
		VDW = OutFreqMHz - 8;	// VDW
		RDW = 3;				// RDW
		OD = 4;					// OD
	} 

	switch (OD) {			// change OD to SDW: s2:s1:s0 
		case 0: SDW = 0; break;
		case 1: SDW = 0; break;
		case 2: SDW = 1; break;
		case 3: SDW = 6; break;
		case 4: SDW = 3; break;
		case 5: SDW = 4; break;
		case 6: SDW = 7; break;
		case 7: SDW = 4; break;
		case 8: SDW = 2; break;
		case 9: SDW = 0; break;
		case 10: SDW = 0; break;
		default: SDW = 0; break;
	}

	
	if (Device == 0) {	// select device 1
		tmp1 = 0x20 | SDW;
		tmp2 = (VDW << 7)&0xff80 | RDW;
		tmp = ((tmp1 & 0xffff) << 16) | (tmp2 & 0xffff);
		if(usb_write_register(0, base, DEVICE0_ADDR_PARAM0, tmp, io_mutex) == -1)		// write data 0
			return -1;
		if(usb_write_register(0, base, DEVICE0_ADDR_COMMAND, 1, io_mutex) == -1)		//  write command set
			return -1;
		if(usb_write_register(0, base, DEVICE0_ADDR_COMMAND, 0, io_mutex) == -1)		//  write command reset		
			return -1;
	} else {			// select device 2
		tmp1 = 0x20 | SDW;
		tmp2 = (VDW << 7)&0xff80 | RDW;
		tmp = ((tmp1 & 0xffff) << 16) | (tmp2 & 0xffff);
		if(usb_write_register(0, base, DEVICE1_ADDR_PARAM0, tmp, io_mutex) == -1)		// write data 0
			return -1;
		if(usb_write_register(0, base, DEVICE1_ADDR_COMMAND, 1, io_mutex) == -1)		//  write command set
			return -1;
		if(usb_write_register(0, base, DEVICE1_ADDR_COMMAND, 0, io_mutex) == -1)		//  write command reset		
			return -1;
	}

	Sleep(100);

	return 1;
}


int usb_set_timing_opt(unsigned long core_idx, void * base, MUTEX_HANDLE io_mutex) 
{
	printf ("\nusb_set_timing_opt : Don't required in case of USB interface\n");
	return 0;
}


int usb_axi_read_burst(libusb_device_handle *usb, unsigned int addr, unsigned char *buf,int len)
{
	int ret;
	cnm_usb_axi_read_req_packet_t req;
	cnm_usb_axi_read_res_packet_t res;
	int actual_length;
	int pos;
	int req_len;

	pos = 0;
	req_len = CNM_USB_MAX_BURST_SIZE;
	if (len < CNM_USB_MAX_BURST_SIZE)
		req_len = len;

	if (req_len < CNM_USB_BUS_LEN) 
	{
		VLOG(ERR, "[VDI] usb_axi_read_burst error transfer len=%d is invalid\n", len);
		return 0;
	}

	do 
	{
		req.cmd = CNM_USB_CMD_READ | CNM_USB_CMD_AXI | CNM_USB_CMD_SIZE(req_len);
		req.addr = addr+pos;
		
		ret = libusb_bulk_transfer(usb, EP_OUT, (unsigned char *)&req, sizeof(cnm_usb_axi_read_req_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
		if (ret < 0 || actual_length != sizeof(cnm_usb_axi_read_req_packet_t))
		{
			VLOG(ERR, "[VDI] usb_axi_read_burst error libusb_bulk_transfer[EP_OUT] req fail, ret=%d, actual_length=%d\n", ret, actual_length);
			return 0;
		}

		ret = libusb_bulk_transfer(usb, EP_IN, (unsigned char *)&res, sizeof(cnm_usb_axi_read_res_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
		if (ret < 0 || actual_length != sizeof(cnm_usb_axi_read_res_packet_t))
		{
			VLOG(ERR, "[VDI] usb_axi_read_burst error libusb_bulk_transfer[EP_IN] res usb, ret=%d, actual_length=%d\n", ret, actual_length);		
			return 0;
		}

		memcpy(buf+pos, &res.data[0], req_len);
		len -= CNM_USB_MAX_BURST_SIZE;
		if (len < CNM_USB_MAX_BURST_SIZE)
			req_len = len;
		pos += CNM_USB_MAX_BURST_SIZE;

	} while(len > 0);


	return 1;
}

int usb_axi_write_burst(libusb_device_handle *usb, unsigned int addr, unsigned char *buf, int len)
{
	int ret;
	cnm_usb_axi_write_req_packet_t req;
	cnm_usb_axi_write_res_packet_t res;
	int actual_length;
	int pos, req_len;

	req_len = CNM_USB_MAX_BURST_SIZE;
	if (len < CNM_USB_MAX_BURST_SIZE)
		req_len = len;

	if (req_len < CNM_USB_BUS_LEN) 
	{
		VLOG(ERR, "[VDI] usb_axi_write_burst error transfer len=%d is invalid\n", len);
		return 0;
	}

	pos = 0;
	do 
	{
		req.cmd = CNM_USB_CMD_WRITE | CNM_USB_CMD_AXI | CNM_USB_CMD_SIZE(req_len);
		req.addr = addr+pos;
		memcpy(&req.data[0], &buf[pos], req_len);

		ret = libusb_bulk_transfer(usb, EP_OUT, (unsigned char *)&req, sizeof(cnm_usb_axi_write_req_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
		if (ret < 0 || actual_length != sizeof(cnm_usb_axi_write_req_packet_t))
		{
			VLOG(ERR, "[VDI] usb_axi_write_burst error libusb_bulk_transfer[EP_OUT] req fail, ret=%d, actual_length=%d\n", ret, actual_length);
			return 0;
		}

		ret = libusb_bulk_transfer(usb, EP_IN, (unsigned char *)&res, sizeof(cnm_usb_axi_write_res_packet_t), &actual_length, CNM_USB_BULK_TIMEOUT);
		if (ret < 0 || actual_length != sizeof(cnm_usb_axi_write_res_packet_t))
		{
			VLOG(ERR, "[VDI] usb_axi_write_burst error libusb_bulk_transfer[EP_IN] res fail, ret=%d, actual_length=%d\n", ret, actual_length);		
			return 0;
		}

		len -= CNM_USB_MAX_BURST_SIZE;
		if (len < CNM_USB_MAX_BURST_SIZE)
			req_len = len;

		pos += CNM_USB_MAX_BURST_SIZE;
		

	} while(len > 0);
	
	
	return 1;
}
#endif //#ifdef CNM_FPGA_USB_INTERFACE
#endif //#include CNM_FPGA_PLATFORM
