//------------------------------------------------------------------------------
// File: vdi.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _CNM_HPI_H_
#define _CNM_HPI_H_

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)

#include "../../config.h"

#ifdef CNM_FPGA_PLATFORM

#define HPI_BUS_LEN 8
#define HPI_BUS_LEN_ALIGN 7


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

#define HPI_CMD_WRITE_VALUE			((8 << 4) + 2)
#define HPI_CMD_READ_VALUE			((8 << 4) + 1)

#define HPI_MAX_PKSIZE 256

// chipsnmedia clock generator in FPGA

#define	DEVICE0_ADDR_COMMAND		0x75
#define DEVICE0_ADDR_PARAM0			0x76
#define	DEVICE0_ADDR_PARAM1			0x77
#define	DEVICE1_ADDR_COMMAND		0x78
#define DEVICE1_ADDR_PARAM0			0x79
#define	DEVICE1_ADDR_PARAM1			0x7a
#define DEVICE_ADDR_SW_RESET		0x7b

#define ACLK_MAX					30
#define ACLK_MIN					16
#define CCLK_MAX					30
#define CCLK_MIN					16


#if defined (__cplusplus)
extern "C" {
#endif 
	void * hpi_init(unsigned long core_idx, unsigned long dram_base);
	void hpi_release(unsigned long core_idx);
	void hpi_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data, HANDLE io_mutex);
	unsigned int hpi_read_register(unsigned long core_idx, void * base, unsigned int addr, HANDLE io_mutex);
	int hpi_write_memory(unsigned long core_idx,void * base, unsigned int addr, unsigned char *data, int len, int endian, HANDLE io_mutex);
	int hpi_read_memory(unsigned long core_idx, void * base, unsigned int addr, unsigned char *data, int len, int endian, HANDLE io_mutex);
	int hpi_hw_reset(void * base);
	
	int hpi_set_timing_opt(unsigned long core_idx, void * base, HANDLE io_mutex);
	int ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz, int InFreqMHz);


#if defined (__cplusplus)
}
#endif 

#endif	//#ifdef CNM_FPGA_PLATFORM

#endif	//#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)

#endif //#ifndef _CNM_HPI_H_

