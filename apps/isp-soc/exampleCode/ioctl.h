/*--------------------------------------------------------------------------
/	Project name	: 
/	Copyright		: Anapass, All rights reserved.
/	File name		: ioctl.h
/	Description		:
/
/	
/	
/	
/	Name		Date			Action
/	thlee		14.07.18		Create
---------------------------------------------------------------------------*/

#ifndef	__ISP_IOCTL_H__
#define __ISP_IOCTL_H__

#include "debug.h"
/* --------------------------------------------------------------------------------------------------------------------
													CONSTANTS
 ------------------------------------------------------------------------------------------------------------------ */

extern unsigned char *ISPBase;
extern unsigned char *SIFBase;
extern unsigned char *DXOBase;

extern int gdebug1;

void isp_setDXORegister(unsigned int offset, unsigned int value);
unsigned int isp_getDXORegister(unsigned int offset);
void isp_setISPRegister(unsigned int offset, unsigned int value);
unsigned int isp_getISPRegister(unsigned int offset);
void isp_setSIFRegister(unsigned int offset, unsigned int value);
unsigned int isp_getSIFRegister(unsigned int offset);

#if 0
#define	DRIVER_MSG_OFF	isp_cmd_write(ISP_IOCTL_DEBUG, 0)
#define	DRIVER_MSG_ON	isp_cmd_write(ISP_IOCTL_DEBUG, 1)

#define DRIVER_INT_WAIT	isp_cmd_write(ISP_IOCTL_WAITINT, 0)

#define DRIVER_SET_INTMASK(a)	isp_cmd_write(ISP_IOCTL_INTMASK, a)

#define DRIVER_RUN_EVENT(a)	isp_cmd_write(ISP_IOCTL_RUN_EVENT, a)
#define I2C_WRITE(a)	    isp_cmd_write(ISP_IOCTL_I2CM_WRITE, a)
#define I2C_READ(a)         isp_cmd_write(ISP_IOCTL_I2CM_READ, a)
#else
#define	DRIVER_MSG_OFF	do { isp_cmd_write(ISP_CTRL_IOC_DEBUG, 0); gdebug1 = 0; } while(0)
#define	DRIVER_MSG_ON	do { isp_cmd_write(ISP_CTRL_IOC_DEBUG, 1); gdebug1 = 1; } while(0)

#define DRIVER_INT_WAIT	isp_cmd_write(ISP_CTRL_IOC_WAITINT, -1)

#define DRIVER_SET_INTMASK(a)	isp_cmd_write(ISP_CTRL_IOC_INTMASK, a)

//#define DRIVER_RUN_EVENT(a)	isp_cmd_write(ISP_IOCTL_RUN_EVENT, a)
void DRIVER_RUN_EVENT(int);
#define I2C_WRITE(a)	    isp_cmd_write(ISP_I2C_IOC_WRITE, (uint32_t)(a))
#define I2C_READ(a)         isp_cmd_write(ISP_I2C_IOC_READ, (uint32_t)(a))
#endif  /*0*/

#if 0
//[[
#define SIF_WRITE(a,b)	isp_RegWrite(CMD_SIF_WRITE, a, b)
#define SIF_READ(a)		isp_RegRead(CMD_SIF_READ, a)

#define DXO_WRITE(a,b)	isp_RegWrite(CMD_DXO_WRITE, a, b)
#define DXO_READ(a)		isp_RegRead(CMD_DXO_READ, a)

#define ISP_WRITE(a,b)	isp_RegWrite(CMD_ISP_WRITE, a, b)
#define ISP_READ(a)		isp_RegRead(CMD_ISP_READ, a)
//]]
#else

#define __USE_IO_MMAP

#   ifdef   __USE_IO_MMAP
#define ISP_WRITE(a, v) (*(volatile uint32_t *)((char *)ISPBase + (a)) = ((uint32_t)(v)))
#define ISP_READ(a)     (*(volatile uint32_t *)((char *)ISPBase + (a)))
#   else
#define ISP_WRITE(a, v) isp_setISPRegister(a, v)
#define ISP_READ(a)     isp_getISPRegister(a)
#   endif  /*__USE_IO_MMAP*/

#   ifdef   __USE_IO_MMAP
#define SIF_WRITE(a, v) (*(volatile uint32_t *)((char *)SIFBase + (a)) = ((uint32_t)(v)))
#define SIF_READ(a)     (*(volatile uint32_t *)((char *)SIFBase + (a)))
#   else
#define SIF_WRITE(a, v)     isp_setSIFRegister(a, v)
#define SIF_READ(a)         isp_getSIFRegister(a)
#   endif   /*__USE_IO_MMAP*/

#   ifdef   __USE_IO_MMAP
#define DXO_WRITE(a, v) (*(volatile uint32_t *)((char *)DXOBase + (a)) = ((uint32_t)(v)))
#define DXO_READ(a)     (*(volatile uint32_t *)((char *)DXOBase + (a)))
#   else
#define DXO_WRITE(a, v)     isp_setDXORegister(a, v)
#define DXO_READ(a)         isp_getDXORegister(a)
#   endif   /*__USE_IO_MMAP*/
//]]
#endif  /*0*/

/* --------------------------------------------------------------------------------------------------------------------
													DATA TYPES
 ------------------------------------------------------------------------------------------------------------------ */

/* --------------------------------------------------------------------------------------------------------------------
													FUNCTION PROTOTYPES
 ------------------------------------------------------------------------------------------------------------------ */
int isp_ioctl_init(void);
void isp_ioctl_close(void);
void isp_RegWrite(uint32_t cmd, uint32_t offset, uint32_t value);
int isp_RegRead(uint32_t cmd, uint32_t offset);
uint32_t isp_cmd_write(uint32_t cmd, uint32_t value);
void isp_setRegister(const int uiOffset, int uiValue);
int isp_getRegister(const int uiOffset);

#endif

/* EOF */

