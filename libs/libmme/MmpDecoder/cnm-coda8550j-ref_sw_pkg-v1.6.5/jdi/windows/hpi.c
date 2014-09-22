//------------------------------------------------------------------------------
// File: hpi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)

#ifdef CNM_FPGA_PLATFORM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../jdi.h"
#include "hpi.h"



static void * s_hpi_base;
static unsigned long s_dram_base;

static int hpi_read_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int *data, HANDLE io_mutex);
static int hpi_write_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int data, HANDLE io_mutex);
static unsigned int pci_read_reg(unsigned int addr);
static void pci_write_reg(unsigned int addr, unsigned int data);
static void pci_write_memory(unsigned int addr, unsigned char *buf, int size);
static void pci_read_memory(unsigned int addr, unsigned char *buf, int size);

extern int jpu_swap_endian(unsigned char *data, int len, int endian);

void * hpi_init(unsigned long core_idx, unsigned long dram_base)
{
#define MAX_NUM_JPU_CORE 2
    if (core_idx>MAX_NUM_JPU_CORE)
        return (void *)-1;
	s_dram_base = dram_base;
	
	return (void *)1;
}

void hpi_release(unsigned long core_idx)
{
}

void hpi_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data, HANDLE io_mutex)
{
	int status;

	if (WaitForSingleObject(io_mutex, INFINITE) != WAIT_OBJECT_0)		
		return;
	
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
		pBuf = (unsigned char *)malloc((len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
		if (pBuf)
		{
			memset(pBuf, 0x00, (len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN);
			memcpy(pBuf, lsBuf, HPI_BUS_LEN);
			memcpy(pBuf+lsOffset, data, len);
			jpu_swap_endian(pBuf, ((len+lsOffset+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN), endian);	
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
			jpu_swap_endian(pBuf, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN, endian);	
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
		jpu_swap_endian(lsBuf, HPI_BUS_LEN, endian);	
		len = len-(HPI_BUS_LEN-lsOffset);
		pBuf = (unsigned char *)malloc(((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));
		if (pBuf)
		{
			memset(pBuf, 0x00, ((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));
			pci_read_memory((addr+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN, pBuf, ((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN));	
			jpu_swap_endian(pBuf, ((len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN), endian);	

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
			jpu_swap_endian(pBuf, (len+HPI_BUS_LEN_ALIGN)&~HPI_BUS_LEN_ALIGN, endian);	
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



int ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz, int InFreqMHz)
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
	unsigned int iAddr;
	unsigned int uData;
	unsigned int uuData;
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


#endif //#include CNM_FPGA_PLATFORM
#endif
