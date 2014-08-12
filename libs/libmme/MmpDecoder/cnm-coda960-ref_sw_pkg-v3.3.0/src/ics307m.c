/*------------------------------------------------------------------------
       This confidential and proprietary software may be used only
     as authorized by a licensing agreement from Chips&Media Inc.
     In the event of publication, the following notice is applicable:

                    (C) COPYRIGHT 2003 - 2007   CHIPS&MEDIA INC.
                           ALL RIGHTS RESERVED

       The entire notice above must be reproduced on all authorized
     copies.

    V1.0.0, Nov 14, 2008    Revised by Thomas kim
  
--------------------------------------------------------------------------
 FILE		:	ICS307M.c
------------------------------------------------------------------------*/
#include "../config.h"
#ifdef PLATFORM_FPGA
#include <stdio.h>
#include <stdlib.h>
#include "../vdi/windows/hpi.h"
#include "regdefine.h"
#include "ics307m.h"
/*------------------------------------------------------------------------
	Usage : used to program output frequency of ICS307M
	Artument :
		Device		: first device selected if 0, second device if 1.
		OutFreqMHz	: Target output frequency in MHz.
		InFreqMHz	: Input frequency applied to device in MHz
					  this must be 10 here.
	Return : TRUE if success, FALSE if invalid OutFreqMHz.
------------------------------------------------------------------------*/



int SetFreqICS307M(int Device, int OutFreqMHz, int InFreqMHz)
{
	
	int		VDW, RDW, OD, SDW, tmp;
	int		min_clk ; 
	int		max_clk ;

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


int SetHPITimingOpt() 
{
	int i;
	UINT iAddr;
	UINT uData;
	UINT uuData;
	int iTemp;
	int testFail;
#define MIX_L1_Y_SADDR			(0x11000000 + 0x0138)
#define MIX_L1_CR_SADDR         (0x11000000 + 0x0140)
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
			if (hpi_write_reg_limit(iAddr, uData)==FALSE) {
				testFail = 1;
				break;
			}
			if (hpi_read_reg_limit(iAddr, &uuData)==FALSE) {
				testFail = 1;
				break;
			} 
			if (uuData != uData) {
				testFail = 1;
				break;
			}
			else {
				if (hpi_write_reg_limit(iAddr, 0)==FALSE) {
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
	} while (testFail);

	pci_write_reg(0x70<<2, i);
	pci_write_reg(0x71<<2, i+i*40/100);
	pci_write_reg(0x72<<2, i*20/100);

	printf ("\nOptimized HPI Tw value : %d\n", pci_read_reg(0x70<<2));
	printf ("Optimized HPI Tr value : %d\n", pci_read_reg(0x71<<2));
	printf ("Optimized HPI Te value : %d\n", pci_read_reg(0x72<<2));



	return i;
}
#endif

